# Neon Filter Type: Formant and Vocal

## Overview
A series of narrow band-pass filters (parallel peaks) that simulate the human vocal tract, creating "A-E-I-O-U" vowel sounds.

## Components

### 1. Vowel Morphing
A 2D space where the user can morph between different vowel sounds (e.g., transitioning from "Oo" to "Ah").

### 2. Multi-Peak Resonator
Standard parallel filters used to emulate wooden enclosures (Violin, Cello) or metallic pipes.

## Parameter Cards
- **Vowel:** (Linear) Morphs smoothly through the standard A-E-I-O-U table.
- **Formant Shift:** (Linear) Shifts all peaks up or down to simulate a larger or smaller "throat."
- **Q / Width:** (Linear) How narrow/sharp the vocal peaks are.
- **Spread:** (Linear) Widens the frequency distance between formants.
- **Vowel Select:** (Toggle/Page) Choose between male, female, or "robot" vowel sets.

## Visual Representation (Top 2/3)
- **Vowel Map:** A 2D grid (XY-style) showing the position of the current sound within a phonetic map.
- **Spectrum:** Three distinct "hills" (representing formants F1, F2, F3) that shift horizontally as the user adjusts the vowel or formant shift.
- **Color:** Filter Teal (`0xff00ffaa`).
