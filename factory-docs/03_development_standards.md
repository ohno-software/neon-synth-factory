# Development Standards 📏

To keep the Neon Synth Factory running smoothly, all modules should adhere to the following architectural standards.

## 0. Parameter Flow Contract (Highest Priority)
All synth implementations must follow this data flow:

`UI Controls -> Central Parameter State (JUCE parameters / APVTS or equivalent) -> DSP Signal Flow`

Rules:
*   UI components write parameter values only.
*   DSP reads parameter values in `processBlock`/engine update paths.
*   UI must never mutate DSP engines or signal routing directly.
*   New modules and controls must integrate through the same contract.

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

### Selection Panel Color Convention
*   **Default factory convention**: Module/category selection panels use `active = 0xFFFF00FF` and `inactive = 0xFF808080`.
*   **Allowed override**: A synth may intentionally override these colors only when it is a documented identity exception (example: SID family blue accent).
*   **Implementation rule**: Set colors explicitly in each editor via `ModuleSelectionPanel::setButtonColors(...)` for clarity and consistency.

## 4. Coding Patterns
*   **Memory Management**: Always use `std::unique_ptr` and JUCE's smart pointers.
*   **Thread Safety**: Ensure all UI interactions occur on the Message Thread and DSP code is lock-free in `processBlock`.

## 5. Compliance Gate (CI/Code Review)
Changes must fail review (and any CI quality gate) if any of the following are present:
*   UI/editor code directly calling DSP engine mutators for runtime parameter changes.
*   UI/editor code directly changing signal routing/state that should be represented by parameters.
*   New controls that bypass JUCE parameter state (APVTS/AudioParameter-based flow).

Minimum reviewer checklist for each synth PR:
*   Confirm `UI -> parameter state -> DSP` flow is preserved.
*   Confirm parameter updates are consumed in DSP processing/update paths (not pushed directly from UI handlers).
*   Confirm no direct editor-to-engine pointer mutation path was added for control values.
