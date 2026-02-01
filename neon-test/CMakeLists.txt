cmake_minimum_required(VERSION 3.22)

project(NeonTest VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Point to JUCE
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../JUCE ${CMAKE_CURRENT_BINARY_DIR}/JUCE)
# Point to neon-components
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../neon-components ${CMAKE_CURRENT_BINARY_DIR}/neon-components)

juce_add_gui_app(NeonTest
    PRODUCT_NAME "Neon Test"
)

target_sources(NeonTest
    PRIVATE
        Main.cpp
)

target_link_libraries(NeonTest
    PRIVATE
        neon_ui_components
        juce::juce_gui_basics
)
