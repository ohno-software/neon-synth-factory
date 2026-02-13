# Neon Synth Factory 🏭✨

A centralized environment for the development and automated building of Neon-series synthesizers.

## Core Architecture Contract (Mandatory)
- UI controls update a centralized parameter state (parameter table/object via JUCE parameters).
- DSP/signal flow reads from that parameter state while processing audio.
- Direct control-to-signal-flow mutation is prohibited.

## Platform Targets
- **Windows/macOS desktop**: VST3 + Standalone.
- **Android**: Standalone (APK via AndroidStudio target).
- **iOS/iPadOS**: Standalone + AUv3.
- **Note**: VST3 is not an iOS plugin format.

## Quick Start
To build the factory default synth (`neon-jr`):
```powershell
.\build.ps1 -Synth neon-jr
```

## Factory Modules
*   **[neon-jr](neon-jr)**: The reference monophonic/polyphonic synth.
*   **[neon-777](neon-777)**: Dual FM oscillator powerhouse.
*   **[neon-components](neon-components)**: Shared UI components and thematic assets.
*   **[artifacts](artifacts)**: The centralized showroom for compiled binaries.

## Documentation
For a deep dive into how the factory operates and how to add your own modules:
👉 **[Factory Documentation](factory-docs/README.md)**

---
*The way of the neon-synth-factory.*
