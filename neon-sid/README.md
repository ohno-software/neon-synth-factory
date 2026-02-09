# Neon Template

A GUI template for future synthesizers based on the Neon look and feel.

## Purpose

This template provides a standardized interface layout that can be used as a starting point for new Neon Audio synth projects. It demonstrates the look, feel, and UI structure without implementing any audio processing.

## Interface Layout

### Top Buttons (1-8)
Eight numbered buttons across the top that act as group selectors. Each button determines which set of bottom buttons is visible.

### Display Area
Centered display showing:
- **Top line**: Module name (e.g., "1a"), Patch name ("Default"), MIDI indicator
- **Bottom line**: Parameter name (e.g., "parameter1"), Parameter value (1-127)

### Parameter Controls
Two rows of parameter cards (4 cards per row) for a total of 8 parameters:
- parameter1 through parameter4 (top row)
- parameter5 through parameter8 (bottom row)

Each parameter ranges from 1 to 127 with a default value of 64.

### Bottom Buttons
Eight groups of eight buttons, organized by the top button selection:
- **Group 1**: 1a, 1b, 1c, 1d, 1e, 1f, 1g, 1h
- **Group 2**: 2a, 2b, 2c, 2d, 2e, 2f, 2g, 2h
- **Group 3-8**: Same pattern (3a-3h, 4a-4h, etc.)

Clicking a top button shows its corresponding group of bottom buttons. Clicking a bottom button updates the module name in the display.

## Building

Use the factory build script:

```powershell
.\build.ps1 -Synth neon-template -Config Release
```

Or for development (Debug):

```powershell
.\build.ps1 -Synth neon-template -Config Debug
```

Add `-Clean` to rebuild from scratch:

```powershell
.\build.ps1 -Synth neon-template -Config Release -Clean
```

## Output Formats

- **VST3**: Plugin for DAWs
- **Standalone**: Desktop application

Artifacts are collected in `artifacts/neon-template/`

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
