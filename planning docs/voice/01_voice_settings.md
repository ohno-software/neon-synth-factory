# Neon Voice & Unison Management

## Overview
The **Voice Module** controls how the synthesizer allocates its resources and handles pitch transitions. This is where the "thiccness" of the sound is defined through Unison.

## Components

### 1. Voicing
- **Polyphony:** Max number of active voices (1 to 64).
- **Legato:** Re-triggers envelopes only if no keys are held.
- **Glide (Portamento):** Time taken to slide between notes.
- **Glide Mode:** Constant Rate vs Constant Time.

### 2. Unison
- **Unison Count:** Number of detuned oscillators per note (1 to 8).
- **Detune:** The amount of pitch drift between unison voices.
- **Spread:** Stereo width of the unison voices.

## Parameter Cards
- **Voices:** (Linear) 1 to 64.
- **Unison:** (Linear) 1 to 8 voices.
- **Detune:** (Linear) Pitch spread amount.
- **Spread:** (Linear) Stereo divergence.
- **Glide:** (Linear) Portamento time.
- **G-Mode:** (Toggle) Rate vs Time.
- **Priority:** (Toggle) Last, Low, High note priority.
- **Retrig:** (Toggle) Envelope re-trigger behavior.

## Visual Representation (Top 2/3)
- **Voice Cloud:** A cluster of glowing dots representing active unison voices. They spread apart horizontally as Detune increases and widen as Spread increases.
- **Orbit Animation:** Dots may pulse or rotate to indicate phase offsets.
- **Color:** **Oscillator Cyan (`0xff00aaff`)**.
