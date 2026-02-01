# Neon Signal Mixer

## Overview
The **Signal Mixer** is the convergence point for all raw sound sources (Oscillators, Mutators, and Noise) before they enter the Filter and Amplifier stages. In the "Neon" brand, the mixer is designed to be both a balancing tool and a source of pre-filter saturation.

## Mixer Components

### 1. Source Channels
Dedicated input strips for each sound generator:
- **OSC 1 & 2:** Primary inputs.
- **Noise:** Dedicated noise channel.
- **Sub/Aux:** For sub-oscillators or external inputs.

### 2. Pre-Filter Drive
Every channel features an optional "Heat" stage that adds harmonic saturation before the signal reaches the filter. This allows for aggressive "driven" tones even with cold digital filters.

## Parameter Cards
- **OSC 1 Level:** (Linear) Volume for Oscillator 1.
- **OSC 2 Level:** (Linear) Volume for Oscillator 2.
- **Noise Level:** (Linear) Volume for the Noise generator.
- **Mixer Drive:** (Linear) Global pre-filter saturation amount.
- **Width:** (Linear) Stereo spread of the combined signals.
- **Phase Invert:** (Toggle) Flip phase of OSC 2 for cancellation effects.
- **Mute/Solo:** (Toggle) Channel state management.

## Visual Representation (Top 2/3)
- **Spectral Balance:** A real-time FFT analyzer showing the combined frequency content of all sources.
- **Color:** A gradient from **Oscillator Cyan (`0xff00aaff`)** to **Neon Orange (`0xffffaa00`)** depending on the Drive level.
- **Input Indicators:** Vertical "Glow Bars" for each channel showing individual pre-mix levels.

## JUCE Implementation
- **Mixing:** Standard summing of audio buffers.
- **Level Control:** Uses `juce::dsp::Gain`.
- **Saturation:** Uses `juce::dsp::WaveShaper`.
