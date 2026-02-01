# Neon Oscillator Mutators

## Overview
Based on the Hydrasynth "Mutator" concept, FM and other waveform transformations are not oscillators themselves, but **Routing Blocks** that sit after the primary oscillators. This allows any oscillator type (VA, Wavetable, PD, Noise) to act as a source for complex modulation.

## Mutator Types

### 1. FM (Linear FM)
Traditional Frequency Modulation where one source (Modulator) affects the frequency of another (Carrier).
- **Source Selection:** Choose between OSC 1, OSC 2, Noise, or a dedicated sub-oscillator.
- **Ratio:** The frequency relationship between the two sources.
- **Depth:** The intensity of the modulation.
- **Feedback:** The ability for the mutator to modulate itself.

### 2. Sync (Soft/Hard)
Slave oscillator reset by a master source.
- **Source:** The master oscillator window.
- **Ratio:** The pitch offset for the sync effect.

### 3. PWM / Harmonic Warp
Squashing or stretching the waveform phase.
- **Warp Amount:** How much the waveform is compressed.
- **Center:** The focal point of the warp.

### 4. Wavestacker
Creates multiple detuned copies of the waveform (Unison-style) within a single oscillator path.
- **Count:** Number of voices (up to 8).
- **Detune:** Spread of frequencies.

## Parameter Cards
- **Mutator Type:** (Toggle) Choose FM, Sync, Warp, etc.
- **Source:** (Toggle) Select which oscillator is the modulator.
- **Depth:** (Linear) The primary intensity of the effect.
- **Feedback:** (Linear) Secondary control for character.
- **Dry/Wet:** (Linear) Balance between transformed and original signal.

## Visual Representation (Top 2/3)
- **Comparison View:** Shows the "Pre-Mutator" waveform (dimmed) overlayed with the "Post-Mutator" waveform (bright Cyan) to clearly show how the sound is being mangled.
- **FM Lissajous:** For FM mode, shows the phase relationship between the carrier and modulator.

## Signal Flow Placement
In the **Module Selection Panel**, Mutators appear as blocks between the Oscillators and the Filter:
`OSC 1` -> `MUTATOR 1` -> `MIXER` -> `FILTER`
