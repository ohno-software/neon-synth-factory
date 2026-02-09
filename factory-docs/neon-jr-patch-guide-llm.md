# Neon Jr Patch Writing Guide (LLM Edition)

This document provides the technical specification for generating `.neon` patch files for the **Neon Jr** synthesizer. It is designed to be used by an LLM to programmatically or creatively generate valid patches.

## 1. File Format & Structure
Patches are JSON-based files with a `.neon` extension. The root object contains a metadata section and a flat map of parameters.

```json
{
    "name": "Patch Name Here",
    "parameters": {
        "Category/Parameter": Value
    }
}
```

## 2. Parameter Reference
All parameters are floating-point values. Boolean toggles use `0.0` or `1.0`. Enumerated dropdowns use integer-represented floats (e.g., `2.0`).

### Oscillator 1 (`Oscillator 1/`)
| Parameter | Range | Default | Note |
| :--- | :--- | :--- | :--- |
| `Waveform` | 0.0 - 200.0+ | 124.0 | Index of the wavetable. `Saw` is typically ~124. |
| `Volume` | 0.0 - 1.0 | 0.8 | Output gain of Osc 1. |
| `Unison` | 1.0 - 4.0 | 1.0 | Number of voices (integer). |
| `USpread` | 0.0 - 1.0 | 0.2 | Stereo width/detune spread for unison. |
| `Symmetry` | 0.0 - 1.0 | 0.5 | Duty cycle or wave symmetry. |
| `Transp` | -24.0 - 24.0 | 0.0 | Semitones. |
| `Detune` | -100.0 - 100.0 | 0.0 | Cents. |
| `Phase` | 0.0 - 360.0 | 0.0 | Starting phase. |
| `KeySync` | 0.0, 1.0 | 1.0 | Reset phase on note-on. |
| `Drive` | 0.0 - 1.0 | 0.0 | Per-voice saturation. |
| `BitRedux` | 0.0 - 1.0 | 0.0 | Bitcrushing effect. |
| `Fold` | 0.0 - 1.0 | 0.0 | Wavefolding amount. |

### Sub & Noise
| Parameter | Path | Range | Note |
| :--- | :--- | :--- | :--- |
| Sub Volume | `Sub Osc/Volume` | 0.0 - 1.0 | |
| Sub Octave | `Sub Osc/Octave` | 0.0, 1.0, 2.0 | 0: -2 oct, 1: -1 oct, 2: -0.5 oct |
| Noise Volume | `Noise/Volume` | 0.0 - 1.0 | |

### Ladder Filter (`Ladder Filter/`)
| Parameter | Range | Default | Note |
| :--- | :--- | :--- | :--- |
| `Cutoff` | 20.0 - 20000.0 | 20000.0 | Frequency in Hz. |
| `Res` | 0.0 - 1.0 | 0.0 | Resonance amount. |
| `Type` | 0.0 - 3.0 | 0.0 | Filter model selection. |
| `Slope` | 0.0, 1.0 | 1.0 | 0: 12dB, 1: 24dB. |
| `Drive` | 0.0 - 10.0 | 1.0 | Input drive/saturation. |
| `KeyTrack` | 0.0 - 1.0 | 0.5 | Key tracking amount. |

### Envelopes (`Amp Env/`, `Filter Env/`, `Pitch Env/`, `Mod Env/`)
All envelopes share these core timing parameters (values in milliseconds):
| Parameter | Range | Default |
| :--- | :--- | :--- |
| `Attack` | 0.0 - 10000.0 | 10.0 |
| `Decay` | 0.0 - 10000.0 | 500.0 |
| `Sustain` | 0.0 - 1.0 | 0.7 |
| `Release` | 0.0 - 10000.0 | 500.0 |

**Envelope Special Parameters:**
- `Filter Env/Amount`: -1.0 to 1.0 (Modulation depth to filter).
- `Pitch Env/Amount`: -1.0 to 1.0 (Modulation depth to pitch).

### LFOs (`LFO 1/`, `LFO 2/`)
| Parameter | Range | Default | Note |
| :--- | :--- | :--- | :--- |
| `Shape` | 0.0 - 4.0 | 0.0 | 0:Tri, 1:RampUp, 2:RampDn, 3:Sq, 4:S&H |
| `Sync` | 0.0, 1.0 | 0.0 | 0: Hz, 1: Beats. |
| `Rate Hz` | 0.01 - 50.0 | 1.0 | |
| `Rate Note` | 0.0 - 12.0 | 4.0 | Sync index (1/4, 1/8, etc.) |
| `Slot [1-4] Target` | 0.0 - 20.0 | 0.0 | See **Modulation Targets** below. |
| `Slot [1-4] Amount` | -1.0 - 1.0 | 0.0 | Modulation intensity. |

### Effects (`FX/`)
| Parameter | Path | Range | Note |
| :--- | :--- | :--- | :--- |
| Mod Type | `FX/Mod Type` | 0.0, 1.0 | 0: Chorus, 1: Phaser |
| Mod Mix | `FX/Mod Mix` | 0.0 - 1.0 | |
| Delay Mix | `FX/Dly Mix` | 0.0 - 1.0 | |
| Delay Time | `FX/Dly Time` | 1.0 - 2000.0 | In milliseconds. |
| Reverb Mix | `FX/Rvb Mix` | 0.0 - 1.0 | |
| Reverb Size | `FX/Rvb Size` | 0.0 - 1.0 | |

### Arpeggiator (`Arp/`)
| Parameter | Range | Default | Note |
| :--- | :--- | :--- | :--- |
| `Arp On` | 0.0, 1.0 | 0.0 | |
| `Mode` | 0.0 - 3.0 | 0.0 | 0:Up, 1:Down, 2:Up-Down, 3:Random |
| `Octave` | 1.0 - 4.0 | 1.0 | Octave range. |
| `Gate` | 1.0 - 100.0 | 80.0 | Note length (%). |

## 3. Modulation Targets (Indices)
When setting `Target` parameters for LFOs or Envelopes, use these integer values:

1.  **Osc1 Pitch**
2.  **Osc1 Symmetry**
3.  **Osc1 Fold**
4.  **Osc1 Drive**
5.  **Osc1 BitRedux**
6.  **Osc1 Level**
7.  **Osc1 Pan**
8.  **Osc1 Detune**
9.  **Sub Level**
10. **Sub Pitch**
11. **Filter Cutoff**
12. **Filter Res**
13-16. **LFO 1 Slot [1-4] Amount** (Self-modulation)
17-20. **LFO 2 Slot [1-4] Amount**

## 4. Control & Master (`Control/`)
| Parameter | Range | Default | Note |
| :--- | :--- | :--- | :--- |
| `Mode` | 0.0, 1.0 | 0.0 | 0: Polyphonic, 1: Monophonic. |
| `PB Range` | 0.0 - 12.0 | 2.0 | Pitch bend range in semitones. |
| `Porta Time` | 0.0 - 1000.0 | 0.0 | Glide time in milliseconds. |
| `Tempo` | 20.0 - 300.0 | 120.0 | Internal BPM. |
| `Tempo Sync`| 0.0, 1.0 | 1.0 | Sync to host DAW. |
