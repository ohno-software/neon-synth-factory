# Neon Chip

A chiptune synthesizer emulating classic SID (C64), NES (2A03), and Atari 2600 (TIA) sound chips. Part of the Neon Synth Factory family.

## Overview

Neon Chip recreates the lo-fi charm of vintage gaming hardware in a modern synth architecture. Switch between three chip emulation modes to get the distinctive timbres of 8-bit era sound chips, with proper MIDI control, polyphonic voice management, filtering, and the standard Neon UI.

## Chip Modes

### SID (MOS 6581/8580)
The Commodore 64 sound chip. Waveforms: Sawtooth, Pulse (variable width), Triangle, Noise (LFSR).

### NES (2A03)
The Nintendo Entertainment System APU. Waveforms: Square 12.5%, Square 25%, Square 50%, Triangle (4-bit quantized), Noise.

### Atari 2600 (TIA)
The Atari 2600 Television Interface Adapter. Waveforms: Square, Div-by-6 buzz, Div-by-31 metallic buzz, Poly noise.

## Interface Layout

Uses the standard Neon module selection panel:

| Category | Modules |
|----------|---------|
| CHIP     | Chip Osc (type, waveform, pulse width, bit depth) |
| FILTER   | Filter (LP/HP/BP, cutoff, resonance) |
| AMP      | Amp Output, Amp Envelope (ADSR) |
| M/FX     | LFO 1, LFO 2, FX |
| MAIN     | Control, Librarian |

## Building

Use the factory build script:

```powershell
.\build.ps1 -Synth neon-chip -Config Release
```

Debug build:

```powershell
.\build.ps1 -Synth neon-chip -Config Debug
```

Clean rebuild:

```powershell
.\build.ps1 -Synth neon-chip -Config Release -Clean
```

## Output Formats

- **VST3**: Plugin for DAWs
- **Standalone**: Desktop application

Artifacts are collected in `artifacts/neon-chip/`

## Features

- ✅ Neon UI components and theming
- ✅ 8x8 button matrix navigation
- ✅ 8 parameter controls
- ✅ Display area with module/parameter info
- ✅ MIDI activity indicator
- ❌ No audio processing (GUI template only)
- ❌ No audio routing

## Dependencies

- JUCE framework
- neon-components (Neon UI component library)
- CMake 3.22+
- Ninja build system

## Usage as Template

To create a new synth based on this template:

1. Copy the `neon-template` directory
2. Rename to your synth name (e.g., `neon-crystal`)
3. Update `CMakeLists.txt`:
   - Change project name
   - Update plugin codes and identifiers
   - Update product name
4. Customize `PluginEditor.cpp`:
   - Add your specific module buttons
   - Implement actual parameter functionality
5. Implement audio processing in `PluginProcessor.cpp`
6. Build using `.\build.ps1 -Synth your-synth-name`

## Architecture

- **PluginProcessor**: Minimal audio processor (no audio routing in template)
- **PluginEditor**: Complete GUI implementation with Neon components
  - Top button group selection
  - Bottom button module selection
  - Parameter cards for value control
  - Display area for status and values

## See Also

- [neon-jr](../neon-jr/) - Full synthesizer implementation
- [neon-components](../neon-components/) - Shared UI component library
- [factory-docs](../factory-docs/) - Build system documentation
