cmake_minimum_required(VERSION 3.22)

project(neon_ui_components VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find JUCE
# This assumes JUCE is either in the parent directory or provided via CMAKE_PREFIX_PATH
if(NOT TARGET juce::juce_gui_basics)
    find_package(JUCE CONFIG REQUIRED)
endif()

juce_add_module(source/neon_ui_components)

target_link_libraries(neon_ui_components
    INTERFACE
        juce::juce_gui_basics
        juce::juce_gui_extra
        juce::juce_graphics
        juce::juce_audio_basics
        juce::juce_audio_formats
)
