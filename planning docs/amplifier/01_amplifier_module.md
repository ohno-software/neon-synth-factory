# Neon Amplifier Module

## Overview
The **Amplifier Module** is the final stage of the synth's signal path. It manages the output volume, stereo image, and final harmonic character via the "Neon Glow" saturation circuit.

## Amplifier Components

### 1. VCA Model (Voltage Controlled Amplifier)
Determines the response curve and harmonic character of the volume control.
- **Modern (Linear):** Perfectly clean, transparent volume adjustment.
- **Vintage (Exponential):** Models the non-linear response of classic hardware VCAs.
- **Saturation Gate:** Adds subtle harmonic drive as the gain increases.

### 2. Output Stage & "Neon Glow"
Final safety and coloring stage.
- **Soft-Clip:** A smooth saturation curve that prevents digital clipping by adding warmth.
- **Hard-Clip:** More aggressive clipping for distorted digital textures.
- **Limiter:** Transparent peak protection to ensure output stays below 0dB.

## Parameter Cards
- **Gain / Master:** (Linear) Final output level (-inf to +12dB).
- **Pan:** (Linear) Stereo positioning (L/R).
- **Drive:** (Linear) Input gain into the saturation stage.
- **VCA Mode:** (Toggle) Choose between Modern, Vintage, or Saturated.
- **Velocity:** (Linear) How much MIDI velocity affects the final output level.
- **Spread:** (Linear) Widens the stereo image by applying micro-delays or pan offsets.
- **Clip Mode:** (Toggle) Soft-Clip / Hard-Clip / Limiter / Off.

## Visual Representation (Top 2/3)
- **Level Visualization:** A real-time stereo "Glow Meter" in **Volcanic Orange (`0xffff4d00`)**.
- **Saturation Indicator:** The background of the display pulses with a subtle "Heat Glow" when the saturation or clipping is active.
- **Stereo Field:** A goniometer (stereo vector scope) showing the width and phase of the output signal.

## JUCE Implementation
- **Volume/Gain:** Uses `juce::dsp::Gain`.
- **Saturation:** Uses `juce::dsp::WaveShaper` with a custom sigmoid or tangent function.
- **Panning:** Uses `juce::dsp::Panner`.
- **Limiter:** Uses `juce::dsp::Limiter`.
