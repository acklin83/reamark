from fastapi import APIRouter, BackgroundTasks, Depends, HTTPException, Request
from sqlalchemy.orm import Session

from ..auth import get_current_admin
from ..database import get_db
from ..email_service import send_comment_notification
from ..models import AppSettings, Comment, Project, Reply, Song, Version
from ..schemas import CommentCreate, CommentOut, CommentUpdate, ReplyCreate, ReplyOut

router = APIRouter(tags=["comments"])


def _validate_share_link(share_link: str, db: Session) -> Project:
    project = db.query(Project).filter(Project.share_link == share_link).first()
    if project is None:
        raise HTTPException(status_code=404, detail="Project not found")
    return project


def _get_comment_in_project(comment_id: int, project: Project, db: Session) -> Comment:
    comment = (
        db.query(Comment)
        .join(Version)
        .join(Song)
        .filter(Song.project_id == project.id, Comment.id == comment_id)
        .first()
    )
    if comment is None:
        raise HTTPException(status_code=404, detail="Comment not found in this project")
    return comment


@router.get("/api/projects/{share_link}/comments", response_model=list[CommentOut])
def get_comments(
    share_link: str,
    version_id: int | None = None,
    song_id: int | None = None,
    db: Session = Depends(get_db),
):
    _validate_share_link(share_link, db)
    query = (
        db.query(Comment)
        .join(Version)
        .join(Song)
        .join(Project)
        .filter(Project.share_link == share_link)
    )
    if version_id is not None:
        query = query.filter(Comment.version_id == version_id)
    if song_id is not None:
        query = query.filter(Song.id == song_id)
    return query.order_by(Comment.timecode).all()


@router.post("/api/projects/{share_link}/comments", response_model=CommentOut, status_code=201)
def create_comment(
    share_link: str,
    req: CommentCreate,
    request: Request,
    background_tasks: BackgroundTasks,
    db: Session = Depends(get_db),
):
    project = _validate_share_link(share_link, db)

    # Verify version belongs to this project
    version = (
        db.query(Version)
        .join(Song)
        .filter(Song.project_id == project.id, Version.id == req.version_id)
        .first()
    )
    if version is None:
        raise HTTPException(status_code=404, detail="Version not found in this project")

    comment = Comment(
        version_id=req.version_id,
        timecode=req.timecode,
        author_name=req.author_name,
        text=req.text,
    )
    db.add(comment)
    db.commit()
    db.refresh(comment)

    base_url = str(request.base_url).rstrip("/")
    background_tasks.add_task(send_comment_notification, comment.id, None, base_url)

    return comment


@router.post("/api/projects/{share_link}/comments/{comment_id}/reply", response_model=ReplyOut, status_code=201)
def reply_to_comment(
    share_link: str,
    comment_id: int,
    req: ReplyCreate,
    request: Request,
    background_tasks: BackgroundTasks,
    db: Session = Depends(get_db),
):
    project = _validate_share_link(share_link, db)
    comment = _get_comment_in_project(comment_id, project, db)

    reply = Reply(
        comment_id=comment.id,
        author_name=req.author_name,
        text=req.text,
    )
    db.add(reply)
    db.commit()
    db.refresh(reply)

    base_url = str(request.base_url).rstrip("/")
    background_tasks.add_task(send_comment_notification, comment.id, reply.id, base_url)

    return reply


@router.patch("/api/projects/{share_link}/comments/{comment_id}/resolve", response_model=CommentOut)
def resolve_comment(
    share_link: str,
    comment_id: int,
    db: Session = Depends(get_db),
    admin=Depends(get_current_admin),
):
    """Resolve/unresolve a comment. Admin-only by default, configurable via settings."""
    project = _validate_share_link(share_link, db)
    comment = _get_comment_in_project(comment_id, project, db)
    comment.solved = not comment.solved
    db.commit()
    db.refresh(comment)
    return comment


@router.patch("/api/projects/{share_link}/comments/{comment_id}/resolve-client", response_model=CommentOut)
def resolve_comment_client(
    share_link: str,
    comment_id: int,
    db: Session = Depends(get_db),
):
    """Resolve/unresolve a comment via share link. Only works if clients_can_resolve is enabled."""
    settings = db.query(AppSettings).filter(AppSettings.id == 1).first()
    if settings is None or not settings.clients_can_resolve:
        raise HTTPException(status_code=403, detail="Clients are not allowed to resolve comments")

    project = _validate_share_link(share_link, db)
    comment = _get_comment_in_project(comment_id, project, db)
    comment.solved = not comment.solved
    db.commit()
    db.refresh(comment)
    return comment


@router.put("/api/projects/{share_link}/comments/{comment_id}", response_model=CommentOut)
def update_comment_client(
    share_link: str,
    comment_id: int,
    req: CommentUpdate,
    db: Session = Depends(get_db),
):
    """Edit a comment via share link (client)."""
    project = _validate_share_link(share_link, db)
    comment = _get_comment_in_project(comment_id, project, db)
    if req.text is not None:
        comment.text = req.text
    db.commit()
    db.refresh(comment)
    return comment


@router.delete("/api/projects/{share_link}/comments/{comment_id}", status_code=204)
def delete_comment_client(
    share_link: str,
    comment_id: int,
    db: Session = Depends(get_db),
):
    """Delete a comment via share link (client)."""
    project = _validate_share_link(share_link, db)
    comment = _get_comment_in_project(comment_id, project, db)
    db.delete(comment)
    db.commit()
