# Neon Synth Factory - Tech Requirements & Platform Strategy

## Target Environments

### Phase 1: Desktop Plugins
- **Formats:** VST3, AU, Standalone.
- **Operating Systems:** 
  - Windows 10/11 (x64)
  - macOS Monterey+ (Universal Binaries: Intel & Apple Silicon)
- **Minimum Requirements:** 2GHz CPU, 8GB RAM, OpenGL 2.0+ or Metal support.

### Phase 2: Embedded & Microcomputing
- **Target Platforms:**
  - Raspberry Pi 4/5 (Running Linux/Patchbox OS)
  - Custom ARM-based boards (ELK OS or similar)
- **Requirements:** 
  - Optimized UI (Possibly 2D software rendering if GPU is limited).
  - High-performance C++ DSP code (SIMD/Neon optimizations).
  - Headless mode support for "Black Box" synth hardware.

## Technical Stack
- **Framework:** JUCE 8 (Latest stable).
- **Language:** C++20.
- **Graphics:** JUCE's Graphics class (CoreGraphics/Direct2D) for UI; OpenGL/Vulkan for complex visualizations.
- **DSP:** Custom C++ classes + JUCE `dsp` module.
- **Build System:** CMake (Crucial for cross-platform and embedded builds).

## Development Goals
1. **Low Latency:** DSP must be lock-free and optimized for real-time threads.
2. **Zero Dependencies:** Keep external library dependencies to a minimum to ensure portability to microcomputers.
3. **Automated Testing:** Unit tests for DSP and Integration tests for the UI.
4. **CI/CD:** Automated builds for Windows and macOS via GitHub Actions.

