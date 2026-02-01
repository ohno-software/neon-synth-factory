cmake_minimum_required(VERSION 3.22)
project(NeonQwenSynth VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Paths to dependencies
set(JUCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../JUCE)
set(COMPONENTS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../neon-components)

# Add JUCE
add_subdirectory(${JUCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}/JUCE)

# Add neon-components module
add_subdirectory(${COMPONENTS_DIR} ${CMAKE_CURRENT_BINARY_DIR}/neon-components)

# Create the Plugin
juce_add_plugin(NeonQwenSynth
    PRODUCT_NAME "Neon Qwen Synth"
    COMPANY_NAME "Neon Audio"
    BUNDLE_ID com.neonaudio.neonqwen
    PLUGIN_MANUFACTURER_CODE NeAu
    PLUGIN_CODE NeQw
    VST2_ID NeQ2
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_RESIZABLE TRUE
    FORMATS VST3 Standalone
    STANDALONE_MENU_BAR TRUE
    PRODUCT_NAME "Neon Qwen Synth"
)

target_sources(NeonQwenSynth
    PRIVATE
        NeonQwenSynthProcessor.cpp
        NeonQwenSynthEditor.cpp
        NeonQwenSynthSound.cpp
        NeonQwenSynthVoice.cpp
)

target_link_libraries(NeonQwenSynth
    PRIVATE
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

target_compile_definitions(NeonQwenSynth
    PUBLIC
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_STANDALONE_FILTER_WINDOW_USE_NAVIGATIONS_BUTTONS=1
)

# Standard Juce aide setup
juce_generate_juce_header(NeonQwenSynth)

# Add standalone executable

