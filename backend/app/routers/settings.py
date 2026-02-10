import os
import shutil

from fastapi import APIRouter, Depends, File, HTTPException, UploadFile, status
from fastapi.responses import FileResponse
from sqlalchemy.orm import Session

from ..auth import get_current_admin
from ..database import get_db
from ..models import AdminUser, AppSettings
from ..email_service import send_test_email
from ..schemas import AdminSettingsOut, SettingsOut, SettingsUpdate

router = APIRouter(tags=["settings"])

LOGO_DIR = os.path.join(os.path.dirname(__file__), "..", "..", "..", "data", "uploads", "logo")
ALLOWED_IMAGE_EXTENSIONS = {".png", ".jpg", ".jpeg"}


def _get_or_create_settings(db: Session) -> AppSettings:
    settings = db.query(AppSettings).filter(AppSettings.id == 1).first()
    if settings is None:
        settings = AppSettings(id=1)
        db.add(settings)
        db.commit()
        db.refresh(settings)
    return settings


def _to_settings_out(settings: AppSettings) -> dict:
    logo_url = "/api/logo" if settings.logo_path and os.path.isfile(settings.logo_path) else None
    return {
        "accent_color": settings.accent_color,
        "dark_900": settings.dark_900,
        "dark_800": settings.dark_800,
        "dark_700": settings.dark_700,
        "dark_600": settings.dark_600,
        "text_color": settings.text_color,
        "waveform_color": settings.waveform_color,
        "waveform_progress_color": settings.waveform_progress_color,
        "light_accent_color": settings.light_accent_color or "#4f46e5",
        "light_bg_900": settings.light_bg_900 or "#ffffff",
        "light_bg_800": settings.light_bg_800 or "#f9fafb",
        "light_bg_700": settings.light_bg_700 or "#f3f4f6",
        "light_bg_600": settings.light_bg_600 or "#e5e7eb",
        "light_text_color": settings.light_text_color or "#111827",
        "light_waveform_color": settings.light_waveform_color or "#d1d5db",
        "light_waveform_progress_color": settings.light_waveform_progress_color or "#4f46e5",
        "logo_url": logo_url,
        "logo_height": settings.logo_height,
        "clients_can_resolve": settings.clients_can_resolve,
    }


def _to_admin_settings_out(settings: AppSettings) -> dict:
    base = _to_settings_out(settings)
    base.update({
        "email_provider": settings.email_provider or "none",
        "email_notifications_enabled": settings.email_notifications_enabled or False,
        "email_admin_address": settings.email_admin_address,
        "smtp_host": settings.smtp_host,
        "smtp_port": settings.smtp_port or 587,
        "smtp_username": settings.smtp_username,
        "smtp_use_tls": settings.smtp_use_tls if settings.smtp_use_tls is not None else True,
        "smtp_from_address": settings.smtp_from_address,
        "smtp_from_name": settings.smtp_from_name or "Mixnote",
        "smtp_password_set": bool(settings.smtp_password),
        "email_api_key_set": bool(settings.email_api_key),
        "email_api_domain": settings.email_api_domain,
    })
    return base


@router.get("/api/settings", response_model=SettingsOut)
def get_settings(db: Session = Depends(get_db)):
    settings = _get_or_create_settings(db)
    return _to_settings_out(settings)


@router.get("/admin/settings", response_model=AdminSettingsOut)
def get_admin_settings(
    _admin: AdminUser = Depends(get_current_admin),
    db: Session = Depends(get_db),
):
    settings = _get_or_create_settings(db)
    return _to_admin_settings_out(settings)


EMAIL_FIELDS = [
    "email_provider", "email_notifications_enabled", "email_admin_address",
    "smtp_host", "smtp_port", "smtp_username", "smtp_use_tls",
    "smtp_from_address", "smtp_from_name", "email_api_domain",
]
# Sensitive fields: only update if non-empty string is sent
EMAIL_SENSITIVE_FIELDS = ["smtp_password", "email_api_key"]


