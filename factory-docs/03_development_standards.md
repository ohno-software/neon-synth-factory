# Development Standards 📏

To keep the Neon Synth Factory running smoothly, all modules should adhere to the following architectural standards.

## 1. Directory Structure
Synths should maintain a "sparse" or "focused" folder structure to minimize clutter:
*   `CMakeLists.txt` (at root of synth folder)
*   `.h` and `.cpp` files (grouped logically)
*   `build/` (ignored by git, managed by `build.ps1`)

## 2. CMake Integration
Every synth must use the factory-standard `CMakeLists.txt` pattern to ensure compatibility with [build.ps1](../build.ps1).

### Key Dependencies
Always link against the shared factory components:
```cmake
target_link_libraries(MySynth
    PRIVATE
        neon_ui_components # Centralized UI/Themes
        juce::juce_gui_basics
        juce::juce_audio_basics
        # ...
)
```

## 3. The "Neon Style"
*   **Centralized Assets**: UI components and themes reside in [neon-components](../neon-components). Do not duplicate binary assets inside individual synth folders.
*   **Namespace Usage**: Use the `juce::` namespace explicitly to avoid collisions and improve readability.
*   **Version Control**: The root `build.ps1` expects `projectName_artefacts` in the local build folder—ensure your CMake project name matches your folder name or is handled by the script.

## 4. Coding Patterns
*   **Memory Management**: Always use `std::unique_ptr` and JUCE's smart pointers.
*   **Thread Safety**: Ensure all UI interactions occur on the Message Thread and DSP code is lock-free in `processBlock`.
