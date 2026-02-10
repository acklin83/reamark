"""Email notification service for Mixnote."""

import asyncio
import logging
import smtplib
from email.mime.multipart import MIMEMultipart
from email.mime.text import MIMEText

import httpx
from jinja2 import Environment

from .database import SessionLocal
from .models import AppSettings, Comment, EmailTemplate, Project, Reply, Song, Version

logger = logging.getLogger(__name__)

jinja_env = Environment(autoescape=True)


def _format_time(seconds: float) -> str:
    m = int(seconds) // 60
    s = int(seconds) % 60
    return f"{m}:{s:02d}"


def render_template(template: EmailTemplate, context: dict) -> tuple[str, str]:
    """Render subject and body from template + context. Returns (subject, html_body)."""
    subject = jinja_env.from_string(template.subject).render(**context)
    body = jinja_env.from_string(template.body_html).render(**context)
    return subject, body


def build_notification_context(
    project: Project,
    song: Song,
    version: Version,
    comment: Comment,
    reply: Reply | None = None,
    base_url: str = "",
) -> dict:
    """Build the template context dict from domain objects."""
    is_reply = reply is not None
    return {
        "project_title": project.title,
        "song_title": song.title,
        "version_label": version.label or "",
        "version_number": version.version_number,
        "author_name": reply.author_name if is_reply else comment.author_name,
        "comment_text": reply.text if is_reply else comment.text,
        "timecode": _format_time(comment.timecode),
        "share_url": f"{base_url}/{project.share_link}",
        "is_reply": is_reply,
        "parent_comment_text": comment.text if is_reply else "",
        "parent_comment_author": comment.author_name if is_reply else "",
    }


def _get_recipient_email(project: Project, settings: AppSettings) -> str | None:
    """Return notification email: per-project override or global admin email."""
    return project.notification_email or settings.email_admin_address


def _send_smtp(settings: AppSettings, to: str, subject: str, html_body: str):
    """Send via SMTP (blocking, run in thread)."""
    msg = MIMEMultipart("alternative")
    msg["Subject"] = subject
    msg["From"] = f"{settings.smtp_from_name} <{settings.smtp_from_address or settings.smtp_username}>"
    msg["To"] = to
    msg.attach(MIMEText(html_body, "html"))

    if settings.smtp_use_tls:
        server = smtplib.SMTP(settings.smtp_host, settings.smtp_port)
        server.starttls()
    else:
        server = smtplib.SMTP(settings.smtp_host, settings.smtp_port)

    if settings.smtp_username and settings.smtp_password:
        server.login(settings.smtp_username, settings.smtp_password)
    server.sendmail(msg["From"], [to], msg.as_string())
    server.quit()


async def _send_sendgrid(settings: AppSettings, to: str, subject: str, html_body: str):
    """Send via SendGrid v3 REST API."""
    async with httpx.AsyncClient() as client:
        resp = await client.post(
            "https://api.sendgrid.com/v3/mail/send",
            headers={
                "Authorization": f"Bearer {settings.email_api_key}",
                "Content-Type": "application/json",
            },
            json={
                "personalizations": [{"to": [{"email": to}]}],
                "from": {
                    "email": settings.smtp_from_address or "noreply@mixnote.app",
                    "name": settings.smtp_from_name or "Mixnote",
                },
                "subject": subject,
                "content": [{"type": "text/html", "value": html_body}],
            },
        )
        if resp.status_code not in (200, 201, 202):
            logger.error(f"SendGrid error {resp.status_code}: {resp.text}")


async def _send_mailgun(settings: AppSettings, to: str, subject: str, html_body: str):
    """Send via Mailgun REST API."""
    domain = settings.email_api_domain
    async with httpx.AsyncClient() as client:
        resp = await client.post(
            f"https://api.mailgun.net/v3/{domain}/messages",
            auth=("api", settings.email_api_key),
            data={
                "from": f"{settings.smtp_from_name or 'Mixnote'} <{settings.smtp_from_address or f'noreply@{domain}'}>",
                "to": [to],
                "subject": subject,
                "html": html_body,
            },
        )
        if resp.status_code != 200:
            logger.error(f"Mailgun error {resp.status_code}: {resp.text}")


async def send_notification(settings: AppSettings, to: str, subject: str, html_body: str):
    """Dispatch to the correct provider based on settings.email_provider."""
    provider = settings.email_provider
    if provider == "smtp":
        await asyncio.to_thread(_send_smtp, settings, to, subject, html_body)
    elif provider == "sendgrid":
        await _send_sendgrid(settings, to, subject, html_body)
    elif provider == "mailgun":
        await _send_mailgun(settings, to, subject, html_body)
    else:
        logger.debug("Email provider is 'none', skipping")
        return
    logger.info(f"Email sent to {to}: {subject}")


async def send_comment_notification(comment_id: int, reply_id: int | None, base_url: str):
    """High-level: load settings + template, render, send. Runs as background task."""
    db = SessionLocal()
    try:
        settings = db.query(AppSettings).first()
        if not settings or not settings.email_notifications_enabled:
            return
        if settings.email_provider == "none":
            return

        comment = db.query(Comment).get(comment_id)
        if not comment:
            return

        reply = db.query(Reply).get(reply_id) if reply_id else None

        version = db.query(Version).get(comment.version_id)
        if not version:
            return
        song = db.query(Song).get(version.song_id)
        if not song:
            return
        project = db.query(Project).get(song.project_id)
        if not project:
            return

        recipient = _get_recipient_email(project, settings)
        if not recipient:
            logger.debug("No recipient email configured, skipping notification")
            return

        # Get template: project-specific or first available (default)
        template = None
        if project.email_template_id:
            template = db.query(EmailTemplate).get(project.email_template_id)
        if not template:
            template = db.query(EmailTemplate).first()
        if not template:
            logger.warning("No email template found, skipping notification")
            return

        context = build_notification_context(project, song, version, comment, reply, base_url)
        subject, html_body = render_template(template, context)
        await send_notification(settings, recipient, subject, html_body)

    except Exception:
        logger.exception("Email notification failed")
    finally:
        db.close()


async def send_test_email(settings: AppSettings, to: str) -> str | None:
    """Send a test email. Returns None on success, error message on failure."""
    try:
        await send_notification(
            settings,
            to,
            "Mixnote Test-E-Mail",
            """<div style="font-family: -apple-system, sans-serif; padding: 20px; max-width: 400px; margin: 0 auto;">
  <h2 style="color: #6366f1;">Mixnote</h2>
  <p>E-Mail-Versand funktioniert! &#127881;</p>
  <p style="color: #888; font-size: 13px;">Provider: {}</p>
</div>""".format(settings.email_provider),
        )
        return None
    except Exception as e:
        logger.exception("Test email failed")
        return str(e)