@router.put("/admin/settings", response_model=AdminSettingsOut)
def update_settings(
    req: SettingsUpdate,
    _admin: AdminUser = Depends(get_current_admin),
    db: Session = Depends(get_db),
):
    settings = _get_or_create_settings(db)
    for field in [
        "accent_color", "dark_900", "dark_800", "dark_700", "dark_600",
        "text_color", "waveform_color", "waveform_progress_color",
        "light_accent_color", "light_bg_900", "light_bg_800", "light_bg_700", "light_bg_600",
        "light_text_color", "light_waveform_color", "light_waveform_progress_color",
        "logo_height", "clients_can_resolve",
    ] + EMAIL_FIELDS:
        value = getattr(req, field, None)
        if value is not None:
            setattr(settings, field, value)
    # Sensitive fields: only overwrite if a non-empty string is provided
    for field in EMAIL_SENSITIVE_FIELDS:
        value = getattr(req, field, None)
        if value is not None and value.strip():
            setattr(settings, field, value)
    db.commit()
    db.refresh(settings)
    return _to_admin_settings_out(settings)


@router.post("/admin/settings/test-email")
async def test_email(
    _admin: AdminUser = Depends(get_current_admin),
    db: Session = Depends(get_db),
):
    settings = _get_or_create_settings(db)
    to = settings.email_admin_address
    if not to:
        raise HTTPException(status_code=400, detail="No admin email address configured")
    if settings.email_provider == "none":
        raise HTTPException(status_code=400, detail="No email provider configured")
    error = await send_test_email(settings, to)
    if error:
        raise HTTPException(status_code=500, detail=f"Email failed: {error}")
    return {"ok": True}


@router.post("/admin/settings/logo", response_model=SettingsOut)
def upload_logo(
    file: UploadFile = File(...),
    _admin: AdminUser = Depends(get_current_admin),
    db: Session = Depends(get_db),
):
    ext = os.path.splitext(file.filename or "")[1].lower()
    if ext not in ALLOWED_IMAGE_EXTENSIONS:
        raise HTTPException(status_code=400, detail="Only PNG and JPG images allowed")

    # Check size (2MB max)
    file.file.seek(0, 2)
    size = file.file.tell()
    file.file.seek(0)
    if size > 2 * 1024 * 1024:
        raise HTTPException(status_code=400, detail="Logo must be under 2MB")

    settings = _get_or_create_settings(db)

    # Delete old logo
    if settings.logo_path and os.path.isfile(settings.logo_path):
        os.remove(settings.logo_path)

    os.makedirs(LOGO_DIR, exist_ok=True)
    dest = os.path.join(LOGO_DIR, f"logo{ext}")
    with open(dest, "wb") as f:
        shutil.copyfileobj(file.file, f)

    settings.logo_path = dest
    db.commit()
    db.refresh(settings)
    return _to_settings_out(settings)


@router.delete("/admin/settings/logo", status_code=status.HTTP_204_NO_CONTENT)
def delete_logo(
    _admin: AdminUser = Depends(get_current_admin),
    db: Session = Depends(get_db),
):
    settings = _get_or_create_settings(db)
    if settings.logo_path and os.path.isfile(settings.logo_path):
        os.remove(settings.logo_path)
    settings.logo_path = None
    db.commit()


@router.get("/api/logo")
def get_logo(db: Session = Depends(get_db)):
    settings = _get_or_create_settings(db)
    if not settings.logo_path or not os.path.isfile(settings.logo_path):
        raise HTTPException(status_code=404, detail="No logo set")
    ext = os.path.splitext(settings.logo_path)[1].lower()
    media = {".png": "image/png", ".jpg": "image/jpeg", ".jpeg": "image/jpeg"}.get(ext, "image/png")
    return FileResponse(settings.logo_path, media_type=media)
