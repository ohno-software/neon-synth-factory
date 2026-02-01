# Neon Synth Factory - Module Selection Panel

## Overview
The **Module Selection Panel** is the navigational heart of the synthesizer. It provides a high-level overview of the instrument's architecture and allows the user to quickly switch focus between different [Synth Modules](./04_synth_modules.md).

## Design Inspiration
- **Hydrasynth:** Signal flow block diagram where each block is a selectable button.
- **Ensoniq ESQ-1:** Grouped tactile buttons for logical sections (Osc, Filter, LFO).
- **Casio CZ-101:** Visual lineage showing DCO -> DCW -> DCA progression.

## Visual Layout
The panel should represent the signal flow of the specific synthesizer. Buttons are connected by "Patch Lines" (Neon Glow lines) that indicate the path of audio and modulation.

```
      ┌───────┐      ┌───────────┐      ┌───────┐      ┌────────┐      ┌─────────┐
      │ OSC 1 ├──────┤ MUTATOR 1 ├──────┤       │      │        │      │         │
      └───────┘      └───────────┘      │       │      │ FILTER ├──────┤   AMP   │──▶ OUTPUT
      ┌───────┐      ┌───────────┐      │ MIXER ├──────┤        │      │         │
      │ OSC 2 ├──────┤ MUTATOR 2 ├──────┤       │      │        │      └─────────┘
      └───────┘      └───────────┘      └───────┘      └────────┘           ▲
                                                                            │
      ┌───────┐      ┌───────────┐      ┌────────┐      ┌──────────┐      ┌─┴───────┐
      │ LFO 1 ├──▶───┤   ENV 1   ├──▶───┤ MATRIX ├──────┤ EFFECTS  ├──────┤ GLOBAL  │
      └───────┘      └───────────┘      └────────┘      └──────────┘      └─────────┘
```

## Functional Behavior
1. **Selection:** Clicking a module button (e.g., "OSC 1" or "MUTATOR 1") instantly updates the main [Synth Module](./04_synth_modules.md) area to show that module's parameters and visualization.
2. **State Indication:**
   - **Active:** The selected module button is highlighted with its specific brand color (Cyan for Osc/Mutator, Teal for Filter, etc.) and an outer glow.
   - **Inactive:** Muted gray background with subtle outline.
   - **Bypassed:** Strikethrough or dimmed label if the module is turned off.
3. **Signal Animation:** Subtle "pulse" animations on the connection lines can indicate the presence of audio signal or modulation activity.

## Implementation Details
- **JUCE Component:** Subclass of `juce::Component`.
- **Buttons:** Custom `NeonModuleButton` subclasses that handle the selection state.
- **Drawing:** Use `juce::Path` for the connection lines with `juce::VBlankListener` or a timer for glow pulses.
- **Dynamic Composition:** The panel should be configurable via a data structure (e.g., a "Module Map") so different synths in the Neon Factory can have different architectures (e.g., a 3-oscillator synth vs. a FM-pair synth).

## Controls Summary
- **Module Buttons:** Selects the target module.
- **Signal Flow Lines:** Visual feedback and aesthetic branding.
- **Global Edit State:** A "Home" or "Global" button to see master settings/mixer.
