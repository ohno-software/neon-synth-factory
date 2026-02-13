# Neon Jr Synthesizer Sound Recipe Book (LLM Edition)

This document provides structured "recipes" for generating specific sonic archetypes for the **Neon Jr** engine. Use these as templates for automated patch generation.

---

## 1. The Punchy Sub Bass
**Goal:** A solid low-end foundation with a percussive click.

```json
{
    "name": "Solid Sub Bass",
    "parameters": {
        "Oscillator 1/Volume": 0.5,
        "Oscillator 1/Waveform": 357, 
        "Sub Osc/Volume": 0.9,
        "Sub Osc/Octave": 1.0,
        "Ladder Filter/Cutoff": 150.0,
        "Ladder Filter/Res": 0.1,
        "Ladder Filter/Drive": 2.0,
        "Amp Env/Attack": 2.0,
        "Amp Env/Decay": 400.0,
        "Amp Env/Sustain": 0.6,
        "Amp Env/Release": 50.0,
        "Filter Env/Attack": 0.0,
        "Filter Env/Decay": 150.0,
        "Filter Env/Amount": 0.3,
        "Control/Mode": 1.0
    }
}
```
*   **Technique:** Use `Sub Osc/Octave: 1.0` (-1 Octave) for the weight. Set `Filter Env/Decay` to <200ms with a small positive `Amount` to create the "thump" at the start.

---

## 2. Supersaw Lead
**Goal:** Huge, wide, and aggressive harmonic content.

```json
{
    "name": "Neon Supersaw",
    "parameters": {
        "Oscillator 1/Volume": 0.8,
        "Oscillator 1/Waveform": 124,
        "Oscillator 1/Unison": 4.0,
        "Oscillator 1/USpread": 0.6,
        "Oscillator 1/Detune": 15.0,
        "Ladder Filter/Cutoff": 5000.0,
        "Ladder Filter/Drive": 1.5,
        "FX/Mod Type": 0.0,
        "FX/Mod Mix": 0.4,
        "FX/Dly Mix": 0.2,
        "FX/Dly Time": 300.0,
        "FX/Rvb Mix": 0.3,
        "Amp Env/Sustain": 1.0
    }
}
```
*   **Technique:** Maximize `Oscillator 1/Unison` to 4.0. Increase `USpread` for stereo width. Add `FX/Mod Mix` (Chorus) to smooth the detuning.

---

## 3. Etherial Ambient Pad
**Goal:** Soft build-up, evolving texture, and vast space.

```json
{
    "name": "Cloud Surfer",
    "parameters": {
        "Oscillator 1/Volume": 0.6,
        "Oscillator 1/Waveform": 160,
        "Oscillator 1/Symmetry": 0.3,
        "Ladder Filter/Cutoff": 800.0,
        "Ladder Filter/Res": 0.2,
        "Amp Env/Attack": 1500.0,
        "Amp Env/Release": 2000.0,
        "LFO 1/Shape": 0.0,
        "LFO 1/Rate Hz": 0.2,
        "LFO 1/Slot 1 Target": 2.0,
        "LFO 1/Slot 1 Amount": 0.4,
        "FX/Rvb Mix": 0.6,
        "FX/Rvb Size": 0.9,
        "Control/Mode": 0.0
    }
}
```
*   **Technique:** Use long `Attack` and `Release` (>1000ms). Target `Osc1 Symmetry` (Target 2) with a slow LFO to create movement in the timbre over time. High `Rvb Mix` is essential.

---

## 4. Digital Pluck (Physical Modeling Style)
**Goal:** Sharp, "plucked string" or "wooden" impact.

```json
{
    "name": "Log Drum",
    "parameters": {
        "Oscillator 1/Waveform": 124,
        "Oscillator 1/Fold": 0.7,
        "Ladder Filter/Cutoff": 100.0,
        "Ladder Filter/Res": 0.7,
        "Filter Env/Attack": 0.0,
        "Filter Env/Decay": 80.0,
        "Filter Env/Sustain": 0.0,
        "Filter Env/Amount": 0.8,
        "Pitch Env/Attack": 0.0,
        "Pitch Env/Decay": 40.0,
        "Pitch Env/Amount": 0.2,
        "Control/Mode": 1.0
    }
}
```
*   **Technique:** High `Res` with a very fast `Filter Env/Decay` creates the "knock". `Pitch Env/Amount` at a short decay adds the transient click. Use `Fold` to add non-harmonic grit.

---

## 5. Glitchy Arp Sequence
**Goal:** Rhythmic, shifting, and precise.

```json
{
    "name": "Binary Rain",
    "parameters": {
        "Oscillator 1/Waveform": 10,
        "Oscillator 1/BitRedux": 0.6,
        "Arp/Arp On": 1.0,
        "Arp/Mode": 3.0,
        "Arp/Rate Note": 2.0,
        "Arp/Gate": 50.0,
        "Ladder Filter/Cutoff": 2000.0,
        "LFO 1/Shape": 4.0,
        "LFO 1/Rate Note": 2.0,
        "LFO 1/Sync": 1.0,
        "LFO 1/Slot 1 Target": 5.0,
        "LFO 1/Slot 1 Amount": 0.8
    }
}
```
*   **Technique:** Set `Arp/Mode: 3.0` (Random). Use `LFO 1/Shape: 4.0` (S&H) synced to the same rate as the Arp, targeting `BitRedux` (Target 5) to change the "crush" amount on every note.

---

## LLM Logic Tips for Custom Sounds

1.  **Timbre Control:** For darker sounds, keep `Ladder Filter/Cutoff` below 1000.0. For brighter/sharper sounds, go above 4000.0.
2.  **Width:** For mono-compatible width, use `Oscillator 1/Unison: 2.0` and `USpread: 0.1-0.3`. For "super wide", use `Unison: 4.0` and `FX/Mod Type: 0.0` (Chorus).
3.  **Tension:** Use `Ladder Filter/Res` between 0.4 and 0.8 to add a "singing" or "squelchy" quality.
4.  **Dynamics:** Always map `Amp Env` sustain based on the needed note length. Percussive sounds = 0.0 Sustain. Sustained drones = 1.0 Sustain.
