cmake_minimum_required(VERSION 3.22)
project(MixnotePlugin VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# macOS: Build universal binary (Apple Silicon + Intel)
if(APPLE)
    set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64" CACHE STRING "Build universal binary")
    set(CMAKE_OSX_DEPLOYMENT_TARGET "11.0" CACHE STRING "Minimum macOS version")
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
juce_add_plugin(MixnotePlugin
    COMPANY_NAME            "Mixnote"
    PLUGIN_MANUFACTURER_CODE Mxnt
    PLUGIN_CODE              Mxnp
    FORMATS                  VST3 Standalone
    PRODUCT_NAME             "Mixnote"
    IS_SYNTH                 FALSE
    NEEDS_MIDI_INPUT         FALSE
    NEEDS_MIDI_OUTPUT        FALSE
    IS_MIDI_EFFECT           FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS TRUE
    COPY_PLUGIN_AFTER_BUILD  FALSE
)

target_sources(MixnotePlugin PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
    Source/MixnoteApi.cpp
    Source/WaveformComponent.cpp
    Source/CommentListComponent.cpp
    Source/MixnoteTheme.cpp
)

target_compile_definitions(MixnotePlugin PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_DISPLAY_SPLASH_SCREEN=0
)

target_link_libraries(MixnotePlugin
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
    set(ENTITLEMENTS_FILE "${CMAKE_CURRENT_SOURCE_DIR}/MixnotePlugin.entitlements")

    foreach(target MixnotePlugin_VST3 MixnotePlugin_Standalone)
        if(TARGET ${target})
            set_target_properties(${target} PROPERTIES
                XCODE_ATTRIBUTE_CODE_SIGN_ENTITLEMENTS "${ENTITLEMENTS_FILE}"
            )
        endif()
    endforeach()
endif()
