#!/usr/bin/env python3
"""Create 16 musically diverse patches for Neon Jr (slots 2-17), preserving Jr. Arena"""
import json
from pathlib import Path

def base():
    """Default init parameters"""
    return {
        "Oscillator 1/Volume": 0.8, "Oscillator 1/Waveform": 195.0, "Oscillator 1/Unison": 1.0,
        "Oscillator 1/USpread": 0.2, "Oscillator 1/Symmetry": 0.5, "Oscillator 1/Transp": 0.0,
        "Oscillator 1/Detune": 0.0, "Oscillator 1/Pan": 0.0, "Oscillator 1/Drive": 0.0,
        "Oscillator 1/Fold": 0.0, "Oscillator 1/BitRedux": 0.0, "Oscillator 1/Phase": 0.0,
        "Oscillator 1/KeySync": 1.0,
        "Oscillator 2/Volume": 0.0, "Oscillator 2/Waveform": 0.0, "Oscillator 2/Unison": 1.0,
        "Oscillator 2/USpread": 0.2, "Oscillator 2/Symmetry": 0.5, "Oscillator 2/Transp": 0.0,
        "Oscillator 2/Detune": 0.0, "Oscillator 2/Pan": 0.0, "Oscillator 2/Drive": 0.0,
        "Oscillator 2/Fold": 0.0, "Oscillator 2/BitRedux": 0.0, "Oscillator 2/Phase": 0.0,
        "Oscillator 2/KeySync": 1.0,
        "Ladder Filter/Cutoff": 20000.0, "Ladder Filter/Res": 0.0, "Ladder Filter/Type": 0.0,
        "Ladder Filter/Slope": 1.0, "Ladder Filter/Drive": 1.0, "Ladder Filter/KeyTrack": 0.5,
        "Filter Env/Attack": 100.0, "Filter Env/Decay": 500.0, "Filter Env/Sustain": 0.7,
        "Filter Env/Release": 1000.0, "Filter Env/Amount": 0.0, "Filter Env/Delay": 0.0,
        "Filter Env/Hold": 0.0, "Filter Env/Env Mode": 0.0,
        "Amp Env/Attack": 10.0, "Amp Env/Decay": 500.0, "Amp Env/Sustain": 0.7,
        "Amp Env/Release": 500.0, "Amp Env/Delay": 0.0, "Amp Env/Hold": 0.0, "Amp Env/Env Mode": 0.0,
        "Pitch Env/Attack": 100.0, "Pitch Env/Decay": 500.0, "Pitch Env/Sustain": 0.7,
        "Pitch Env/Release": 1000.0, "Pitch Env/Amount": 0.0, "Pitch Env/Delay": 0.0,
        "Pitch Env/Hold": 0.0, "Pitch Env/Env Mode": 0.0,
        "Mod Env/Attack": 100.0, "Mod Env/Decay": 500.0, "Mod Env/Sustain": 0.7,
        "Mod Env/Release": 1000.0, "Mod Env/Delay": 0.0, "Mod Env/Hold": 0.0,
        "Mod Env/Env Mode": 0.0, "Mod Env/ ": 0.0,
        "Mod Env/Slot 1 Target": 0.0, "Mod Env/Slot 1 Amount": 0.0,
        "Mod Env/Slot 2 Target": 0.0, "Mod Env/Slot 2 Amount": 0.0,
        "Mod Env/Slot 3 Target": 0.0, "Mod Env/Slot 3 Amount": 0.0,
        "Mod Env/Slot 4 Target": 0.0, "Mod Env/Slot 4 Amount": 0.0,
        "LFO 1/Rate Hz": 1.0, "LFO 1/Rate Note": 4.0, "LFO 1/Sync": 0.0, "LFO 1/Shape": 0.0,
        "LFO 1/KeySync": 1.0, "LFO 1/Phase": 0.0, "LFO 1/Delay": 0.0,
        "LFO 1/Slot 1 Target": 0.0, "LFO 1/Slot 1 Amount": 0.0,
        "LFO 1/Slot 2 Target": 0.0, "LFO 1/Slot 2 Amount": 0.0,
        "LFO 1/Slot 3 Target": 0.0, "LFO 1/Slot 3 Amount": 0.0,
        "LFO 1/Slot 4 Target": 0.0, "LFO 1/Slot 4 Amount": 0.0,
        "LFO 2/Rate Hz": 1.0, "LFO 2/Rate Note": 4.0, "LFO 2/Sync": 0.0, "LFO 2/Shape": 0.0,
        "LFO 2/KeySync": 1.0, "LFO 2/Phase": 0.0, "LFO 2/Delay": 0.0,
        "LFO 2/Slot 1 Target": 0.0, "LFO 2/Slot 1 Amount": 0.0,
        "LFO 2/Slot 2 Target": 0.0, "LFO 2/Slot 2 Amount": 0.0,
        "LFO 2/Slot 3 Target": 0.0, "LFO 2/Slot 3 Amount": 0.0,
        "LFO 2/Slot 4 Target": 0.0, "LFO 2/Slot 4 Amount": 0.0,
        "LFO 3/Rate Hz": 1.0, "LFO 3/Rate Note": 4.0, "LFO 3/Sync": 0.0, "LFO 3/Shape": 0.0,
        "LFO 3/KeySync": 1.0, "LFO 3/Phase": 0.0, "LFO 3/Delay": 0.0,
        "LFO 3/Slot 1 Target": 0.0, "LFO 3/Slot 1 Amount": 0.0,
        "LFO 3/Slot 2 Target": 0.0, "LFO 3/Slot 2 Amount": 0.0,
        "LFO 3/Slot 3 Target": 0.0, "LFO 3/Slot 3 Amount": 0.0,
        "LFO 3/Slot 4 Target": 0.0, "LFO 3/Slot 4 Amount": 0.0,
        "Mod/Slot 1 Source": 0.0, "Mod/Slot 1 Target": 0.0, "Mod/Slot 1 Amount": 0.0,
        "Mod/Slot 2 Source": 0.0, "Mod/Slot 2 Target": 0.0, "Mod/Slot 2 Amount": 0.0,
        "Mod/Slot 3 Source": 0.0, "Mod/Slot 3 Target": 0.0, "Mod/Slot 3 Amount": 0.0,
        "Mod/Slot 4 Source": 0.0, "Mod/Slot 4 Target": 0.0, "Mod/Slot 4 Amount": 0.0,
        "Mod/Slot 5 Source": 0.0, "Mod/Slot 5 Target": 0.0, "Mod/Slot 5 Amount": 0.0,
        "Mod/Slot 6 Source": 0.0, "Mod/Slot 6 Target": 0.0, "Mod/Slot 6 Amount": 0.0,
        "Mod/Slot 7 Source": 0.0, "Mod/Slot 7 Target": 0.0, "Mod/Slot 7 Amount": 0.0,
        "Mod/Slot 8 Source": 0.0, "Mod/Slot 8 Target": 0.0, "Mod/Slot 8 Amount": 0.0,
        "Control/Mode": 0.0, "Control/PB Range": 2.0, "Control/Tempo": 120.0, "Control/Tempo Sync": 1.0,
        "FX/Mod Type": 1.0, "FX/Mod Rate": 1.0, "FX/Mod Depth": 0.5, "FX/Mod Mix": 0.0,
        "FX/Dly Time": 400.0, "FX/Dly FB": 0.3, "FX/Dly Mix": 0.0, "FX/Dly Sync": 0.0,
        "FX/Rvb Size": 0.5, "FX/Rvb Damp": 0.5, "FX/Rvb Width": 0.5, "FX/Rvb Mix": 0.0,
        "Amp Output/Level": 0.8, "Amp Output/Pan": 0.0, "Amp Output/Spread": 0.0, "Amp Output/Velocity": 0.5,
        "Arp/Arp On": 0.0, "Arp/Mode": 0.0, "Arp/Octave": 1.0, "Arp/Rate Note": 2.0,
        "Arp/Gate": 80.0, "Arp/Latch": 0.0,
    }

