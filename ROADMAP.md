# ReaMark Roadmap

## V1 — Erledigt

- [x] Core Backend + Database (SQLAlchemy, FastAPI, Auth, File Upload)
- [x] Admin Interface (Login, Projects, Upload, Management)
- [x] Client Interface (Share Links, Wavesurfer.js, Comments, Versions)
- [x] Waveform Generation (ffmpeg, Peak Caching)
- [x] Mobile Responsiveness
- [x] Docker + DiskStation Deployment
- [x] REAPER Integration (Lua/ImGui Script)

## V2 — To Do

- [x] Light-Mode Fix (Text- und Hintergrundfarben, Waveform-Farben bei Theme-Wechsel)
- [x] Email-Benachrichtigungen bei neuen Kommentaren (SMTP/SendGrid/Mailgun, Templates, pro Projekt)
- [x] VST3 Plugin (JUCE/C++, selbe Features wie Lua Script — DAW-unabhängig für Logic, Cubase, Ableton etc.)
- [ ] Real-time Updates (WebSocket)
- [ ] Link-Ablaufdatum (Share Links mit Expiration)
- [ ] Mobile App
- [ ] reamark.lua für ReaPack parat machen (in Documents/reaper-scripts)
- [ ] Regionen statt nur Timestamps (Waveform-Bereich markieren, z.B. 0:15–0:35, in Client + REAPER Lua)
- [ ] Alternative Layout-Ansicht (Waveform sticky oben, Versionen rechts, Kommentare unten)

## Open-Source Release (self-hosted, white-label)

- [x] White-Labeling vollständig: `site_name` + Favicon-Upload (zusätzlich zu Logo + Theme-Farben)
- [x] `.env.example`, generischer `nginx server_name`, interne Files aus public Repo ausgeschlossen
- [x] LICENSE (AGPLv3 — JUCE-Frei ist AGPLv3, nicht GPLv3; VST3 SDK ist MIT) + `LICENSING.md`
- [ ] Git-History-Scrub (`claude.md` raus) vor Public-Schalten — **braucht Force-Push, Befehle stehen bereit**
- [x] Prebuilt Images via ghcr.io + GitHub Actions (`Dockerfile.dist`, `docker-publish.yml`, `docker-compose.ghcr.yml`)
- [x] Caddy-Compose für automatisches HTTPS (`docker-compose.caddy.yml`, `Caddyfile`)
- [x] README erweitert (prebuilt images, Caddy-HTTPS, Branding, Configuration)
- [x] Rate-Limiting (slowapi: Login 10/min, Setup 5/min, Comments/Replies 30/min, X-Forwarded-For-aware)
- [x] ReaPack-Header in `reamark.lua` (Repo/index.xml-Setup = separater Schritt)
- [x] VST3-Release-Workflow (macOS universal, an GitHub Release angehängt bei Tag)
