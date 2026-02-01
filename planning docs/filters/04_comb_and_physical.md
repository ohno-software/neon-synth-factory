# Neon Filter Type: Comb and Physical

## Overview
Filters that use extremely short delay lines and feedback to create series of harmonic peaks and notches, useful for "Physical Modeling" sounds like strings, bells, and pipes.

## Filter Modes

### 1. Feed-Forward Comb
Creates a series of notches across the spectrum. Great for phaser-like textures.
- **Character:** Hollow, spacey.

### 2. Feedback Comb
Creates strong harmonic peaks. When tuned to MIDI pitch, it behaves like a plucked string (Karplus-Strong synthesis).
- **Character:** Metallic, resonant, melodic.

### 3. String Modeler
An optimized feedback comb with damping and diffusion to simulate acoustic vibration.

## Parameter Cards
- **Pitch/Freq:** (Linear) The fundamental frequency of the comb (often tracked to MIDI).
- **Feedback:** (Linear) The intensity of the peaks. High values create sustained ringing.
- **Damping:** (Linear) High-frequency roll-off within the feedback loop (simulates soft materials).
- **Detune:** (Linear) Offsets the peaks for a chorus-like thickness.
- **Invert:** (Toggle) Flips the phase of the feedback to shift peaks/notches.

## Visual Representation (Top 2/3)
- **Spectral Peaks:** Instead of a single curve, the display shows a forest of vertical spikes representing the harmonic series of the comb filter.
- **Color:** Filter Teal (`0xff00ffaa`) with shifting intensity based on feedback.