# 16 patches (slots 2-17)
patches = [
    ("Warm Pad", {
        "Oscillator 1/Waveform": 15.0, "Oscillator 1/Unison": 4.0, "Oscillator 1/USpread": 0.15,
        "Oscillator 2/Waveform": 20.0, "Oscillator 2/Volume": 0.6, "Oscillator 2/Transp": -12.0, "Oscillator 2/Detune": 0.05,
        "Ladder Filter/Cutoff": 2500.0, "Filter Env/Amount": 3500.0, "Filter Env/Attack": 800.0, "Filter Env/Decay": 1500.0, "Filter Env/Sustain": 0.4,
        "Amp Env/Attack": 800.0, "Amp Env/Release": 1200.0, "FX/Rvb Mix": 0.35, "FX/Rvb Size": 0.8,
    }),
    ("Bright Lead", {
        "Oscillator 1/Waveform": 35.0, "Oscillator 1/Unison": 2.0, "Oscillator 1/USpread": 0.08,
        "Ladder Filter/Cutoff": 8000.0, "Ladder Filter/Res": 0.3, "Filter Env/Amount": 4000.0, "Filter Env/Attack": 10.0, "Filter Env/Decay": 800.0, "Filter Env/Sustain": 0.5,
        "Amp Env/Attack": 5.0, "Amp Env/Decay": 300.0, "Amp Env/Sustain": 0.8, "Amp Env/Release": 400.0,
        "FX/Mod Mix": 0.2,
    }),
    ("Sub Bass", {
        "Oscillator 1/Waveform": 195.0, "Oscillator 1/Transp": -12.0,
        "Oscillator 2/Waveform": 195.0, "Oscillator 2/Volume": 0.5, "Oscillator 2/Transp": -24.0,
        "Ladder Filter/Cutoff": 800.0, "Ladder Filter/Res": 0.2, "Filter Env/Amount": 600.0, "Filter Env/Attack": 5.0, "Filter Env/Decay": 200.0,
        "Amp Env/Attack": 5.0, "Amp Env/Decay": 400.0, "Amp Env/Sustain": 0.9, "Amp Env/Release": 150.0,
        "Control/Mode": 1.0,
    }),
    ("Pluck", {
        "Oscillator 1/Waveform": 90.0, "Oscillator 1/Unison": 3.0, "Oscillator 1/USpread": 0.1,
        "Ladder Filter/Cutoff": 3500.0, "Ladder Filter/Res": 0.4, "Filter Env/Amount": 5000.0, "Filter Env/Attack": 0.0, "Filter Env/Decay": 350.0, "Filter Env/Sustain": 0.0,
        "Amp Env/Attack": 0.0, "Amp Env/Decay": 400.0, "Amp Env/Sustain": 0.0, "Amp Env/Release": 200.0,
        "FX/Rvb Mix": 0.15,
    }),
    ("Arp Pulse", {
        "Oscillator 1/Waveform": 115.0, "Oscillator 1/Symmetry": 0.25,
        "Ladder Filter/Cutoff": 4000.0, "Ladder Filter/Res": 0.45, "Filter Env/Amount": 3000.0, "Filter Env/Attack": 5.0, "Filter Env/Decay": 250.0, "Filter Env/Sustain": 0.2,
        "Amp Env/Attack": 5.0, "Amp Env/Decay": 200.0, "Amp Env/Sustain": 0.5, "Amp Env/Release": 100.0,
        "Arp/Arp On": 1.0, "Arp/Rate Note": 3.0, "FX/Dly Mix": 0.25, "FX/Dly Time": 250.0, "FX/Dly Sync": 1.0,
    }),
    ("Synth Brass", {
        "Oscillator 1/Waveform": 30.0, "Oscillator 1/Unison": 3.0, "Oscillator 1/USpread": 0.12,
        "Oscillator 2/Waveform": 195.0, "Oscillator 2/Volume": 0.7, "Oscillator 2/Detune": 0.02,
        "Ladder Filter/Cutoff": 3000.0, "Ladder Filter/Res": 0.25, "Filter Env/Amount": 2500.0, "Filter Env/Attack": 100.0, "Filter Env/Decay": 600.0, "Filter Env/Sustain": 0.6,
        "Amp Env/Attack": 80.0, "Amp Env/Decay": 200.0, "Amp Env/Sustain": 0.85, "Amp Env/Release": 300.0,
    }),
    ("Shimmer", {
        "Oscillator 1/Waveform": 150.0, "Oscillator 1/Unison": 5.0, "Oscillator 1/USpread": 0.18,
        "Oscillator 2/Waveform": 65.0, "Oscillator 2/Volume": 0.5, "Oscillator 2/Transp": 12.0,
        "Ladder Filter/Cutoff": 12000.0, "Ladder Filter/Res": 0.15,
        "Amp Env/Attack": 600.0, "Amp Env/Decay": 1000.0, "Amp Env/Sustain": 0.7, "Amp Env/Release": 1500.0,
        "LFO 1/Rate Hz": 0.3, "LFO 1/Slot 1 Target": 8.0, "LFO 1/Slot 1 Amount": 15.0,
        "FX/Rvb Mix": 0.45, "FX/Rvb Size": 0.9, "FX/Mod Mix": 0.15,
    }),
    ("Square Lead", {
        "Oscillator 1/Waveform": 200.0, "Oscillator 1/Unison": 2.0, "Oscillator 1/USpread": 0.06,
        "Ladder Filter/Cutoff": 6000.0, "Ladder Filter/Res": 0.35, "Filter Env/Amount": 3500.0, "Filter Env/Attack": 10.0, "Filter Env/Decay": 500.0, "Filter Env/Sustain": 0.4,
        "Amp Env/Attack": 5.0, "Amp Env/Decay": 150.0, "Amp Env/Sustain": 0.9, "Amp Env/Release": 200.0,
        "Control/Mode": 1.0, "Control/PB Range": 12.0,
    }),
    ("Deep Vox", {
        "Oscillator 1/Waveform": 230.0, "Oscillator 1/Unison": 4.0, "Oscillator 1/USpread": 0.14,
        "Oscillator 2/Waveform": 235.0, "Oscillator 2/Volume": 0.6, "Oscillator 2/Transp": -12.0, "Oscillator 2/Detune": 0.08,
        "Ladder Filter/Cutoff": 3500.0, "Ladder Filter/Res": 0.3,
        "Amp Env/Attack": 400.0, "Amp Env/Decay": 600.0, "Amp Env/Sustain": 0.8, "Amp Env/Release": 800.0,
        "LFO 1/Rate Hz": 0.2, "LFO 1/Slot 1 Target": 11.0, "LFO 1/Slot 1 Amount": 500.0,
        "FX/Rvb Mix": 0.3,
    }),
    ("Sync Sweep", {
        "Oscillator 1/Waveform": 210.0, "Oscillator 1/Unison": 2.0, "Oscillator 1/USpread": 0.05,
        "Ladder Filter/Cutoff": 8000.0, "Ladder Filter/Res": 0.4, "Filter Env/Amount": 5000.0, "Filter Env/Attack": 20.0, "Filter Env/Decay": 700.0, "Filter Env/Sustain": 0.3,
        "Amp Env/Attack": 5.0, "Amp Env/Decay": 200.0, "Amp Env/Sustain": 0.85, "Amp Env/Release": 250.0,
        "Control/Mode": 1.0,
    }),
    ("Ambient Drift", {
        "Oscillator 1/Waveform": 50.0, "Oscillator 1/Unison": 6.0, "Oscillator 1/USpread": 0.22,
        "Oscillator 2/Waveform": 100.0, "Oscillator 2/Volume": 0.55, "Oscillator 2/Transp": 7.0, "Oscillator 2/Detune": 0.1,
        "Ladder Filter/Cutoff": 6000.0, "Ladder Filter/Res": 0.2,
        "Amp Env/Attack": 1200.0, "Amp Env/Decay": 2000.0, "Amp Env/Sustain": 0.65, "Amp Env/Release": 2500.0,
        "LFO 1/Rate Hz": 0.15, "LFO 1/Slot 1 Target": 11.0, "LFO 1/Slot 1 Amount": 1200.0,
        "LFO 2/Rate Hz": 0.25, "LFO 2/Slot 1 Target": 17.0, "LFO 2/Slot 1 Amount": 0.15,
        "FX/Rvb Mix": 0.5, "FX/Rvb Size": 0.95, "FX/Mod Type": 2.0, "FX/Mod Mix": 0.2,
    }),
    ("Stab", {
        "Oscillator 1/Waveform": 195.0, "Oscillator 1/Unison": 5.0, "Oscillator 1/USpread": 0.2,
        "Oscillator 2/Waveform": 200.0, "Oscillator 2/Volume": 0.7, "Oscillator 2/Detune": 0.03,
        "Ladder Filter/Cutoff": 5000.0, "Ladder Filter/Res": 0.5, "Filter Env/Amount": 6000.0, "Filter Env/Attack": 0.0, "Filter Env/Decay": 180.0, "Filter Env/Sustain": 0.0,
        "Amp Env/Attack": 0.0, "Amp Env/Decay": 250.0, "Amp Env/Sustain": 0.0, "Amp Env/Release": 100.0,
        "FX/Rvb Mix": 0.2,
    }),
    ("Bell", {
        "Oscillator 1/Waveform": 70.0, "Oscillator 1/Unison": 3.0, "Oscillator 1/USpread": 0.08,
        "Oscillator 2/Waveform": 75.0, "Oscillator 2/Volume": 0.4, "Oscillator 2/Transp": 19.0,
        "Ladder Filter/Cutoff": 10000.0, "Ladder Filter/Res": 0.2, "Filter Env/Amount": -2000.0, "Filter Env/Attack": 0.0, "Filter Env/Decay": 2000.0,
        "Amp Env/Attack": 0.0, "Amp Env/Decay": 2500.0, "Amp Env/Sustain": 0.3, "Amp Env/Release": 1500.0,
        "FX/Rvb Mix": 0.4, "FX/Rvb Size": 0.7,
    }),
    ("Wobble Bass", {
        "Oscillator 1/Waveform": 195.0, "Oscillator 1/Unison": 2.0, "Oscillator 1/USpread": 0.05,
        "Oscillator 2/Waveform": 200.0, "Oscillator 2/Volume": 0.8, "Oscillator 2/Transp": -12.0,
        "Ladder Filter/Cutoff": 1200.0, "Ladder Filter/Res": 0.7, "Filter Env/Amount": 2000.0, "Filter Env/Decay": 200.0,
        "Amp Env/Attack": 5.0, "Amp Env/Sustain": 0.9, "Amp Env/Release": 100.0,
        "LFO 1/Rate Hz": 4.0, "LFO 1/Slot 1 Target": 11.0, "LFO 1/Slot 1 Amount": 3000.0,
        "Control/Mode": 1.0,
    }),
    ("Strings", {
        "Oscillator 1/Waveform": 195.0, "Oscillator 1/Unison": 6.0, "Oscillator 1/USpread": 0.16,
        "Oscillator 2/Waveform": 195.0, "Oscillator 2/Volume": 0.6, "Oscillator 2/Transp": -12.0, "Oscillator 2/Detune": 0.04, "Oscillator 2/Unison": 4.0,
        "Ladder Filter/Cutoff": 4500.0, "Ladder Filter/Res": 0.1, "Filter Env/Amount": 1800.0, "Filter Env/Attack": 500.0, "Filter Env/Decay": 800.0, "Filter Env/Sustain": 0.6,
        "Amp Env/Attack": 500.0, "Amp Env/Decay": 600.0, "Amp Env/Sustain": 0.8, "Amp Env/Release": 800.0,
        "FX/Rvb Mix": 0.35, "FX/Rvb Size": 0.8,
    }),
    ("Epic Pad", {
        "Oscillator 1/Waveform": 120.0, "Oscillator 1/Unison": 7.0, "Oscillator 1/USpread": 0.25,
        "Oscillator 2/Waveform": 15.0, "Oscillator 2/Volume": 0.7, "Oscillator 2/Transp": -5.0, "Oscillator 2/Unison": 5.0, "Oscillator 2/Detune": 0.06,
        "Ladder Filter/Cutoff": 5500.0, "Ladder Filter/Res": 0.15, "Filter Env/Amount": 3000.0, "Filter Env/Attack": 1000.0, "Filter Env/Decay": 1500.0, "Filter Env/Sustain": 0.5,
        "Amp Env/Attack": 1000.0, "Amp Env/Decay": 1200.0, "Amp Env/Sustain": 0.75, "Amp Env/Release": 2000.0,
        "LFO 1/Rate Hz": 0.1, "LFO 1/Slot 1 Target": 11.0, "LFO 1/Slot 1 Amount": 800.0,
        "FX/Rvb Mix": 0.5, "FX/Rvb Size": 0.92, "FX/Rvb Width": 1.0, "FX/Mod Mix": 0.12, "FX/Mod Rate": 0.2,
    }),
]

def main():
    docs = Path.home() / "Documents" / "NeonSynth" / "NeonJr" / "Banks" / "Factory"
    docs.mkdir(parents=True, exist_ok=True)
    
    # Read existing index
    index_file = docs / "index.txt"
    lines = index_file.read_text().splitlines() if index_file.exists() else ["INIT PATCH"] * 128
    
    # Create patches 2-17
    for i, (name, params) in enumerate(patches, start=2):
        p = base()
        p.update(params)
        data = {"name": name, "parameters": p}
        (docs / f"patch_{i}.neon").write_text(json.dumps(data, indent=2))
        lines[i-1] = name
        print(f"{i:3d}. {name}")
    
    # Update index
    index_file.write_text("\n".join(lines))
    print(f"\n✅ Created 16 patches (slots 2-17) preserving Jr. Arena in slot 1")

if __name__ == "__main__":
    main()
