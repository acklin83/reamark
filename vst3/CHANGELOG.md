# Changelog

All notable changes to the ReaMark VST3 Plugin will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased] - 2026-02-22

### Fixed
- **Universal Binary Build**: Plugin now builds as Universal Binary (arm64 + x86_64)
  - No more Rosetta/x86 bridge required on Apple Silicon Macs
  - Forced architecture settings in CMakeLists.txt with `FORCE` flag
- **Font Deprecation Warnings**: Updated to modern JUCE Font API
  - `juce::Font(12.0f)` → `juce::Font(juce::FontOptions(12.0f))`
  - `font.getStringWidth()` → `GlyphArrangement` with `getBoundingBox()`
  - Fixed in `WaveformComponent.cpp` and `CommentListComponent.cpp`
- **Unused Parameter Warnings**: Added `juce::ignoreUnused()` for all unused parameters
  - `PluginProcessor.cpp`: `buffer`, `sampleRate`, `samplesPerBlock`, etc.
  - `WaveformComponent.cpp`: `event` in `mouseExit()`
  - `ReaMarkTheme.cpp`: `button`, `isButtonDown`, `box`
  - `CommentListComponent.cpp`: Removed unused `statusArea` variable
- **Declaration Shadowing**: Fixed nested lambda parameter conflicts
  - `PluginEditor.cpp`: Renamed inner lambda `err` → `projectsErr` in `doLogin()`
  - `PluginEditor.cpp`: Renamed loop variable `v` → `version` in `doToggleFavourite()`

### Changed
- **Plugin Manufacturer**: Updated to "Stoersender"
  - Manufacturer Code: `Stss` (was: `Mxnt`)
  - Plugin Code: `Stmx` (was: `Mxnp`)
- **Build Format**: VST3 only (removed Standalone format)
  - Simplified deployment and testing workflow
- **JUCE Splash Screen**: Set `JUCE_DISPLAY_SPLASH_SCREEN=1`
  - Compliance with GPL license requirements
  - Alternative: Purchase JUCE commercial license to disable

### Added
- **Documentation**: Comprehensive build and architecture docs
  - `docs/BUILD.md`: Step-by-step build instructions for macOS
  - `docs/ARCHITECTURE.md`: Complete code structure and design documentation
  - `CHANGELOG.md`: This file!

## [1.0.0] - 2026-02-XX (Initial Release)

### Added
- **Core Plugin Functionality**
  - VST3 audio plugin (passthrough, no audio processing)
  - Comment and review system for DAW sessions
  - Real-time transport sync with DAW playhead
  
- **Authentication & Projects**
  - Server connection and login system
  - Admin project management
  - Project/song/version hierarchy
  
- **Comment System**
  - Create comments with timecode markers
  - Reply to comments (threaded discussions)
  - Resolve/unresolve comments (admin)
  - Edit and delete comments (admin)
  - Filter comments: All / Open / Done
  
- **Waveform Display**
  - Visual audio waveform from peak data
  - Comment markers (color-coded by status)
  - Playhead position indicator
  - Click-to-seek functionality
  - Hover tooltips for comments
  
- **Calibration System**
  - Per-song time offset calibration
  - "Set Offset Here" button for easy alignment
  - Offset display with warning for uncalibrated songs
  
- **User Experience**
  - Dark theme with custom look & feel
  - Responsive resizable UI (min 420x300)
  - Keyboard focus for text inputs
  - Favourite version marking
  - Autoplay toggle for automatic playback on seek
  
- **State Persistence**
  - Save/restore in DAW session:
    - Server URL and login credentials
    - Last project and share link
    - Per-song calibration offsets
    - User preferences (remember password, autoplay)

### Technical
- **Framework**: JUCE 8.0.6
- **Language**: C++17
- **Build System**: CMake 3.22+
- **Platforms**: macOS 11.0+ (Universal Binary)
- **Network**: HTTP REST API with JWT authentication
- **Architecture**: Clean separation of concerns (Processor, Editor, API, Components)

---

## Release Notes Format

### Categories
- **Added**: New features
- **Changed**: Changes in existing functionality
- **Deprecated**: Soon-to-be removed features
- **Removed**: Removed features
- **Fixed**: Bug fixes
- **Security**: Vulnerability fixes

### Commit Message Convention
We follow [Conventional Commits](https://www.conventionalcommits.org/):

- `feat:` - New feature
- `fix:` - Bug fix
- `docs:` - Documentation changes
- `style:` - Code style changes (formatting, etc.)
- `refactor:` - Code refactoring
- `perf:` - Performance improvements
- `test:` - Adding or updating tests
- `chore:` - Build process or auxiliary tool changes

**Examples**:
```
feat: add waveform zoom controls
fix: resolve crash when no audio file loaded
docs: update API endpoint documentation
refactor: extract comment rendering to separate component
```
