# ReaMark - Development Roadmap

---

## Task 1: REAPER Waveform-Player mit ReaMark-Audio (PRIORITY)

**Goal:** Waveform des aktuell geladenen ReaMark-Songs direkt im Lua Script anzeigen — mit Playhead-Steuerung und Kommentar-Markern.

**Ansatz:** Peak-Daten werden serverseitig vorberechnet und als JSON-Endpoint bereitgestellt. Das Lua Script lädt die Peaks und rendert sie mit ImGui Draw-Primitives.

**Backend:**
- Beim Upload: Peak-Daten generieren (normalisierte Amplituden-Array, ~500–1000 Samples)
- Neuer Endpoint: `GET /api/versions/{id}/peaks` → JSON-Array `[0.12, 0.45, 0.87, ...]`
- Peaks als JSON-Datei neben der Audiodatei speichern (Cache)

**Lua Script (ImGui):**
1. Peaks vom Backend laden (einmalig pro Version-Wechsel)
2. Waveform zeichnen via `ImGui_DrawList_AddLine()` / `AddRectFilled()`
3. Kommentar-Marker als farbige vertikale Linien auf der Waveform
4. Klick auf Waveform → Timecode berechnen (mit Calibration-Offset) → `reaper.SetEditCurPos()`
5. Playhead-Linie: Echtzeit-Position als vertikale Linie über der Waveform
6. Zoom/Scroll der Waveform (optional, Phase 2)

**Abhängigkeiten:**
- Calibration-Offset (bereits vorhanden im Script)
- Versions-Auswahl (bereits vorhanden)
- Kommentar-Timecodes (bereits vorhanden)

**Files:**
- Backend: `routers/projects.py` oder `routers/comments.py` (neuer Endpoint), `utils/audio.py` (Peak-Berechnung)
- Lua: `reaper/reamark_v2.lua` (Waveform-Rendering + Interaktion)

---

## Task 2: Marker Display

**Goal:** Eingebettete REAPER-Marker im Waveform anzeigen (Web-Frontend + Lua Script)

**How it works:**
1. User exportiert aus REAPER mit "Embed markers"
2. Backend extrahiert Marker aus WAV/MP3/FLAC beim Upload
3. Frontend zeigt vertikale Linien mit Labels auf der Waveform
4. Klick auf Marker = Jump to Position

**Visual:**
```
    Intro    Verse    Chorus    Bridge
      |        |         |         |
  > [=|========|=========|=========|====]
```

**Backend:**
- Marker aus Audio-Dateien extrahieren (BWF cue points, ID3 tags)
- Neues `Marker` DB-Model
- `GET /api/versions/{id}/markers` Endpoint

**Frontend:**
- Vertikale Linien an Marker-Positionen rendern
- Marker-Namen als Labels
- Click-Handler zum Timecode springen

**Database:**
```sql
CREATE TABLE markers (
    id INTEGER PRIMARY KEY,
    version_id INTEGER REFERENCES versions(id),
    timecode FLOAT NOT NULL,
    name VARCHAR(255) NOT NULL,
    color VARCHAR(7) DEFAULT '#f59e0b',
    created_at TIMESTAMP
);
```

**Files:**
- Backend: `models.py`, `utils/audio.py`, `routers/projects.py`
- Frontend: `app.js`, `styles.css`

---

## Task 3: VST3 Plugin (DAW-unabhängig)

**Goal:** ReaMark als VST3-Plugin für alle DAWs (Logic Pro, Cubase, Ableton, Pro Tools, Studio One etc.) — selbe Features wie das REAPER Lua Script.

**Tech Stack:** C++17, JUCE 7, CMake, VST3 SDK (über JUCE)

**Features (1:1 vom Lua Script):**
- Admin-Login (JWT) mit Remember-me
- Projekt-Browser (Admin: Dropdown, Client: Share-Link)
- Song/Version-Auswahl mit Favourites
- Waveform-Anzeige (Peaks vom Backend, Kommentar-Marker, Playhead)
- Calibration-Offset (pro Song, manuell via DAW-Transport-Position)
- Kommentar-Erstellung mit Timecode aus DAW-Transport
- Kommentar-Liste mit Filter (All/Open/Done)
- Reply, Edit, Delete, Resolve (Admin)
- Autoplay-Toggle
- Dark Theme (ReaMark Website Design)

**Einschränkungen vs. Lua Script:**
- Kein Transport-Seek: VST3 kann DAW-Transport nicht steuern (nur lesen)
- Klick auf Timecode/Waveform springt nicht automatisch zur Position

**Architektur:**
```
vst3/
├── CMakeLists.txt            # Build-System, JUCE via FetchContent
└── Source/
    ├── PluginProcessor.h/cpp  # Audio-Passthrough, Transport-Info, State
    ├── PluginEditor.h/cpp     # Haupt-UI (Login, Projekt, Layout)
    ├── ReaMarkApi.h/cpp       # Async HTTP-Client (alle Endpoints)
    ├── ReaMarkModels.h        # Datenstrukturen + JSON-Parsing
    ├── ReaMarkTheme.h/cpp     # Farben + Custom LookAndFeel
    ├── WaveformComponent.h/cpp      # Waveform mit Markern + Playhead
    └── CommentListComponent.h/cpp   # Kommentar-Cards mit Aktionen
```

**Build:**
```bash
cd vst3 && mkdir build && cd build
cmake .. && cmake --build . --config Release
```

**API-Endpoints (identisch zum Lua Script):**
- `POST /admin/auth/login` — Login
- `GET /admin/projects` — Admin-Projekte
- `GET /api/projects/{uuid}` — Projekt laden
- `GET /api/projects/{uuid}/comments` — Kommentare
- `POST /api/projects/{uuid}/comments` — Kommentar erstellen
- `POST /api/projects/{uuid}/comments/{id}/reply` — Antworten
- `PATCH /api/projects/{uuid}/comments/{id}/resolve` — Lösen
- `PUT /admin/comments/{id}` — Bearbeiten
- `DELETE /admin/comments/{id}` — Löschen
- `PATCH /admin/versions/{id}/favourite` — Favorit
- `GET /api/versions/{id}/peaks` — Waveform-Peaks

**Files:** `vst3/` (komplett neues Verzeichnis)

---

## Completed / Cancelled (Archive)

| Task | Status | Notes |
|------|--------|-------|
| Lua/ImGui Beautification | DONE | Dark theme, styled cards, accent colors |
| Client Comment Edit/Delete | DONE | PUT/DELETE endpoints, client UI |
| REAPER Upload from Script | CANCELLED | Kein File-Browser in ImGui |
| WebView Script | CANCELLED | Kein Built-in Browser in REAPER |
