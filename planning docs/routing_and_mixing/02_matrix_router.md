# Neon Matrix Router

## Overview
The **Matrix Router** is the brain of the instrument's modulation architecture. It allows "any-to-any" routing of modulation sources (Envelopes, LFOs, Velocity) to destinations (Cutoff, Pitch, Wavetable Position).

## Routing Logic

### 1. Slot-Based Matrix
A list of modulation "slots" where each slot defines:
- **Source:** The modulation generator (e.g., LFO 1).
- **Destination:** The parameter to be modulated (e.g., Filter 1 Cutoff).
- **Amount:** The intensity and polarity (bipolar) of the modulation.
- **Modifier/Via:** An optional secondary source that controls the modulation amount (e.g., Mod Wheel controlling LFO depth).

### 2. Signal Types
- **Unipolar:** 0.0 to 1.0 (typical for Envelopes).
- **Bipolar:** -1.0 to 1.0 (typical for LFOs).

## Parameter Cards
- **Slot Select:** (Toggle/Page) Navigate through the available modulation slots.
- **Source:** (Toggle/List) Choose the modulation source.
- **Dest:** (Toggle/List) Choose the modulation destination.
- **Amount:** (Linear) The depth of modulation (-100% to +100%).
- **Via Source:** (Toggle) Choose an optional controller (Mod Wheel, Aftertouch).
- **Via Amount:** (Linear) Intensity of the 'Via' source.
- **Lag/Smooth:** (Linear) Softens the modulation transitions to prevent clicking.

## Visual Representation (Top 2/3)
- **Modulation Flow Graph:** A stylized, animated flow diagram showing sources on the left and destinations on the right, connected by glowing "Data Streams."
- **Activity Glow:** Lines pulse or brighten based on the real-time intensity of the modulation.
- **Color:** **Modulation Orange (`0xffffaa00`)**.

## Implementation Details
- **JUCE Component:** A scrolled list of `NeonParameterCard` sets or a dedicated grid view.
- **Logic:** A robust `ModulationMatrix` C++ class that updates parameter offsets every audio block or at a sub-sample rate.
