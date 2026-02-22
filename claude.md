Never use any agents for this project to save tokens - only when user explicitly requests it.
Be very prudent with token use!

# Mixnote System - Project Documentation

## Project Overview
A self-hosted audio review platform for Störsender-Studio that allows clients to listen to mix versions and leave timeline-based comments via secure share links.

## Core Requirements

### Functionality
- **Projects**: Container for multiple songs (e.g., "Album XY - Mastering")
- **Songs**: Individual tracks within a project
- **Versions**: Multiple mix versions per song (v1, v2, v3, auto-incrementing)
- **Comments**: Timeline-based comments with precise timecode (@0:45)
- **Share Links**: UUID-based, one link per project (gives access to all songs/versions)
- **Admin Interface**: Upload management, project organization
- **Client View**: Clean, minimal player interface for customers

### Data Model
```
Project
  ├─ id (UUID)
  ├─ title
  ├─ share_link (UUID)
  ├─ created_at
  └─ Songs[]
       ├─ id
       ├─ title
       ├─ position (order in project)
       └─ Versions[]
            ├─ id
            ├─ version_number (v1, v2, v3...)
            ├─ file_path
            ├─ created_at
            └─ Comments[]
                 ├─ id
                 ├─ timecode (seconds, float)
                 ├─ author_name
                 ├─ text
                 ├─ solved (boolean)
                 ├─ created_at
                 └─ Replies[]
                      ├─ id
                      ├─ author_name
                      ├─ text
                      └─ created_at
```

### Key Design Decisions
- **Comments are per-version**: Each version has its own comment thread
- **No user accounts for clients**: Share link is the only auth needed
- **Admin uses simple login**: Basic auth for studio owner, password set during setup
- **Download enabled**: Clients can download approved versions
- **File formats**: WAV, MP3, FLAC (other formats rejected at upload)

## Technical Stack

### Backend
- **Framework**: FastAPI (Python 3.11+)
- **Database**: SQLite (sufficient for use case, simple backup)
- **ORM**: SQLAlchemy
- **Validation**: Pydantic models
- **Auth**: JWT for admin, UUID share links for clients

### Frontend
- **Framework**: Vanilla JavaScript (no build step needed)
- **Audio Player**: Wavesurfer.js (waveform visualization + playback)
- **Styling**: TailwindCSS via CDN
- **Comments**: Real-time display with jump-to-timecode functionality

