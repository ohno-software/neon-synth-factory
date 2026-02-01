# Neon Modulation: Performance Controls

## Overview
Performance Controls capture human interaction from MIDI inputs and MPE controllers. These serve as high-priority sources in the Modulation Matrix.

## Sources

### 1. Keyboard (Keytracking)
- **Key Track:** Maps note number (0-127) to modulation.
- **Center Key:** The "pivot" note where modulation is zero.
- **Curve/Scale:** Linear or custom response curves for how pitch affects parameters.

### 2. Velocity
- **Velocity Track:** Maps strike velocity to modulation.
- **Response Curve:** Fixed set of curves (Soft 1/2, Linear, Hard 1/2).

### 3. Aftertouch (Polyphonic by Default)
- **Mode:** Polyphonic (individual keys) or Channel (global).
- **Default:** Always defaults to Polyphonic if the hardware supports it.
- **Smoothing:** A "Lag" parameter specifically for aftertouch to prevent jittery pressure changes.

### 4. Ribbon Controller
- **Mode:** Absolute (0-127 based on position) or Relative (increments from current touch).
- **Hold:** When released, stay at the last value or snap back to center/zero.
- **Length:** Internal scaling for the virtual ribbon UI.

### 5. XY Pad
- **Dimensions:** Two independent modulation sources (X and Y).
- **Physics:** Optional "Inertia" or "Spring" behavior where the cursor drifts or snaps back.

### 6. External HID (Joystick / Gamepad)
- **Source:** Map standard OS HID axes (X, Y, Z, Rotation) to modulation slots.
- **Calibration:** Deadzone and Sensitivity controls for game controllers.

### 7. MIDI CC (General Purpose)
- **CC Selector:** Ability to pick any 4 standard CC numbers to act as direct modulation sources.

## Parameter Cards (Page 1: Traditional)
- **PB Range:** (Linear) Semitones for Pitch Bend.
- **Velo Curve:** (Page/Toggle) Selection of response curves.
- **AT Smooth:** (Linear) Smoothing time for pressure.
- **AT Mode:** (Toggle) Polyphonic vs Channel.
- **CC A Select:** (Linear) MIDI CC Number.
- **CC B Select:** (Linear) MIDI CC Number.
- **CC C Select:** (Linear) MIDI CC Number.
- **CC D Select:** (Linear) MIDI CC Number.

## Parameter Cards (Page 2: Physical/HID)
- **Ribbon Mode:** (Toggle) Absolute vs Relative.
- **Rib Mode:** (Toggle) Hold vs Reset.
- **XY Physics:** (Toggle) None, Spring, Inertia.
- **XY Spring:** (Linear) Snap speed to center.
- **HID Axis X:** (Toggle) Map Joystick X.
- **HID Axis Y:** (Toggle) Map Joystick Y.
- **HID Deadzn:** (Linear) Joystick deadzone.
- **HID Sens:** (Linear) Joystick sensitivity.

## Visual Representation (Top 2/3)
- **MIDI Activity monitor:** Real-time dots lighting up for Notes, Velocity bars, and Aftertouch pressure waves.
- **Response Curves:** Graphic display of the selected Velocity/Aftertouch curves.
- **Physical Controls:** A large XY Pad and a horizontal Ribbon display that respond to mouse/touch and HID input.
- **Color:** Modulation Orange (`0xffffaa00`).
