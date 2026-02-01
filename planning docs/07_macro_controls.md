# Neon Macro Controls

## Overview
The **Macro Module** provides a single point of control for multiple parameters across the entire synthesizer. This is the "Performer's Dashboard," allowing drastic sonic changes with a single movement.

## Features

### 1. Macro Mapping
Each of the 8 Macros can be assigned to an unlimited number of parameters via the [Matrix Router](../routing_and_mixing/02_matrix_router.md).

### 2. Macro Names
Users can assign short, 5-character names (in 7-segment style) to each macro (e.g., "GRIME", "SPACE", "WARP").

## Parameter Cards
- **Macro 1-8:** (Linear) The current value of the macro (0 to 127).
- **Smooth:** (Linear) Global smoothing for macro movements to prevent jumps.

## Visual Representation (Top 2/3)
- **Macro Matrix:** A set of 8 large, glowing "Power Meters" representing the macro levels.
- **Assignment Map:** Small text labels showing what each macro is controlling (summarized).
- **Color:** **Oscillator Cyan (`0xff00aaff`)** or **Value Indicator White (`0xffffffff`)**.

## Implementation Details
- **Control Logic:** Macros act as modulation sources in the Matrix. 
- **Persistence:** Macro assignments and names are saved per-patch.
