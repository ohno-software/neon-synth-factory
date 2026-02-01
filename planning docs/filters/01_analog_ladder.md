# Neon Filter Type: Analog Ladder

## Overview
Classic "Vintage" style low-pass filters modeled after transistor and diode ladder circuits. These provide the characteristic warmth and aggressive resonance associated with classic subtractive synthesis.

## Filter Models

### 1. Transistor Ladder (24dB/Oct)
Modeled after the classic Moog architecture. Known for its steep slope and significant low-end compensation (or loss) at high resonance.
- **Character:** Creamy, thick, and powerful.
- **Resonance:** Deep and "growly."

### 2. Diode Ladder (18dB/Oct)
Inspired by the TB-303 and Roland SH-101. The non-linearities in the diode stages create a unique "acid" squelch.
- **Character:** Sharp, biting, and highly musical when saturated.
- **Resonance:** Liquid and nasal.

## Parameter Cards
- **Type:** (Toggle) Switch between Transistor (24dB) and Diode (18dB).
- **Cutoff:** (Linear) 20Hz - 20kHz.
- **Resonance:** (Linear) 0% to "Self-Oscillation."
- **Drive:** (Linear) Input saturation to hit the ladder stages harder.
- **Slope:** (Toggle) 12dB / 18dB / 24dB behavior.
- **Keytrack:** (Linear) Cutoff tracking of MIDI note pitch.

## Visual Representation (Top 2/3)
- **Bode Plot:** A steep frequency response curve in **Filter Teal (`0xff00ffaa`)**.
- **Saturation Glow:** The area under the curve glows slightly orange when the **Drive** is high, indicating internal clipping.
