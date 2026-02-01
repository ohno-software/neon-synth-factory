# Neon Oscillator Type: Wavetable

## Overview
A powerful modern oscillator that cycles through a set of single-cycle waveforms (a "table") for complex, evolving textures.

## Features
- **Table Scanning:** Smoothly interpolate between positions in the table.
- **Spectral Morphing:** Real-time FFT-based modification of the waveform (Sync, Bend, Mirror).

## Core Parameters (Card Grid)
- **Position:** Scans through the wavetable frames.
- **Warp Mode:** Selects the morphing algorithm (Bend, PWM, Sync).
- **Warp Amount:** Intensity of the spectral morph.
- **Interpolation:** Crossfade between discrete waves.
- **Table Category:** (Toggle) Choose from categories like Lumina, Radiance, or Atomic.
- **Spectral Level:** Low-pass filtering of the table harmonics before the main filter.

## Included Wave Categories (Neon H2O)
The "Neon H2O" factory library features several restructured wave categories:
- **Lumina / Radiance:** Bright, harmonics-rich digital waves.
- **Prism / Chroma:** Complex, evolving spectral textures.
- **Cobalt / Nova:** Dense, darker textures with low-end weight.
- **Razor / Impact:** Aggressive, biting waves for leads and percussion.
- **Aura / Expanse:** Wide, atmospheric pads and drones.
- **Atomic / Orbit:** Digital FM-style interactions.
- **Spectral / Vocal:** Formant and vowel-like processing.

## Visual Representation (Top 2/3)
A 3D isometric wireframe "mesh" of the wavetable, with the current position highlighted by a bright glowing scan-line.
