# Building Mixnote VST3

## macOS (Apple Silicon + Intel)

### Prerequisites
- **Xcode** 14+ (with Command Line Tools)
- **CMake** 3.22+
- **macOS** 11.0+ (Big Sur or later)

### Build Steps

```bash
# Navigate to project directory
cd /path/to/mixnote/vst3

# Clean previous builds (optional but recommended)
rm -rf build

# Generate Xcode project with CMake
cmake -B build -G Xcode

# Open in Xcode
open build/MixnotePlugin.xcodeproj
```

### Building in Xcode

1. **Select Build Scheme**
   - Top left: Choose `MixnotePlugin_VST3` from scheme dropdown

2. **Choose Build Configuration**
   - Debug: For development with debugging symbols
   - Release: For production (optimized)

3. **Build**
   - Product → Build (⌘B)
   - Or: Product → Clean Build Folder (⇧⌘K) then Build

### Build Output Locations

**Debug Build:**
```
build/MixnotePlugin_artefacts/Debug/VST3/Mixnote.vst3
```

**Release Build:**
```
build/MixnotePlugin_artefacts/Release/VST3/Mixnote.vst3
```

### Installation

#### System-wide Installation
```bash
# Install Release build to system VST3 folder
cp -r build/MixnotePlugin_artefacts/Release/VST3/Mixnote.vst3 \
      ~/Library/Audio/Plug-Ins/VST3/

# Or Debug build for testing
cp -r build/MixnotePlugin_artefacts/Debug/VST3/Mixnote.vst3 \
      ~/Library/Audio/Plug-Ins/VST3/
```

#### DAW-specific Path (Reaper)
Alternatively, add the build directory to your DAW's VST3 search paths:
- **Reaper**: Preferences → Plug-ins → VST → Add:
  ```
  /Users/[username]/Documents/dev/mixnote/vst3/build/MixnotePlugin_artefacts/Debug/VST3
  ```

### Verify Installation

```bash
# Check if plugin is Universal Binary (arm64 + x86_64)
lipo -info ~/Library/Audio/Plug-Ins/VST3/Mixnote.vst3/Contents/MacOS/Mixnote

# Expected output:
# Architectures in the fat file: Mixnote are: x86_64 arm64
```

### Troubleshooting

**CMake Error: "does not appear to contain CMakeLists.txt"**
- Make sure you're in the root directory (where `CMakeLists.txt` is located)
- Run `pwd` to verify current directory

**Build fails with deprecation warnings**
- We've fixed all known deprecation warnings
- Make sure you have the latest code from main branch

**Plugin runs through Rosetta/x86 bridge**
- Verify Universal Binary: `lipo -info [path-to-plugin]`
- Clean and rebuild: `rm -rf build && cmake -B build -G Xcode`
- Check architectures in CMakeLists.txt: `CMAKE_OSX_ARCHITECTURES "arm64;x86_64"`

**JUCE Splash Screen Warning**
- Set `JUCE_DISPLAY_SPLASH_SCREEN=1` in CMakeLists.txt (GPL compliance)
- Or purchase JUCE commercial license and set to `0`

## Windows (Future)

Coming soon...

## Linux (Future)

Coming soon...
