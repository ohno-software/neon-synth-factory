# Unified Build System 🏗️

The Neon Synth Factory uses a centralized build system to maintain consistency across all projects. This is managed via the `build.ps1` script located at the root.

## The Script: `build.ps1`
The script automates the CMake configuration, Ninja compilation, and artifact collection.

### Core Usage
To build a specific synth, run the script from the root directory:
```powershell
.\build.ps1 -Synth <synth-folder-name>
```

### Parameters
| Parameter | Description | Default |
| :--- | :--- | :--- |
| `-Synth` | The name of the synth folder to build (e.g., `neon-qwen`, `neon-jr`). | **Mandatory** |
| `-Config` | Build mode: `Debug` or `Release`. | `Debug` |
| `-Clean` | A switch to delete the local `build/` folder before starting. | Off |

### Examples
1.  **Standard Build**: `.\build.ps1 -Synth neon-qwen`
2.  **Full Rebuild**: `.\build.ps1 -Synth neon-jr -Clean`
3.  **Release Build**: `.\build.ps1 -Synth neon-test -Config Release`

## Artifact Management
The script follows a "harvest and centralize" strategy. Once a build is complete, the final binaries are copied into a central `artifacts/` folder at the root.

### Structure
Built versions are sorted by synth name and format:
*   `artifacts/<synth-name>/Standalone/` - Windows Native Executables
*   `artifacts/<synth-name>/VST3/` - VST3 Plugins

This structure ensures that the factory showroom is always organized and easy to navigate for testing or distribution.

## Technical Requirements
*   **Generator**: Ninja (for speed and consistency).
*   **Compiler**: Clang (LLVM).
*   **JUCE**: Handled as a submodule at the parent directory.
