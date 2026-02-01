# Neon Synth Factory - Synth Modules

A **Synth Module** is a discernable, functional part of a synthesizer. In the Neon brand, every Synth Module follows a strict vertical layout template and is accessed via a centralized **Module Selection Panel**.

## Module Selection Panel
Inspired by the Hydrasynth and Ensoniq ESQ-1, the [Module Selection Panel](./07_module_selection_panel.md) is a visual block diagram of the synthesizer. Selecting a block in this panel brings the corresponding module into focus in the main editor area.

## Module Layout Template

```
┌─────────────────────────────────────────────────────────────┐
│                                                             │
│                    MODULE VISUAL DISPLAY                    │
│                                                             │
│             (Occupies TOP 2/3 of Module area)               │
│         Waveform / ADSR Graph / Filter Curve / etc.         │
│                                                             │
├─────────────────────────────────────────────────────────────┤
│                                                             │
│       [ P1 ]    [ P2 ]    [ P3 ]    [ P4 ]                  │
│       [ P5 ]    [ P6 ]    [ P7 ]    [ P8 ]                  │
│                                                             │
│             (Occupies BOTTOM 1/3 of Module area)            │
│               2 Rows of 4 Parameter Cards                   │
│                                                             │
├─────────────────────────────────────────────────────────────┤
│             <      ○      ●      ○      ○      >            │
│           (Paging Controls - HORIZONTALLY CENTERED)         │
└─────────────────────────────────────────────────────────────┘
```

### Layout Components
- **Top 2/3 (Display):** Provides immediate visual feedback of the module's state using JUCE-native graphics.
- **Bottom 1/3 (Parameters):** A grid of 8 parameter cards (2x4).
- **Paging Controls (Footer):** 
  - **Position:** Horizontally centered at the very bottom of the module.
  - **Left/Right Arrows:** Increment/Decrement parameter pages.
  - **Page Dots (Circles):** Direct access to specific parameter pages (Filled dot = active page).

---

## 1. Oscillator Module
*Accent Color: Oscillator Cyan (`0xff00aaff`)*
- **Details:** [Oscillators Subdirectory](./oscillators/)
- **Primary Parameters:** Shape, Detune, Octave, Semi, Fine, Level, Pan, Width.

## 2. Mutator Module
*Accent Color: Oscillator Cyan (`0xff00aaff`)*
- **Details:** [Oscillator Mutators](./oscillators/05_oscillator_mutators.md)
- **Primary Parameters:** Mutant Type, Source, Depth, Feedback, Ratio, Dry/Wet, Fallback, Reserved.

## 3. Mixer Module
*Accent Color: Oscillator Cyan (`0xff00aaff`)*
- **Details:** [Signal Mixer](./routing_and_mixing/01_signal_mixer.md)
- **Primary Parameters:** OSC 1 Level, OSC 2 Level, Noise Level, Mixer Drive, Width, Reserved 1, Reserved 2, Reserved 3.

## 4. Filter Module
*Accent Color: Filter Teal (`0xff00ffaa`)*
- **Details:** [Filters Subdirectory](./filters/)
- **Primary Parameters:** Cutoff, Resonance, Drive, Mode, Slope, Keytrack, Env Depth, LFO Depth.

## 5. Amplifier Module
*Accent Color: Volcanic Orange (`0xffff4d00`)*
- **Details:** [Amplifier Module](./amplifier/01_amplifier_module.md)
- **Primary Parameters:** Master, Pan, Soft-Clip, Velocity, Trim, Spreads, Routing, Output.

## 6. Effect Module
*Accent Color: Sky Blue (`0xff0088ff`)*
- **Details:** [Effects Module](./effects/01_effects_base.md)
- **Primary Parameters:** Type, Mix, Time/Rate, Feedback/Depth, Tone, Width, Sync, Mode.

## 7. Envelope Module
*Accent Color: Modulation Orange (`0xffffaa00`)*
- **Details:** [Envelope Module](./modulation/02_envelope_module.md)
- **Primary Parameters:** Attack, Decay, Sustain, Release, A-Curve, D-Curve, R-Curve, Velocity.

## 8. LFO Module
*Accent Color: Modulation Orange (`0xffffaa00`)*
- **Details:** [LFO Module](./modulation/01_lfo_module.md)
- **Primary Parameters:** Rate, Sync, Shape, Phase, Trigger, Fade-In, Smoothing, Steps.

## 9. Performance Controls
*Accent Color: Modulation Orange (`0xffffaa00`)*
- **Details:** [Performance Controls](./modulation/03_performance_controls.md)
- **Primary Parameters:** PB Range, Velo Curve, AT Smooth, CC A, CC B, CC C, CC D, Mono/Poly AT. (Page 2: Ribbon, XY, HID).

## 10. Macro Controls
*Accent Color: Oscillator Cyan (`0xff00aaff`)*
- **Details:** [Macro Controls](./07_macro_controls.md)
- **Primary Parameters:** Macro 1, Macro 2, Macro 3, Macro 4, Macro 5, Macro 6, Macro 7, Macro 8.

## 11. Arpeggiator & Sequencer
*Accent Color: Pan Pink (`0xffFF00AA`)*
- **Details:** [Arpeggiator & Sequencer](./sequencing/01_arpeggiator.md)
- **Primary Parameters:** Mode, Rate, Direction, Range, Gate, Swing, Steps, Oct Shift.

## 12. Voice & Unison
*Accent Color: Oscillator Cyan (`0xff00aaff`)*
- **Details:** [Voice & Unison Management](./voice/01_voice_settings.md)
- **Primary Parameters:** Voices, Unison, Detune, Spread, Glide, G-Mode, Priority, Retrig.

## 13. Matrix Router Module
*Accent Color: Modulation Orange (`0xffffaa00`)*
- **Details:** [Matrix Router](./routing_and_mixing/02_matrix_router.md)
- **Primary Parameters:** Slot, Source, Destination, Amount, Via, Via-Amt, Smooth, Curve.

## 14. Global Config Panel
- **Details:** [Global Config Panel](./09_global_config_panel.md)
- **Primary Parameters:** Patch Search, Category, Author, Tune, Voices, Theme, MIDI In, MIDI Out.

