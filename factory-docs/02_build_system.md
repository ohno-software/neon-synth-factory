# Unified Build System 🏗️

The Neon Synth Factory uses centralized scripts and CI workflows to keep builds consistent across desktop and mobile targets.

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

## Android Build Script: `build-android.ps1`
Use this script for Android APK builds:

```powershell
.\build-android.ps1 -Synth <synth-folder-name> -Config Release
```

### Android Parameters
| Parameter | Description | Default |
| :--- | :--- | :--- |
| `-Synth` | The synth folder to build (e.g., `neon-jr`) | `neon-jr` |
| `-Config` | Build mode: `Debug` or `Release` | `Release` |
| `-AndroidNdkPath` | Explicit Android NDK path override | From env vars |
| `-Clean` | Delete local Android build folder before build | Off |

Environment variables recognized for NDK discovery:
- `ANDROID_NDK_ROOT`
- `ANDROID_NDK_HOME`
- `NDK_HOME`

## Artifact Management
The scripts follow a "harvest and centralize" strategy. Once a build is complete, final binaries are copied into `artifacts/` at the workspace root.

### Structure
Built versions are sorted by synth name and format:
*   `artifacts/<synth-name>/Standalone/` - Windows Native Executables
*   `artifacts/<synth-name>/VST3/` - VST3 Plugins
*   `artifacts/<synth-name>/Android/` - Android APK outputs (when built with `build-android.ps1`)

This structure ensures that the factory showroom is always organized and easy to navigate for testing or distribution.

## Platform Format Matrix
- **Windows/macOS desktop**: `VST3` + `Standalone`
- **Android**: `Standalone` app packaging via `AndroidStudio` format
- **iOS/iPadOS**: `Standalone` + `AUv3`

> VST3 is a desktop plugin format and is not an iOS plugin format.

## GitHub Workflows
Current CI workflows under `.github/workflows/`:
- Per-synth desktop + Android build workflows (Windows/macOS/Linux-Android matrix)
- `build-ios.yml` for iOS-compatible outputs (Standalone + AUv3 artifacts)

Typical CI trigger paths:
- Push / PR to `main` (validation builds)
- Tag `v*` (release artifact publication)
- Manual workflow dispatch

## iOS Notes
iOS builds require macOS + Xcode toolchains and Apple signing/provisioning for installable distribution. CI can compile and package unsigned build outputs for verification, while App Store/TestFlight distribution requires team signing setup.

## Technical Requirements
*   **Generator**: Ninja (for speed and consistency).
*   **Compiler**: Clang (LLVM).
*   **JUCE**: Handled as a submodule at the parent directory.
*   **Android**: Android NDK + JDK 17 (for APK/Gradle builds).
*   **iOS**: Xcode toolchain on macOS for iOS/AUv3 targets.
