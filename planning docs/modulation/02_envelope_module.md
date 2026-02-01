# Neon Envelope Module

## Overview
The **Envelope Module** generates transient modulation signals. To match the Hydrasynth's flexibility, we use a **DAHDSR** architecture (Delay, Attack, Hold, Decay, Sustain, Release) with adjustable curves for every segment.

## Envelope Segments
- **Delay:** Time before the attack starts after a note-on.
- **Attack:** Time to reach peak level (0dB).
- **Hold:** Duration to stay at peak level before starting the decay.
- **Decay:** Time to transition from peak to sustain level.
- **Sustain:** Constant level held while the note is depressed.
- **Release:** Time to return to zero after note-off.

## Parameter Cards
- **Attack:** (Linear) Time (ms).
- **Decay:** (Linear) Time (ms).
- **Sustain:** (Linear) Level (0% to 100%).
- **Release:** (Linear) Time (ms).
- **A-Curve:** (Linear) From Logarithmic to Exponential.
- **D-Curve:** (Linear) From Logarithmic to Exponential.
- **R-Curve:** (Linear) From Logarithmic to Exponential.
- **Velocity:** (Linear) How much note velocity scales the envelope depth.

## Visual Representation (Top 2/3)
- **ADSR Graph:** A filled silhouette of the envelope shape in **Envelope Green (`0xff00ff00`)**.
- **Nodes:** Large, glowing circular nodes at the start/end of each segment that can be "implied" (not directly draggable but provide visual focus).
- **Real-time Cursor:** A bright vertical line that scans across the shape as notes are played to track the current phase.

## JUCE Implementation
- **Envelope Engine:** Uses `juce::ADSR` for basic logic, or a custom multi-segment ramp class for the full DAHDSR experience.
