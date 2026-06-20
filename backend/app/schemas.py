from datetime import datetime
from pydantic import BaseModel, Field


# --- Auth ---

class SetupRequest(BaseModel):
    username: str = Field(min_length=3, max_length=50)
    password: str = Field(min_length=8)


class LoginRequest(BaseModel):
    username: str
    password: str


class TokenResponse(BaseModel):
    access_token: str
    token_type: str = "bearer"


# --- Project ---

class ProjectCreate(BaseModel):
    title: str = Field(min_length=1, max_length=200)


class ProjectUpdate(BaseModel):
    title: str | None = Field(default=None, min_length=1, max_length=200)
    notification_email: str | None = None
    email_template_id: int | None = None
    notifications_enabled: bool | None = None


class ProjectSummary(BaseModel):
    id: str
    title: str
    share_link: str
    song_count: int
    comment_count: int
    notifications_enabled: bool = True
    created_at: datetime
    updated_at: datetime

    model_config = {"from_attributes": True}


class ProjectDetail(BaseModel):
    id: str
    title: str
    share_link: str
    notification_email: str | None = None
    email_template_id: int | None = None
    notifications_enabled: bool = True
    created_at: datetime
    updated_at: datetime
    songs: list["SongOut"]

    model_config = {"from_attributes": True}


# --- Song ---

class SongCreate(BaseModel):
    title: str = Field(min_length=1, max_length=200)


class SongOut(BaseModel):
    id: int
    title: str
    position: int
    created_at: datetime
    version_count: int = 0
    comment_count: int = 0
    open_count: int = 0
    versions: list["VersionOut"]

    model_config = {"from_attributes": True}


# --- Version ---

class VersionOut(BaseModel):
    id: int
    version_number: int
    label: str
    original_filename: str
    favourite: bool
    created_at: datetime

    model_config = {"from_attributes": True}


# --- Comment ---

class CommentCreate(BaseModel):
    version_id: int
    timecode: float = Field(ge=0)
    author_name: str = Field(min_length=1, max_length=100)
    text: str = Field(min_length=1, max_length=5000)


class CommentUpdate(BaseModel):
    text: str | None = None
    solved: bool | None = None


class ReplyCreate(BaseModel):
    author_name: str = Field(min_length=1, max_length=100)
    text: str = Field(min_length=1, max_length=5000)


class ReplyOut(BaseModel):
    id: int
    comment_id: int
    author_name: str
    text: str
    created_at: datetime

    model_config = {"from_attributes": True}


class CommentOut(BaseModel):
    id: int
    version_id: int
    timecode: float
    author_name: str
    text: str
    solved: bool = False
    replies: list[ReplyOut] = []
    created_at: datetime

    model_config = {"from_attributes": True}


# --- Settings ---

class SettingsUpdate(BaseModel):
    accent_color: str | None = Field(default=None, pattern=r'^#[0-9A-Fa-f]{6}$')
    dark_900: str | None = Field(default=None, pattern=r'^#[0-9A-Fa-f]{6}$')
    dark_800: str | None = Field(default=None, pattern=r'^#[0-9A-Fa-f]{6}$')
    dark_700: str | None = Field(default=None, pattern=r'^#[0-9A-Fa-f]{6}$')
    dark_600: str | None = Field(default=None, pattern=r'^#[0-9A-Fa-f]{6}$')
    text_color: str | None = Field(default=None, pattern=r'^#[0-9A-Fa-f]{6}$')
    waveform_color: str | None = Field(default=None, pattern=r'^#[0-9A-Fa-f]{6}$')
    waveform_progress_color: str | None = Field(default=None, pattern=r'^#[0-9A-Fa-f]{6}$')
    light_accent_color: str | None = Field(default=None, pattern=r'^#[0-9A-Fa-f]{6}$')
    light_bg_900: str | None = Field(default=None, pattern=r'^#[0-9A-Fa-f]{6}$')
    light_bg_800: str | None = Field(default=None, pattern=r'^#[0-9A-Fa-f]{6}$')
    light_bg_700: str | None = Field(default=None, pattern=r'^#[0-9A-Fa-f]{6}$')
    light_bg_600: str | None = Field(default=None, pattern=r'^#[0-9A-Fa-f]{6}$')
    light_text_color: str | None = Field(default=None, pattern=r'^#[0-9A-Fa-f]{6}$')
    light_waveform_color: str | None = Field(default=None, pattern=r'^#[0-9A-Fa-f]{6}$')
    light_waveform_progress_color: str | None = Field(default=None, pattern=r'^#[0-9A-Fa-f]{6}$')
    logo_height: int | None = Field(default=None, ge=16, le=120)
    site_name: str | None = Field(default=None, min_length=1, max_length=100)
    clients_can_resolve: bool | None = None
    # Email settings
    email_provider: str | None = Field(default=None, pattern=r'^(none|smtp|sendgrid|mailgun)$')
    email_notifications_enabled: bool | None = None
    email_admin_address: str | None = None
    smtp_host: str | None = None
    smtp_port: int | None = Field(default=None, ge=1, le=65535)
    smtp_username: str | None = None
    smtp_password: str | None = None
    smtp_use_tls: bool | None = None
    smtp_from_address: str | None = None
    smtp_from_name: str | None = None
    email_api_key: str | None = None
    email_api_domain: str | None = None
    email_batch_enabled: bool | None = None
    email_batch_delay_minutes: int | None = Field(default=None, ge=1, le=60)


class SettingsOut(BaseModel):
    accent_color: str
    dark_900: str
    dark_800: str
    dark_700: str
    dark_600: str
    text_color: str
    waveform_color: str
    waveform_progress_color: str
    light_accent_color: str
    light_bg_900: str
    light_bg_800: str
    light_bg_700: str
    light_bg_600: str
    light_text_color: str
    light_waveform_color: str
    light_waveform_progress_color: str
    logo_url: str | None = None
    logo_height: int = 32
    site_name: str = "ReaMark"
    favicon_url: str | None = None
    clients_can_resolve: bool = False

    model_config = {"from_attributes": True}


# --- Client view ---

class ClientProjectOut(BaseModel):
    title: str
    songs: list[SongOut]

    model_config = {"from_attributes": True}


# --- Admin Settings (with email config) ---

class AdminSettingsOut(SettingsOut):
    email_provider: str = "none"
    email_notifications_enabled: bool = False
    email_admin_address: str | None = None
    smtp_host: str | None = None
    smtp_port: int = 587
    smtp_username: str | None = None
    smtp_use_tls: bool = True
    smtp_from_address: str | None = None
    smtp_from_name: str = "ReaMark"
    smtp_password_set: bool = False
    email_api_key_set: bool = False
    email_api_domain: str | None = None
    email_batch_enabled: bool = False
    email_batch_delay_minutes: int = 5


# --- Email Template ---

class EmailTemplateCreate(BaseModel):
    name: str = Field(min_length=1, max_length=100)
    subject: str = Field(min_length=1, max_length=500)
    body_html: str = Field(min_length=1)


class EmailTemplateUpdate(BaseModel):
    name: str | None = Field(default=None, min_length=1, max_length=100)
    subject: str | None = Field(default=None, min_length=1, max_length=500)
    body_html: str | None = None


class EmailTemplateOut(BaseModel):
    id: int
    name: str
    subject: str
    body_html: str
    created_at: datetime
    updated_at: datetime

    model_config = {"from_attributes": True}
