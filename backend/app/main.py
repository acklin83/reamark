import logging
import os
import sqlite3

from fastapi import FastAPI
from fastapi.middleware.cors import CORSMiddleware
from fastapi.responses import FileResponse
from fastapi.staticfiles import StaticFiles

from .database import Base, engine, SessionLocal
from .models import EmailTemplate
from .routers import admin, comments, projects, settings

logger = logging.getLogger(__name__)

DATA_DIR = os.environ.get("MIXNOTE_DATA_DIR", "/data")
DB_PATH = os.path.join(DATA_DIR, "database", "mixnote.db")


def _migrate_db():
    """Add new columns to existing tables (idempotent)."""
    if not os.path.exists(DB_PATH):
        return
    conn = sqlite3.connect(DB_PATH)
    migrations = [
        "ALTER TABLE app_settings ADD COLUMN email_provider TEXT DEFAULT 'none'",
        "ALTER TABLE app_settings ADD COLUMN email_notifications_enabled BOOLEAN DEFAULT 0",
        "ALTER TABLE app_settings ADD COLUMN email_admin_address TEXT",
        "ALTER TABLE app_settings ADD COLUMN smtp_host TEXT",
        "ALTER TABLE app_settings ADD COLUMN smtp_port INTEGER DEFAULT 587",
        "ALTER TABLE app_settings ADD COLUMN smtp_username TEXT",
        "ALTER TABLE app_settings ADD COLUMN smtp_password TEXT",
        "ALTER TABLE app_settings ADD COLUMN smtp_use_tls BOOLEAN DEFAULT 1",
        "ALTER TABLE app_settings ADD COLUMN smtp_from_address TEXT",
        "ALTER TABLE app_settings ADD COLUMN smtp_from_name TEXT DEFAULT 'Mixnote'",
        "ALTER TABLE app_settings ADD COLUMN email_api_key TEXT",
        "ALTER TABLE app_settings ADD COLUMN email_api_domain TEXT",
        "ALTER TABLE projects ADD COLUMN notification_email TEXT",
        "ALTER TABLE projects ADD COLUMN email_template_id INTEGER REFERENCES email_templates(id)",
        "ALTER TABLE projects ADD COLUMN notifications_enabled BOOLEAN DEFAULT 1",
        "ALTER TABLE app_settings ADD COLUMN email_batch_enabled BOOLEAN DEFAULT 0",
        "ALTER TABLE app_settings ADD COLUMN email_batch_delay_minutes INTEGER DEFAULT 5",
    ]
    for sql in migrations:
        try:
            conn.execute(sql)
        except sqlite3.OperationalError:
            pass  # Column already exists
    conn.commit()
    conn.close()


def _seed_default_template():
    """Insert default email templates if none exist."""
    db = SessionLocal()
    try:
        if db.query(EmailTemplate).count() == 0:
            # German template
            tpl_de = EmailTemplate(
                name="Standard-Benachrichtigung (Deutsch)",
                subject="Neuer {{ 'Reply' if is_reply else 'Kommentar' }} – {{ project_title }} / {{ song_title }}",
                body_html="""<div style="font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', sans-serif; max-width: 600px; margin: 0 auto; padding: 20px; color: #333;">
  <p style="font-size: 16px; font-weight: bold; color: #6366f1; margin: 0 0 4px 0;">{{ project_title }}</p>
  <p style="color: #888; margin-top: 0; font-size: 14px;">{{ song_title }} &middot; v{{ version_number }} {{ version_label }}</p>
  <hr style="border: none; border-top: 1px solid #e5e7eb; margin: 16px 0;">
  {% if is_reply %}
  <p style="color: #888; font-size: 14px;">Antwort auf Kommentar von <strong>{{ parent_comment_author }}</strong>:</p>
  <div style="border-left: 3px solid #6366f1; padding-left: 12px; margin: 8px 0; color: #666; font-size: 14px;">
    {{ parent_comment_text }}
  </div>
  {% endif %}
  <div style="background: #f9fafb; padding: 16px; border-radius: 8px; margin: 16px 0;">
    <p style="margin: 0 0 8px 0; font-size: 14px;"><strong>{{ author_name }}</strong> <span style="color: #888; font-size: 13px;">@ {{ timecode }}</span></p>
    <p style="margin: 0; font-size: 15px;">{{ comment_text }}</p>
  </div>
  <p style="font-size: 14px;"><a href="{{ share_url }}" style="color: #6366f1; text-decoration: none;">In Mixnote &ouml;ffnen &rarr;</a></p>
</div>""",
            )
            # English template
            tpl_en = EmailTemplate(
                name="Standard Notification (English)",
                subject="New {{ 'Reply' if is_reply else 'Comment' }} – {{ project_title }} / {{ song_title }}",
                body_html="""<div style="font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', sans-serif; max-width: 600px; margin: 0 auto; padding: 20px; color: #333;">
  <p style="font-size: 16px; font-weight: bold; color: #6366f1; margin: 0 0 4px 0;">{{ project_title }}</p>
  <p style="color: #888; margin-top: 0; font-size: 14px;">{{ song_title }} &middot; v{{ version_number }} {{ version_label }}</p>
  <hr style="border: none; border-top: 1px solid #e5e7eb; margin: 16px 0;">
  {% if is_reply %}
  <p style="color: #888; font-size: 14px;">Reply to comment by <strong>{{ parent_comment_author }}</strong>:</p>
  <div style="border-left: 3px solid #6366f1; padding-left: 12px; margin: 8px 0; color: #666; font-size: 14px;">
    {{ parent_comment_text }}
  </div>
  {% endif %}
  <div style="background: #f9fafb; padding: 16px; border-radius: 8px; margin: 16px 0;">
    <p style="margin: 0 0 8px 0; font-size: 14px;"><strong>{{ author_name }}</strong> <span style="color: #888; font-size: 13px;">@ {{ timecode }}</span></p>
    <p style="margin: 0; font-size: 15px;">{{ comment_text }}</p>
  </div>
  <p style="font-size: 14px;"><a href="{{ share_url }}" style="color: #6366f1; text-decoration: none;">Open in Mixnote &rarr;</a></p>
</div>""",
            )
            db.add(tpl_de)
            db.add(tpl_en)
            db.commit()
            logger.info("Seeded default email templates (DE + EN)")
    finally:
        db.close()


Base.metadata.create_all(bind=engine)
_migrate_db()
_seed_default_template()

app = FastAPI(title="Mixnote", version="0.1.0")

app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],  # tighten for production
    allow_methods=["*"],
    allow_headers=["*"],
)

app.include_router(admin.router)
app.include_router(projects.router)
app.include_router(comments.router)
app.include_router(settings.router)

FRONTEND_DIR = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "..", "frontend"))


@app.get("/health")
def health():
    return {"status": "ok"}


@app.get("/admin")
@app.get("/admin/{path:path}")
def admin_page(path: str = ""):
    return FileResponse(os.path.join(FRONTEND_DIR, "admin", "index.html"))


# Client share link page (must be after all /api and /admin routes)
@app.get("/{share_link}")
def client_page(share_link: str):
    return FileResponse(os.path.join(FRONTEND_DIR, "client", "index.html"))


# Serve static frontend files (js, css)
app.mount("/admin-static", StaticFiles(directory=os.path.join(FRONTEND_DIR, "admin")), name="admin-static")
app.mount("/client-static", StaticFiles(directory=os.path.join(FRONTEND_DIR, "client")), name="client-static")
