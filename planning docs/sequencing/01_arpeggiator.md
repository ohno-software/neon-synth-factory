# Neon Arpeggiator & Phrase Sequencer

## Overview
The **Arpeggiator Module** transforms static chords into rhythmic patterns or complex melodic phrases. It is positioned at the start of the MIDI chain, before the voices are triggered.

## Modes

### 1. Classic Arp
Standard note-ordering logic.
- **Direction:** Up, Down, Up/Down (Inclusive/Exclusive), Random, Order Played.
- **Octave Range:** 1 to 4 octaves.
- **Gate / Length:** Duration of individual notes (0% to 100%).

### 2. Phrase Sequencer (Step Mode)
A 32-step sequencer for complex rhythmic and melodic lines.
- **Step Edit:** Per-step control over Velocity, Gate, and Pitch Offset.
- **Swing:** Global rhythmic offset for a "shuffled" feel.
- **Latch:** Holds the pattern even after keys are released.

## Parameter Cards
- **Mode:** (Toggle) Classic Arp or Step Sequencer.
- **Rate:** (Linear) Tempo-synced divisions (1/4, 1/8t, 1/16, etc.).
- **Direction:** (Toggle) Note ordering logic for Classic mode.
- **Range:** (Linear) Octave reach (1-4).
- **Gate:** (Linear) Note duration.
- **Swing:** (Linear) Rhythmic shuffle amount.
- **Steps:** (Linear) Length of the pattern in Step Mode (1-32).
- **Octave Shift:** (Linear) Global transpose of the arpeggio.

## Visual Representation (Top 2/3)
- **Step Grid:** A scrolling high-contrast grid showing active notes and their velocities.
- **Rhythm Pulse:** A glowing bar that pulses in time with the arpeggiator's rate.
- **Color:** **Pan Pink (`0xffFF00AA`)** or **Modulation Orange (`0xffffaa00`)**.

## Implementation Details
- **JUCE Class:** `juce::Arpeggiator` (base) or a custom MIDI processor.
- **Sync:** Tied to the DAW transport via `juce::AudioPlayHead`.
