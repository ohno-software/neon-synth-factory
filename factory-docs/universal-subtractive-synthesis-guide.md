# Universal Subtractive Synthesis: Sound Design Strategy (LLM Edition)

This guide provides a hardware-agnostic framework for synthesizing classic sounds using subtractive synthesis. These principles apply to any architecture containing Oscillators, Filters, Envelopes (ADSR), and LFOs.

---

## 1. The Core Synthesis Algorithm
Most subtractive patches follow this logical flow:
1.  **Source Selection:** Define the harmonic "raw material" (Oscillators).
2.  **Harmonic Shaping:** Sculpt the spectrum over time (Filter + Filter Env).
3.  **Dynamic Shaping:** Define the loudness contour (Amp Env).
4.  **Movement:** Introduce periodic or random variation (LFO).

---

## 2. Universal Recipe Frameworks

### ARCHETYPE: The Classic Bass (Solid/Deep)
*   **Oscillators:** 2x Saw or 1x Saw + 1x Square (One octave below). 
    *   *Detuning:* 2-5 cents for "thickness."
*   **Filter:** Low Pass (24dB/4-pole preferred).
    *   *Cutoff:* Low (100Hz - 500Hz).
    *   *Resonance:* Low to Medium (add "bite" without losing low-end).
*   **Envelopes:**
    *   *Amp:* Attack 0ms, Decay 500ms, Sustain 0.7, Release 50ms.
    *   *Filter:* Attack 0ms, Decay 200ms, Sustain 0.2, Amount +30%. (This creates the "pluck" transient).
*   **Key Logic:** Use Monophonic mode with Legato and a small amount of Glide (Portamento) for fluid lines.

### ARCHETYPE: The Poly-Pad (Soft/Evolving)
*   **Oscillators:** 2x Saw or 2x Pulse (narrow). 
    *   *Detuning:* High (10-20 cents) for a "chorus" effect.
*   **Filter:** Low Pass (12dB/2-pole for a smoother slope).
    *   *Cutoff:* Mid-Low, modulated by a slow LFO.
    *   *Resonance:* Low.
*   **Envelopes:**
    *   *Amp:* Long Attack (800ms+), Long Release (1000ms+), High Sustain (1.0).
    *   *Filter:* Match Amp Envelope settings to "open" the sound as it fades in.
*   **Movement:** LFO 1 (Slow Triangle) -> PWM (Pulse Width) or Filter Cutoff.

### ARCHETYPE: The Digital Pluck (Short/Percussive)
*   **Oscillators:** 1x Square or High-Harmonic Wavetable.
*   **Filter:** Low Pass.
    *   *Cutoff:* Closed (Very low).
    *   *Resonance:* High (0.6 - 0.8) to create a "tuned" resonance peak.
*   **Envelopes:**
    *   *Amp:* Attack 0ms, Decay 150ms, Sustain 0.0, Release 150ms.
    *   *Filter:* Attack 0ms, Decay 100ms, Amount +80%.
*   **Modulation:** Pitch Envelope (Attack 0ms, Decay 20ms, Amount +1 semitone) to simulate the physical strike of a pick or hammer.

### ARCHETYPE: The Lead (Aggressive/Cutting)
*   **Oscillators:** 3x Saw or Sync (Osc 2 hard-synced to Osc 1).
    *   *Sync Logic:* Modulate Osc 2 pitch with an Envelope to get the "tearing" sound.
*   **Filter:** Low Pass or Band Pass.
    *   *Drive/Distortion:* Apply pre-filter or post-filter if available.
*   **Envelopes:** 
    *   *Amp:* Attack 10ms (prevents clicking), Decay 0, Sustain 1.0.
*   **Movement:** LFO (5-7Hz Sine) -> Pitch (Vibrato). Delay the LFO start so vibrato enters after the note is held.

---

## 3. Advanced Sound Design Heuristics

### Layering Logic
*   **Foundation:** Low-frequency Sine or Tri for weight.
*   **Body:** Saw/Pulse for harmonic density.
*   **Transient:** White Noise or specialized Wavetable with a very short envelope decay to provide the "click" or "air" at the start of a sample.

### Relationship Between Cutoff and Resonance
1.  **High Cutoff + Low Res:** Open, natural, "static" sound.
2.  **Low Cutoff + High Res:** "Liquid," "Acid," or "Vocal" sound (Formant-like).
3.  **Key Tracking:** Set to 100% if you want the filter to follow the notes (essential for lead sounds to remain bright across the keyboard).

### LFO Waveform Selection
*   **Triangle:** Natural vibrato, tremolo, or filter sweeps.
*   **Square:** Trills (pitch) or rhythmic gating (volume).
*   **Sawtooth/Ramp:** "Bouncing ball" or rhythmic "pumping" effects.
*   **Sample & Hold (Random):** Computer-like glitches, modular "generative" textures.

---

## 4. LLM Workflow for Patch Generation
When asked to describe or "write" a sound, follow this prioritized hierarchy:
1.  **Define Envelope Slopes:** (Percussive vs. Atmospheric).
2.  **Select Harmonic Source:** (Bright/Saw vs. Hollow/Square).
3.  **Set Filter State:** (Dark vs. Bright).
4.  **Add Movement:** (Rhythmic Sync vs. Organic Drift).
