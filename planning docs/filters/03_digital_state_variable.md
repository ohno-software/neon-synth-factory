# Neon Filter Type: Digital State Variable

## Overview
A modern, Zero-Delay Feedback (ZDF) implementation of the State Variable Filter (SVF). This is the choice for surgical sound design and clean, modern textures.

## Key Features
- **Perfect Transparency:** No non-linearities or unwanted distortion.
- **ZDF Stability:** Stable even with extremely fast modulation (audio-rate modulation) and high resonance values.
- **Multi-Mode Output:** Offers Low-Pass, High-Pass, Band-Pass, and Notch response.

## Parameter Cards
- **Mode:** (Toggle) LP / HP / BP / Notch.
- **Cutoff:** (Linear) High-precision frequency control.
- **Resonance:** (Linear) Clean, stable resonance peak.
- **Slope:** (Toggle) 12dB or 24dB.
- **Spread:** (Linear) For stereo cards, offsets the cutoff between Left and Right channels.
- **Keytrack:** (Linear) 0% to 200%.

## Visual Representation (Top 2/3)
- **High-Definition Curve:** A very sharp, clean Bode plot in **Filter Teal (`0xff00ffaa`)**.
- **Mode Transition:** The visualization smoothly morphs its shape when the **Mode** toggle is changed.
