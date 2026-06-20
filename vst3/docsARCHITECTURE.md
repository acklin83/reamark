# ReaMark VST3 Architecture

## Plugin Information

### Identity
- **Company**: Stoersender
- **Product Name**: ReaMark
- **Manufacturer Code**: `Stss` (4-char identifier)
- **Plugin Code**: `Stmx` (4-char identifier)
- **Version**: 1.0.0

### Build Configuration
- **Formats**: VST3 only
- **Architectures**: Universal Binary (arm64 + x86_64)
- **Minimum macOS**: 11.0 (Big Sur)
- **C++ Standard**: C++17
- **Framework**: JUCE 8.0.6

### Plugin Type
- **Category**: Utility / Collaboration Tool
- **Audio Processing**: Passthrough (no audio manipulation)
- **Purpose**: Comment and review system for DAW sessions
- **MIDI**: No MIDI input/output

## Code Structure

### Core Components

#### `PluginProcessor.cpp` / `PluginProcessor.h`
**Responsibility**: Audio processing and state management

- **Audio**: Passes audio through unchanged (passthrough)
- **Transport**: Reads DAW transport position, BPM, play state
- **State Persistence**: Saves/restores plugin settings in DAW session
  - Server URL
  - Username & credentials
  - Calibration offsets per song
  - Last selected project/share link
  - Autoplay settings

**Key Members**:
```cpp
std::atomic<double> transportPosition;
std::atomic<bool> transportPlaying;
std::atomic<double> transportBpm;
juce::String serverUrl, username, authorName;
std::map<int, double> calibrationOffsets;
```

#### `PluginEditor.cpp` / `PluginEditor.h`
**Responsibility**: User interface and API orchestration

**Major Sections**:
1. **Login Panel**: Server connection, authentication
2. **Project Selection**: Admin projects dropdown
3. **Song/Version Selection**: Song and version management
4. **Waveform Display**: Visual timeline with comments
5. **Comment Creation**: New comment input
6. **Comment List**: Filterable comment display (All/Open/Done)

**Key Features**:
- Real-time transport sync (60 FPS timer)
- Calibration offset management per song
- Favourite version tracking
- Admin vs. guest user permissions

#### `ReaMarkApi.cpp` / `ReaMarkApi.h`
**Responsibility**: Backend HTTP communication

**Endpoints**:
- `POST /api/login` - User authentication
- `GET /api/admin/projects` - List admin projects
- `GET /api/projects/:code` - Load project details
- `GET /api/songs/:id/comments` - Fetch comments
- `POST /api/songs/:versionId/comments` - Create comment
- `PUT /api/comments/:id` - Update comment
- `DELETE /api/comments/:id` - Delete comment
- `POST /api/comments/:id/resolve` - Mark as resolved
- `POST /api/comments/:id/replies` - Add reply
- `GET /api/songs/:versionId/peaks` - Fetch waveform data
- `POST /api/versions/:id/favourite` - Toggle favourite

**Implementation**: JUCE `URL` with async callbacks

#### `WaveformComponent.cpp` / `WaveformComponent.h`
**Responsibility**: Waveform visualization with comment markers

**Features**:
- Audio peak data rendering
- Comment markers (color-coded: amber=open, green=solved)
- Playhead position indicator (synced with transport)
- Click-to-seek functionality
- Hover tooltips for comments
- Calibration offset support

#### `CommentListComponent.cpp` / `CommentListComponent.h`
**Responsibility**: Comment display and interaction

**Components**:
- **CommentCard**: Individual comment display
  - Timecode button (click to seek)
  - Author and text
  - Admin actions: Resolve, Edit, Delete
  - Reply thread support
  - Inline editing

- **CommentListComponent**: Container with filtering
  - Filter modes: All / Open / Done
  - Counts display
  - Scrollable viewport
  - Refresh button

#### `ReaMarkTheme.cpp` / `ReaMarkTheme.h`
**Responsibility**: Consistent look & feel

