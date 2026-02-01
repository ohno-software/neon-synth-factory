# Neon LFO Module

## Overview
The **LFO (Low Frequency Oscillator) Module** provides cyclic modulation sources. Keeping with the Hydrasynth inspiration, the LFOs are highly flexible, offering both traditional shapes and complex, user-definable Step-Sequences.

## LFO Models

### 1. Traditional (Waveshape)
Standard cyclic waveforms.
- **Shapes:** Sine, Triangle, Saw Up, Saw Down, Square, Random (S&H).
- **Smooth:** For Random and Square shapes, adds a slewing effect to the transitions.

### 2. Step LFO (Sequencer)
A rhythmic LFO where the user defines a series of steps (up to 64).
- **Step Mode:** Set individual levels for each step.
- **Interpolation:** Choose between "Jump" (square), "Line" (linear), or "Curve" (exponential) transitions between steps.

## Parameter Cards
- **Rate:** (Linear) Speed in Hz or sync'd divisions (1/4, 1/8, etc.).
- **Sync:** (Toggle) Switch between free-running (Hz) and DAW-synced (BPM).
- **Shape:** (Toggle) Cycle through the standard waveforms or Step-LFO.
- **Phase/Start:** (Linear) The starting point of the waveform on trigger.
- **Trigger:** (Toggle) Choose between Poly (one LFO per voice) and Mono (one LFO for all voices).
- **Fade-In:** (Linear) Gradually increases the modulation depth over time.
- **Smoothing:** (Linear) Adds a lag-filter to the LFO output.
- **Steps:** (Linear) Number of active steps in Step-Mode (2 to 64).

## Visual Representation (Top 2/3)
- **LFO Trace:** A glowing line showing the real-time modulation value in **Modulation Orange (`0xffffaa00`)**.
- **Playhead:** A vertical line or dot that moves across the waveform to indicate the current phase.
- **Step Background:** In Step-LFO mode, a dimmed grid appears behind the trace to show the active steps.

## JUCE Implementation
- **Standard LFO:** Uses `juce::dsp::Oscillator`.
- **Step Logic:** A custom buffer-reading class that interpolates between user-defined values.
- **Smoothing:** Uses `juce::LinearSmoothedValue`.
