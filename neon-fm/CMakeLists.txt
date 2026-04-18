cmake_minimum_required(VERSION 3.22)

project(NeonFM VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Paths to dependencies
set(JUCE_DIR_CANDIDATES
    ${CMAKE_CURRENT_SOURCE_DIR}/../JUCE
    ${CMAKE_CURRENT_SOURCE_DIR}/../third-party/JUCE
)

set(JUCE_DIR "")
foreach(candidate IN LISTS JUCE_DIR_CANDIDATES)
    if (EXISTS "${candidate}/CMakeLists.txt")
        set(JUCE_DIR ${candidate})
        break()
    endif()
endforeach()

if (JUCE_DIR STREQUAL "")
    message(FATAL_ERROR "JUCE not found. Expected one of: ${JUCE_DIR_CANDIDATES}")
endif()

set(COMPONENTS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../neon-components)

# Add JUCE
add_subdirectory(${JUCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}/JUCE)

# Add neon-components module
add_subdirectory(${COMPONENTS_DIR} ${CMAKE_CURRENT_BINARY_DIR}/neon-components)

# Gather all SVG resources
file(GLOB_RECURSE SVG_FILES "${CMAKE_CURRENT_SOURCE_DIR}/source/resources/algorithms/*.svg")
juce_add_binary_data(NeonFmBinaryData SOURCES ${SVG_FILES})

# Create the Plugin
juce_add_plugin(NeonFM
    PRODUCT_NAME "Neon FM"
    COMPANY_NAME "Neon Audio"
    BUNDLE_ID com.neonaudio.neonfm
    PLUGIN_MANUFACTURER_CODE NeAu
    PLUGIN_CODE NeFm
    VST2_ID NeF2
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_RESIZABLE TRUE
    FORMATS VST3 AU AUv3 Standalone AndroidStudio
    STANDALONE_MENU_BAR TRUE
)

target_sources(NeonFM
    PRIVATE
        source/PluginProcessor.cpp
        source/PluginEditor.cpp
        source/FmSignalPath.cpp
)

target_link_libraries(NeonFM
    PRIVATE
        NeonFmBinaryData
        neon_ui_components
        juce::juce_gui_basics
        juce::juce_gui_extra
        juce::juce_graphics
        juce::juce_audio_basics
        juce::juce_audio_formats
        juce::juce_audio_devices
        juce::juce_audio_utils
        juce::juce_audio_processors
        juce::juce_dsp
)

target_compile_definitions(NeonFM
    PUBLIC
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_STANDALONE_FILTER_WINDOW_USE_NAVIGATIONS_BUTTONS=1
)

# Standard Juce aide setup
juce_generate_juce_header(NeonFM)
