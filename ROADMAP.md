# Mixnote Roadmap

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
- [ ] mixnote.lua für ReaPack parat machen (in Documents/reaper-scripts)
- [ ] Regionen statt nur Timestamps (Waveform-Bereich markieren, z.B. 0:15–0:35, in Client + REAPER Lua)
- [ ] Alternative Layout-Ansicht (Waveform sticky oben, Versionen rechts, Kommentare unten)
