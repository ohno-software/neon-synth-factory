# Neon Filter Type: Analog Highpass

## Overview
A dedicated analog-modeled high-pass filter, essential for removing mud from a mix or creating thin, biting lead sounds. Inspired by the sallen-key and Korg-style high-pass circuits.

## Filter Models

### 1. Sallen-Key Highpass (12dB/Oct)
Focuses on a smooth, musical cutoff with a gentle slope.
- **Character:** Natural and transparent.

### 2. Aggressive HP (24dB/Oct)
A steeper, "Korg-style" circuit that can be driven into screamed resonance.
- **Character:** Harsh, biting, and resonant.

## Parameter Cards
- **Cutoff:** (Linear) High-pass cutoff frequency.
- **Resonance:** (Linear) Peak boost at the cutoff point.
- **Drive:** (Linear) Analog-style soft clipping.
- **Slope:** (Toggle) 12dB vs 24dB.
- **Keytrack:** (Linear) Tracking of MIDI note pitch.

## Visual Representation (Top 2/3)
- **Highpass Curve:** An inverse Bode plot showing frequencies being cut from the left.
- **Color:** Filter Teal (`0xff00ffaa`).