**Custom LookAndFeel**:
- Dark theme color scheme
- Rounded corners (4px radius)
- Custom button, text editor, combo box rendering
- Accent colors for interactive elements

**Theme Colors**:
```cpp
bgBody()    // Main background
bgCard()    // Card/panel backgrounds
bgInput()   // Input field backgrounds
accent()    // Primary accent (blue)
text()      // Primary text
textMuted() // Secondary text
green()     // Success/resolved
amber()     // Warning/open
red()       // Error/delete
```

## Data Models

### Project Structure
```
Project
├── id, title, shareLink
└── songs[]
    ├── id, title
    └── versions[]
        ├── id, versionNumber, label
        ├── favourite (bool)
        └── audioFilePath
```

### Comment Structure
```
Comment
├── id, text, timecode
├── authorName, solved
└── replies[]
    ├── text
    └── authorName
```

## State Management

### Session Persistence (DAW save/load)
Stored in `getStateInformation` / `setStateInformation`:
- Server URL and credentials
- Last project/share link
- Per-song calibration offsets (JSON structure)
- Autoplay preference

### Runtime State
- Current project, song, version selection
- Loaded comments and waveform peaks
- Transport position (polled in timer)
- Login status and JWT token

## Audio Thread Safety

- **No audio processing** in this plugin (passthrough only)
- Transport info read in `processBlock()` via atomics
- UI updated on message thread via `Timer` (60 FPS)
- No locks or allocations in audio thread

## Network Architecture

- **Async HTTP**: All API calls use JUCE URL with callbacks
- **JWT Authentication**: Token stored after login, sent in headers
- **Error Handling**: Callbacks provide `(bool success, data, error message)`
- **No blocking**: Network calls never block UI or audio thread

## Build System

### CMake Configuration
- **JUCE**: Fetched from GitHub (tag 8.0.6)
- **Plugin Format**: VST3 only (no Standalone, AU, etc.)
- **Architectures**: Forced Universal Binary for macOS
- **Entitlements**: Network access for macOS sandboxing

### Key CMake Variables
```cmake
CMAKE_OSX_ARCHITECTURES "arm64;x86_64" FORCE
CMAKE_OSX_DEPLOYMENT_TARGET "11.0"
JUCE_DISPLAY_SPLASH_SCREEN 1  # GPL compliance
```

## Code Conventions

### Naming
- **Classes**: PascalCase (`WaveformComponent`)
- **Methods**: camelCase (`loadComments()`)
- **Members**: camelCase (`transportPosition`)
- **Namespaces**: lowercase (`reamark`)

### JUCE Patterns
- **Callbacks**: `std::function<>` members (`onSeek`, `onResolve`)
- **UI Updates**: `juce::dontSendNotification` for programmatic changes
- **Modern Font API**: `juce::FontOptions` → `juce::Font`
- **String Width**: `GlyphArrangement` for deprecated `getStringWidth`

### Memory Management
- **Smart Pointers**: `std::unique_ptr` for owned components
- **JUCE Ownership**: `addAndMakeVisible` for child components
- **No Manual Delete**: JUCE manages component lifecycle

## Future Enhancements

### Planned Features
- [ ] Real-time collaboration (WebSocket)
- [ ] Offline mode with sync
- [ ] Audio attachment to comments
- [ ] Version diffing
- [ ] Custom keyboard shortcuts

### Platform Support
- [x] macOS (Apple Silicon + Intel)
- [ ] Windows (VST3)
- [ ] Linux (VST3)

### Plugin Formats
- [x] VST3
- [ ] AU (Audio Unit)
- [ ] AAX (Pro Tools)

## Dependencies

### External
- **JUCE 8.0.6**: Framework for audio plugins
- **C++ Standard Library**: STL containers, threading

### System
- **macOS**: Core Foundation, AppKit
- **Network**: URLSession (via JUCE)

## License

- **Code**: [Your License]
- **JUCE**: GPL or Commercial (JUCE_DISPLAY_SPLASH_SCREEN=1 for GPL)
