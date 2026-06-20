cmake_minimum_required(VERSION 3.22)
project(ReaMarkPlugin VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# macOS settings
if(APPLE)
    set(CMAKE_OSX_DEPLOYMENT_TARGET "11.0" CACHE STRING "Minimum macOS version")
    # Force Universal Binary for Apple Silicon + Intel Macs
    set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64" CACHE STRING "Build universal binary" FORCE)
endif()

# Windows settings
if(WIN32)
    # Force 64-bit build on Windows to avoid x86 bridge in Reaper
    if(CMAKE_SIZEOF_VOID_P EQUAL 4)
        message(FATAL_ERROR "32-bit builds are not supported. Please configure with: cmake -B build -A x64")
    endif()
endif()

# Fetch JUCE
include(FetchContent)
FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG        8.0.6
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(JUCE)

# Plugin target
juce_add_plugin(ReaMarkPlugin
    COMPANY_NAME            "Stoersender"
    PLUGIN_MANUFACTURER_CODE Stss
    PLUGIN_CODE              Stmx
    FORMATS                  VST3
    PRODUCT_NAME             "ReaMark"
    IS_SYNTH                 FALSE
    NEEDS_MIDI_INPUT         FALSE
    NEEDS_MIDI_OUTPUT        FALSE
    IS_MIDI_EFFECT           FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS TRUE
    COPY_PLUGIN_AFTER_BUILD  FALSE
)

target_sources(ReaMarkPlugin PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
    Source/ReaMarkApi.cpp
    Source/WaveformComponent.cpp
    Source/CommentListComponent.cpp
    Source/ReaMarkTheme.cpp
)

target_compile_definitions(ReaMarkPlugin PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_DISPLAY_SPLASH_SCREEN=1
)

target_link_libraries(ReaMarkPlugin
    PRIVATE
        juce::juce_audio_utils
        juce::juce_gui_extra
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

# macOS: Apply entitlements for network access
if(APPLE)
    set(ENTITLEMENTS_FILE "${CMAKE_CURRENT_SOURCE_DIR}/ReaMarkPlugin.entitlements")

    foreach(target ReaMarkPlugin_VST3 ReaMarkPlugin_Standalone)
        if(TARGET ${target})
            set_target_properties(${target} PROPERTIES
                XCODE_ATTRIBUTE_CODE_SIGN_ENTITLEMENTS "${ENTITLEMENTS_FILE}"
            )
        endif()
    endforeach()
endif()