### Deployment
- **Container**: Docker + Docker-Compose
- **Target**: Synology DiskStation (10GbE network)
- **Reverse Proxy**: nginx with SSL/TLS (Let's Encrypt)
- **Storage**: Volume mount to DiskStation shared folder

## Infrastructure

### DiskStation Setup
- **Network**: 10GbE connection (bandwidth not a bottleneck)
- **Docker**: Native Docker support via Container Manager
- **Storage**: Dedicated volume for audio files and database
- **Domain**: mix.stoersender.ch (subdomain of existing stoersender.ch)
- **SSL**: Let's Encrypt via Synology DSM
- **Reverse Proxy**: nginx routing to Docker container port 8000

### URL Structure
- **Client Share Links**: `https://mix.stoersender.ch/{short-uuid}`
  - Short UUID format: 12 characters (e.g., `7f3a9c2d4b8e`)
  - Provides 3+ trillion unique combinations
  - Example: `https://mix.stoersender.ch/a1b2c3d4e5f6`
- **Admin Interface**: `https://mix.stoersender.ch/admin`
  - Login protected (JWT authentication)
  - Password set during initial setup (not default password)

### Directory Structure
```
mixnote/
├── docker-compose.yml
├── nginx/
│   ├── Dockerfile
│   └── nginx.conf
├── backend/
│   ├── Dockerfile
│   ├── requirements.txt
│   ├── app/
│   │   ├── main.py
│   │   ├── models.py
│   │   ├── schemas.py
│   │   ├── database.py
│   │   ├── auth.py
│   │   └── routers/
│   │       ├── admin.py
│   │       ├── projects.py
│   │       └── comments.py
│   └── tests/
├── frontend/
│   ├── admin/
│   │   ├── index.html
│   │   ├── css/
│   │   └── js/
│   └── client/
│       ├── index.html
│       ├── css/
│       └── js/
├── reaper/
│   └── mixnote_comments.lua  # REAPER ReaImGui integration script
└── data/
    ├── uploads/          # Audio files
    ├── database/         # SQLite DB
    └── static/           # Generated waveforms, thumbnails
```

## Interface Design

### Admin Interface
**Projects Overview:**
- Table view: Title, Items (song count), Comments, Created, Modified
- Actions: View, Upload Version, Settings, Delete
- "New Project" button

**Upload Flow:**
1. Select existing project or create new
2. Select song (existing or new)
3. Auto-increment version number
4. Drag-drop audio file
5. Optional notes/description

**Project Detail:**
- List all songs with version counts
- Quick actions per song
- Share link display with copy button
- Access management (optional future: expiration dates)

### Client Interface (Share Link View)
**Layout:**
- Project title and metadata at top
- Primary audio player with waveform
- Version selector dropdown
- Song list (if multiple songs in project)
- Comment timeline on waveform
- Comment input with current timecode
- Comment thread below player

**Player Features:**
- Play/pause, scrubbing
- Waveform with comment markers
- Click waveform to jump
- Click comment marker to jump and highlight comment
- Download button (if enabled for version)

**Comment Features:**
- Timecode auto-links: `@0:45` format
- Author name (required)
- Click timecode in comment to jump
- Chronological display
- Visual markers on waveform

## Development Phases

### Phase 1: Core Backend + Database
- SQLAlchemy models
- FastAPI endpoints (CRUD)
- Authentication (admin JWT, share link validation)
- File upload handling
- Database migrations

### Phase 2: Admin Interface
- Login page
- Projects overview
- Upload interface
- Project management

### Phase 3: Client Interface
- Share link handler
- Wavesurfer.js integration
- Comment system
- Version switching

### Phase 4: Polish & Deploy
- Waveform generation (ffmpeg)
- Error handling
- Mobile responsiveness
- Docker optimization
- DiskStation deployment

## API Endpoints (Draft)

### Admin Routes (JWT protected)
```
POST   /admin/auth/login              # Initial setup creates admin user
POST   /admin/auth/setup              # One-time setup endpoint
GET    /admin/projects
POST   /admin/projects
GET    /admin/projects/{id}
PUT    /admin/projects/{id}
DELETE /admin/projects/{id}
PUT    /api/projects/{uuid}/comments/{id}          # Edit comment (client, no auth)
DELETE /api/projects/{uuid}/comments/{id}          # Delete comment (client, no auth)
POST   /admin/projects/{id}/songs
POST   /admin/songs/{id}/versions
POST   /admin/upload
```

### Client Routes (Share link validated)
```
GET    /{short-uuid}                                    # Main client view
GET    /api/projects/{short-uuid}                       # Project metadata
GET    /api/projects/{short-uuid}/songs/{song_id}       # Song details
GET    /api/projects/{short-uuid}/versions              # All versions
GET    /api/audio/{version_id}                          # Audio file streaming
GET    /api/versions/{version_id}/peaks                 # Waveform peak data (JSON, lazy-generated + cached)
POST   /api/projects/{short-uuid}/comments              # Post comment
GET    /api/projects/{short-uuid}/comments              # Get all comments
```

## Security Considerations
- Admin password set during initial setup (not hardcoded)
- Password hashing with bcrypt
- JWT with expiration for admin sessions
- Share links: 12-character short UUID (cryptographically random)
- Rate limiting on comment endpoints
- File upload validation (type: WAV/MP3/FLAC only, size limits)
- Sanitize user input (comment text)
- CORS configuration for production
- HTTPS only in production (mix.stoersender.ch)

## Performance Optimization
- Audio streaming (range requests)
- Waveform caching (generate once, serve static)
- Database indexing (share_link, project_id, song_id)
- Nginx serving static files
- Optional: Multiple quality audio files (320kbps review, 128kbps mobile)

## Future Enhancements (Out of Scope for V1)
- Email notifications on new comments
- Multiple file formats per version
- Stem playback (separate tracks)
- A/B comparison mode
- Mobile app
- Real-time updates (WebSocket)
- Analytics (play counts, engagement)
- Link expiration dates
- Password protection per project

## Reference Implementation
This system is inspired by mixup.audio's interface:
- Clean, minimal design (dark theme)
- Waveform-centric player
- Inline commenting with timecode links
- Version management
- Project/playlist organization

## Deployment Instructions

### Initial Setup
```bash
# First start - creates admin account
docker-compose up -d

# Visit setup page
https://mix.stoersender.ch/admin/setup
# Set admin password (bcrypt hashed, stored in DB)

# After setup, setup endpoint is disabled
```

### Local Development
```bash
# Clone/create project
cd ~/Projects/mixnote

# Start services
docker-compose up --build

# Access
Admin: http://localhost:8000/admin
Client: http://localhost:8000/{short-uuid}
API Docs: http://localhost:8000/docs
```

### Synology Deployment
```bash
# From development machine
rsync -avz ~/Projects/mixnote/ user@diskstation:/volume1/docker/mixnote/

# On DiskStation
ssh user@diskstation
cd /volume1/docker/mixnote
docker-compose up -d

# Configure in Synology DSM:
# 1. Control Panel → Security → Certificate → Add Let's Encrypt for mix.stoersender.ch
# 2. Control Panel → Application Portal → Reverse Proxy → Create:
#    Source: mix.stoersender.ch (HTTPS, port 443)
#    Destination: localhost (HTTP, port 8000)
```

### Reverse Proxy Configuration (Synology DSM)
```
Source:
  Protocol: HTTPS
  Hostname: mix.stoersender.ch
  Port: 443

Destination:
  Protocol: HTTP
  Hostname: localhost
  Port: 8000

Custom Headers:
  X-Real-IP: $remote_addr
  X-Forwarded-For: $proxy_add_x_forwarded_for
  X-Forwarded-Proto: $scheme
```

## Contact & Studio Info
**Studio**: Störsender-Studio  
**Owner**: Frank  
**Location**: Switzerland  
**Network**: 10GbE DiskStation  
**Use Case**: Professional audio production review workflow

## Comment System

### Data Model
Comments support nested replies (one level deep) and a resolved/solved status:

```
Comment
  ├─ id, version_id, timecode, author_name, text, solved (bool)
  ├─ created_at
  └─ Replies[]
       ├─ id, comment_id, author_name, text
       └─ created_at
```

### Comment API Endpoints
```
GET    /api/projects/{uuid}/comments                        # Returns comments with replies + solved status
POST   /api/projects/{uuid}/comments                        # Create root comment
POST   /api/projects/{uuid}/comments/{id}/reply             # Reply to comment (one level)
PATCH  /api/projects/{uuid}/comments/{id}/resolve           # Toggle resolved (admin JWT required)
PATCH  /api/projects/{uuid}/comments/{id}/resolve-client    # Toggle resolved (share link, requires clients_can_resolve setting)
```

### Settings
- `clients_can_resolve` (boolean, default: false) - Controls whether clients with share links can resolve/unresolve comments. Configurable via `PUT /admin/settings`.

## REAPER Integration

### Script: `reaper/mixnote.lua`
A ReaImGui-based script for managing Mixnote comments directly from REAPER.

**Requirements:**
- REAPER 6.0+
- ReaImGui extension (install via ReaPack)

**Installation:**
1. Copy `reaper/mixnote.lua` to your REAPER Scripts folder
2. In REAPER: Actions → Show Action List → Load ReaScript
3. Assign a keyboard shortcut if desired

**Features:**
- Admin login (JWT authentication via username/password)
- Load project by share link
- Song/version selection dropdowns
- **Waveform display**: Peak data loaded from backend, rendered with ImGui DrawList
  - Comment markers (colored vertical lines: amber=open, green=resolved)
  - Real-time playhead tracking (follows REAPER cursor/play position)
  - Click on waveform to seek (with calibration offset)
  - Hover over marker shows tooltip with comment text
- **Autoplay toggle**: Controls whether clicking waveform/timecode seeks+plays or only seeks
- **Calibration**: Set song start offset from REAPER cursor position (persisted per song/version)
- **New comments**: Author name + text, timecode from current REAPER cursor position (relative to calibration offset)
- **Comment list**: Filterable (All / Open / Resolved), sorted by timecode
- **Jump**: Sets REAPER edit cursor to comment timecode (respects autoplay toggle)
- **Reply**: Inline reply input per comment
- **Resolve/Unresolve**: Toggle via admin JWT (requires login)
- **Edit/Delete**: Comments editable and deletable by admin
- **Favourite**: Toggle favourite version (star button)
- **Refresh**: Reload comments from server

**Persisted state** (via `reaper.ExtState`):
- Server URL, username, author name, last share link
- Calibration offsets per song/version
- Autoplay toggle state

## Session Log

### 2025-01-31: WebView Experiment (CANCELLED)
- **Branch:** `webview`
- **Attempted:** Replace Lua/ImGui script with Python WebView approach (local HTTP + WebSocket server opening a browser window)
- **Result:** REAPER has no built-in browser/webview. Opening an external browser provides no advantage over just using the Mixnote web interface directly. Cancelled after ~1 hour.
- **Decision:** Keep the Lua/ImGui script (`mixnote_comments.lua`) as the REAPER integration. Improve it instead of replacing it.
- **Also considered C++ REAPER extension** - rejected as overkill for a comment/form UI.

### 2025-02-01: Lua/ImGui Beautification (DONE)
- **Branch:** `lua-beautification`
- **Goal:** Visually improve `mixnote_comments.lua` to look closer to the Mixnote website. Dark theme, better colors, spacing, comment card styling.
- **Approach:** Stay with Lua/ReaImGui, use style customization (PushStyleColor, PushStyleVar, fonts, draw lists).
- **Result:** Website-style dark theme, styled comment cards, accent colors, improved spacing.

### 2025-02-01: REAPER Script Fixes + Client Edit/Delete
- **Changes:**
  1. **REAPER author_name fix**: Default comment author was "Guest" instead of logged-in username. Fixed by always setting `author_name = username` on login (ExtState had stale "Guest" value).
  2. **REAPER Reply button alignment**: Reply button right-aligned to match Delete button.
  3. **Client comment Edit/Delete**: Added public (no JWT) endpoints `PUT` and `DELETE` on `/api/projects/{share_link}/comments/{comment_id}`. Added Edit/Delete buttons to client comment cards.
  4. **Client reply form fix**: Reply form restructured to vertical layout to prevent overflow.
- **Files modified:**
  - `reaper/mixnote.lua` — author_name fix, Reply button alignment
  - `backend/app/routers/comments.py` — client edit/delete endpoints
  - `backend/app/schemas.py` — CommentUpdate schema
  - `frontend/client/js/client.js` — edit/delete UI, reply form layout fix

### 2025-02-01: Logout Reset + Edit Line Breaks
- **Changes:**
  1. **Logout reset**: On logout, all state cleared so UI returns to initial login view.
  2. **Edit mode line breaks**: Edit textarea auto-sizes height based on newlines.
- **Files modified:**
  - `reaper/mixnote.lua`
- **Note:** "Add Song" / "Upload New Version" from REAPER rejected — no file browser in ReaImGui.

### 2025-02-02: Waveform Player + Autoplay Toggle
- **Changes:**
  1. **Backend peak generation**: New `audio_utils.py` with pydub-based peak generation (800 samples, lazy + cached as `.peaks.json`). New endpoint `GET /api/versions/{id}/peaks`.
  2. **Dockerfile**: Added ffmpeg dependency for audio processing.
  3. **Lua waveform rendering**: Peak data loaded per version, rendered with ImGui DrawList. Performance-optimized (downsampled to pixel width). Comment markers as colored lines, real-time playhead.
  4. **Autoplay toggle**: Checkbox on offset line controls seek+play vs seek-only behavior for waveform clicks and timecode buttons. Persisted in ExtState.
  5. **Marker tooltips**: Hovering over comment markers on waveform shows tooltip with timecode, author and text.
  6. **Login section**: Removed collapsible TreeNode, now always visible.
  7. **Renamed**: `mixnote_v2.lua` → `mixnote.lua`
- **Files modified:**
  - `backend/Dockerfile` — ffmpeg install
  - `backend/requirements.txt` — pydub
  - `backend/app/audio_utils.py` — NEW: peak generation + caching
  - `backend/app/routers/projects.py` — peaks endpoint
  - `reaper/mixnote.lua` — waveform, autoplay, tooltips, layout

### 2026-02-10: Light-Mode Fix
- **Changes:**
  1. **Light-mode text colors**: Added CSS overrides for `text-gray-200` through `text-gray-600` to map to dark equivalents in light mode.
  2. **Hover colors**: `hover:text-white` remapped to dark color in light mode.
  3. **Reply box background**: Hardcoded `background:#2d2d2d` overridden to `bg700` in light mode.
  4. **Waveform live update**: Theme toggle now updates WaveSurfer colors (waveColor, progressColor, cursorColor) without reload.
  5. **Created `ROADMAP.md`**: Consolidated project status and future plans.
- **Files modified:**
  - `frontend/admin/js/admin.js` — light-mode CSS overrides, waveform theme update
  - `frontend/client/js/client.js` — light-mode CSS overrides, waveform theme update
  - `ROADMAP.md` — NEW

### 2026-02-10: Email Notifications
- **Changes:**
  1. **Email service**: New `email_service.py` with SMTP, SendGrid, Mailgun support. Jinja2 template rendering. Background task sending (never blocks comment flow).
  2. **EmailTemplate model**: New table for customizable email templates with Jinja2 placeholders. Default template seeded on startup.
  3. **AppSettings extended**: Email provider config (provider, host, port, credentials, from address/name), global admin email, enable toggle.
  4. **Project extended**: Per-project `notification_email` (overrides global) and `email_template_id` (selectable template).
  5. **Comment hooks**: `create_comment` and `reply_to_comment` schedule `send_comment_notification` via FastAPI BackgroundTasks.
  6. **Admin UI**: Email settings section (provider selector, conditional fields), template CRUD (create/edit/delete/preview), project email config (notification email + template dropdown).
  7. **Admin settings endpoint**: `GET /admin/settings` returns extended settings with email config (passwords/keys never exposed). `POST /admin/settings/test-email` for testing.
  8. **DB migration**: Idempotent `ALTER TABLE` statements for existing SQLite databases.
- **Files modified:**
  - `backend/requirements.txt` — jinja2, httpx
  - `backend/app/models.py` — EmailTemplate, AppSettings email fields, Project email fields
  - `backend/app/schemas.py` — AdminSettingsOut, EmailTemplate schemas, ProjectUpdate extended
  - `backend/app/main.py` — migration, seed
  - `backend/app/email_service.py` — NEW
  - `backend/app/routers/settings.py` — admin settings, test email
  - `backend/app/routers/admin.py` — template CRUD, project update
  - `backend/app/routers/comments.py` — BackgroundTasks hooks
  - `frontend/admin/index.html` — email settings UI, template UI, project email fields
  - `frontend/admin/js/admin.js` — email settings logic, template CRUD, project email save

### 2026-02-10: Email Deliverability Improvements
- **Issue**: Test emails landing in spam folder
- **Changes:**
  1. **Plain-text alternative**: All emails now include both HTML and plain-text versions (RFC 2046 compliant)
  2. **Email headers**: Added Message-ID, Date, Reply-To, X-Mailer headers for better deliverability
  3. **Reply-To**: Uses admin email for replies (not noreply@)
  4. **Multipart MIME**: Proper MIME structure (plain-text first, then HTML)
  5. **SendGrid/Mailgun**: Added plain-text + Reply-To support
  6. **Documentation**: Created comprehensive EMAIL_DELIVERABILITY.md guide covering DNS setup (SPF/DKIM/DMARC), provider recommendations, and troubleshooting
- **Files modified:**
  - `backend/app/email_service.py` — Plain-text conversion, headers, multipart MIME
  - `EMAIL_DELIVERABILITY.md` — NEW: Complete deliverability setup guide
  - `README.md` — Email notifications section with link to guide

### 2026-02-10: Email Templates & Project Notification Toggle
- **Changes:**
  1. **Dual default templates**: Added English template alongside German one (seeded on startup)
  2. **Per-project notification toggle**: New `notifications_enabled` boolean field on Project model (default: true). Checkbox in project detail view controls email notifications per project.
  3. **Settings UI improvement**: "Save Changes" button moved to bottom of Settings view (after Email Templates section)
  4. **Project list badge**: Notification icon (🔔/🔕) shows enabled/disabled status per project
  5. **Email service check**: `send_comment_notification` now respects `project.notifications_enabled` flag
- **Files modified:**
  - `backend/app/main.py` — Seed both DE+EN templates, migration for `notifications_enabled`
  - `backend/app/models.py` — Added `notifications_enabled` field to Project
  - `backend/app/schemas.py` — ProjectUpdate, ProjectSummary, ProjectDetail extended
  - `backend/app/email_service.py` — Check `project.notifications_enabled` before sending
  - `frontend/admin/index.html` — Checkbox in project email settings, "Save Changes" button moved to bottom
  - `frontend/admin/js/admin.js` — Project list badge, save/load notifications_enabled

### 2026-02-10: Email Batching & Admin Comment Filtering
- **Changes:**
  1. **Batch comments**: New `email_batch_enabled` and `email_batch_delay_minutes` settings. When enabled, comments are batched and sent as single email after X minutes of no new comments.
  2. **Admin comment filter**: Comments from admin users never trigger email notifications (prevents self-notifications).
  3. **In-memory queue**: Batch queue uses in-memory timer (no DB table needed), cancelled/restarted on new comments.
  4. **Conditional project email settings**: Project email settings section only shown if email notifications globally enabled.
  5. **UI**: Batch checkbox + delay field in Settings → Email. Project email section auto-hidden if email disabled.
- **Files modified:**
  - `backend/app/models.py` — Added `email_batch_enabled`, `email_batch_delay_minutes` to AppSettings
  - `backend/app/schemas.py` — Extended AdminSettingsOut, SettingsUpdate with batch fields
  - `backend/app/main.py` — Migration for batch settings
  - `backend/app/routers/settings.py` — Save/load batch settings
  - `backend/app/email_service.py` — Batch logic with in-memory queue, admin comment filter, batched email generation
  - `frontend/admin/index.html` — Batch checkbox + delay input, project email settings ID
  - `frontend/admin/js/admin.js` — Batch UI logic, conditional project email section display

### 2026-02-22: VST3 Universal Binary + Build Fixes
- **Universal Binary**: Plugin baut jetzt als arm64 + x86_64. Kein Rosetta mehr nötig auf Apple Silicon.
  - `CMakeLists.txt`: Forced architecture settings mit `FORCE` flag
- **Font API**: JUCE deprecation warnings behoben
  - `juce::Font(12.0f)` → `juce::Font(juce::FontOptions(12.0f))`
  - `getStringWidth()` → `GlyphArrangement` + `getBoundingBox()`
  - Betroffen: `WaveformComponent.cpp`, `CommentListComponent.cpp`
- **Unused parameters**: `juce::ignoreUnused()` in `PluginProcessor.cpp`, `WaveformComponent.cpp`, `MixnoteTheme.cpp`, `CommentListComponent.cpp`
- **Declaration shadowing**: Lambda-Parameter-Konflikte in `PluginEditor.cpp` behoben (`err` → `projectsErr`, `v` → `version`)
- **Manufacturer**: "Stoersender" (Codes: `Stss`/`Stmx`, war `Mxnt`/`Mxnp`)
- **Build Format**: Nur noch VST3 (Standalone entfernt)
- **JUCE Splash Screen**: Aktiviert (`JUCE_DISPLAY_SPLASH_SCREEN=1`) für GPL-Compliance
- **Docs**: `vst3/docs/BUILD.md`, `vst3/docs/ARCHITECTURE.md`, `vst3/CHANGELOG.md` erstellt
- **Files modified:** `vst3/CMakeLists.txt`, `vst3/Source/WaveformComponent.cpp`, `vst3/Source/CommentListComponent.cpp`, `vst3/Source/PluginProcessor.cpp`, `vst3/Source/PluginEditor.cpp`, `vst3/Source/MixnoteTheme.cpp`

### 2026-02-10: Template Migration & Batch Email Grouping
- **Changes:**
  1. **English template migration**: Auto-adds English template to existing DBs that only have German template (one-time migration on startup)
  2. **Batch email grouping**: Comments grouped by song in batched emails. Song header with title + comment count, comment cards stacked below without repetitive song/version info. Light theme matching original templates (#f9fafb cards, #6366f1 accent).
  3. **Fire-and-forget timer**: Fixed batch timer to run as separate async task (prevents multiple emails). Timer cancelled on new comments, restarted with fresh delay.
  4. **Custom batch email format**: Batched emails use custom HTML (not template rendering) to avoid repetitive metadata. Clean layout with project title, song sections, version info inline with timecode.
- **Files modified:**
  - `backend/app/main.py` — Template migration for existing DBs
  - `backend/app/email_service.py` — Song-grouped batch emails, fire-and-forget timer task, custom HTML generation

## Development Notes
- Prefer simple, maintainable solutions over complex frameworks
- Direct, efficient code - no unnecessary abstractions
- Performance matters: 10GbE network, optimize for concurrent users
- Swiss user base: Consider German language support (optional)
- Integration potential: REAPER scripts, existing studio workflows
