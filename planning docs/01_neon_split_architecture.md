# Neon Split Synth Architecture

## Overview
Neon Split is a multi-engine synthesizer built on the JUCE framework, featuring five distinct audio engines routed through a centralized signal path. The architecture emphasizes modularity, parameter-driven modulation, and seamless integration with the shared `neon-ui-components` library.

## 1. Module Architecture
The synth consists of five independent audio engines, each responsible for a specific sound generation or processing role:

- `BassEngine`: Handles low-frequency bass sounds (e.g., analog-style basslines)
- `PadEngine`: Manages sustained harmonic textures and ambient pads
- `ArpEngine`: Implements arpeggiator functionality with pattern-based note sequencing
- `DrumEngine`: Processes drum hits and rhythmic elements
- `PatternEngine`: Controls global timing, step sequencing, and transport synchronization

These engines are instantiated as member variables within `SplitSignalPath` and do not communicate directly — all interaction is mediated through the signal path.

## 2. Signal Path
The `SplitSignalPath` class acts as the central audio processor:

- **Audio Processing**: Implements `processBlock()` to render output from all five engines into a single stereo buffer.
- **MIDI Splitting**: Uses a configurable split point (`splitPoint = 60`, Middle C) to route MIDI notes to appropriate engines (e.g., low notes → Bass, high notes → Pad/Arp).
- **Transport Sync**: Pulls BPM and play state from the host DAW via `AudioProcessor::getPlayHead()` and updates engine timing accordingly.
- **Voice Management**: Tracks active voices across engines for UI feedback (`getActiveVoicesCount()`).

The signal path is prepared and released by `PluginProcessor`, ensuring lifecycle alignment with the audio graph.

## 3. Modulation System
Modulation targets are defined via a centralized enum in `ModulationTargets.h`:

```cpp
enum class SplitModTarget
{
    None = 0,
    BassLPF,
    BassVolume,
    PadChorusMix,
    PadVolume,
    ArpFilterCutoff,
    ArpVolume,
    MasterVolume,
    Count
};
```

These targets are exposed to the UI through `NeonRegistry::setTargetNames()` in `PluginProcessor.cpp`, which maps enum values to human-readable strings (e.g., "Bass LPF"). Control sources (Mod Wheel, Pitch Bend, etc.) are similarly defined via `SplitCtrlSource`.

All modulation routing is handled by the UI components in `neon-ui-components`, which bind these named targets to parameter controls and send updates to the audio engine via JUCE’s parameter system. The actual DSP parameters (e.g., filter cutoff) are updated within each engine during `processBlock()` based on received parameter values.

## 4. Parameter Registry Integration
The `NeonRegistry` class (from `neon-ui-components/core/NeonRegistry.h`) serves as the bridge between UI and audio:

- Initialized in `NeonSplitAudioProcessor::NeonSplitAudioProcessor()` via calls to `setTargetNames()` and `setSourceNames()`.
- Enables dynamic UI generation: parameter cards, modulation matrices, and macro controls automatically reflect available targets without hardcoding.
- Does not store state — only provides metadata for UI binding. State is managed by JUCE’s `AudioParameter` system within each engine.

## Summary
Neon Split follows a clean separation of concerns:

| Layer | Responsibility |
|-------|----------------|
| **UI** | `neon-ui-components`, `PluginEditor` — displays and controls parameters |
| **Registry** | `NeonRegistry` — maps UI labels to enum targets |
| **Audio Engine** | `SplitSignalPath` — orchestrates engines, handles timing and MIDI routing |
| **Modules** | `BassEngine`, `PadEngine`, etc. — implement DSP algorithms and respond to parameter changes |

This architecture enables scalable expansion: new modules can be added without modifying the signal path, and new modulation targets require only an enum addition and UI registration.