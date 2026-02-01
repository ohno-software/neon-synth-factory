import json
import os
import random

# Specific waveform indices based on file listing
WAVE_SINE = 134
WAVE_SAW = 124
WAVE_SQUARE = 151
WAVE_TRI = 166

def clamp(v, lo, hi):
    return max(lo, min(hi, v))

def get_base_params():
    return {
        "Oscillator 1/Volume": 0.8,
        "Oscillator 1/Waveform": WAVE_SAW,
        "Oscillator 1/Unison": 1.0,
        "Oscillator 1/USpread": 0.2,
        "Oscillator 1/Symmetry": 0.5,
        "Oscillator 2/Volume": 0.0,
        "Oscillator 2/Waveform": WAVE_SAW,
        "Ladder Filter/Cutoff": 20000.0,
        "Ladder Filter/Res": 0.0,
        "Ladder Filter/Type": 0.0, # LP
        "Ladder Filter/Slope": 1.0, # 24dB
        "Ladder Filter/Drive": 1.0,
        "Ladder Filter/KeyTrack": 0.5,
        "Amp Env/Attack": 10.0,
        "Amp Env/Decay": 500.0,
        "Amp Env/Sustain": 1.0,
        "Amp Env/Release": 500.0,
        "Filter Env/Attack": 10.0,
        "Filter Env/Decay": 500.0,
        "Filter Env/Sustain": 1.0,
        "Filter Env/Release": 500.0,
        "Filter Env/Amount": 0.0,
        "Control/Mode": 0.0, # Poly
        "FX/Mod Mix": 0.0,
        "FX/Dly Mix": 0.0,
        "FX/Rvb Mix": 0.0,
    }

def create_patch(name, category):
    p = get_base_params()
    
    if category == "BASS":
        # Basic Bass Recipe: Single Saw, low cutoff, high resonance pluck
        p.update({
            "Control/Mode": 1.0, # Mono
            "Oscillator 1/Waveform": WAVE_SAW,
            "Ladder Filter/Cutoff": 150.0 + random.random() * 300,
            "Ladder Filter/Res": 0.3 + random.random() * 0.4,
            "Filter Env/Amount": 4.0 + random.random() * 4.0,
            "Filter Env/Attack": 0.0,
            "Filter Env/Decay": 100.0 + random.random() * 200,
            "Filter Env/Sustain": 0.1,
            "Filter Env/Release": 200.0,
            "Amp Env/Attack": 0.0,
            "Amp Env/Release": 100.0,
            "Ladder Filter/Drive": 1.1 + random.random() * 0.5
        })
        if "Sub" in name:
            p["Oscillator 1/Waveform"] = WAVE_SINE
            p["Ladder Filter/Res"] = 0.0
            p["Ladder Filter/Cutoff"] = 100.0
        elif "Pulse" in name or "Acid" in name:
            p["Oscillator 1/Waveform"] = WAVE_SQUARE
            p["Oscillator 1/Symmetry"] = 0.2 + random.random() * 0.2
            
    elif category == "LEAD":
        # Lead Recipe: Unison Saw, vibrato, delay
        p.update({
            "Control/Mode": 1.0, # Mono
            "Oscillator 1/Unison": 3.0 + random.random() * 4.0,
            "Oscillator 1/USpread": 0.1 + random.random() * 0.2,
            "Ladder Filter/Cutoff": 2000.0 + random.random() * 5000,
            "FX/Dly Mix": 0.2,
            "FX/Rvb Mix": 0.1,
            "LFO 1/Slot 1 Target": 1.0, # Osc 1 Pitch
            "LFO 1/Slot 1 Amount": 0.1, # Suble vibrato
            "LFO 1/Rate Hz": 1.0 + random.random() * 4.0,
        })
        if "Sync" in name:
            p["Oscillator 2/Volume"] = 0.8
            p["Oscillator 2/Transp"] = 12.0 + random.random() * 12.0
            p["Oscillator 2/Detune"] = random.random() * 50.0
            
    elif category == "PAD":
        # Strings/Pad Recipe: Dual Saws, detuned, slow attack/release
        p.update({
            "Oscillator 1/Unison": 6.0,
            "Oscillator 1/USpread": 0.4,
            "Oscillator 2/Volume": 0.5,
            "Oscillator 2/Detune": 15.0,
            "Amp Env/Attack": 1000.0 + random.random() * 1000,
            "Amp Env/Release": 1500.0 + random.random() * 1000,
            "Ladder Filter/Cutoff": 800.0 + random.random() * 1000,
            "FX/Rvb Mix": 0.4,
            "FX/Mod Type": 1.0, # Chorus
            "FX/Mod Mix": 0.3
        })
        
    elif category == "BRASS":
        # Brass Recipe: Saw mix, filter swell (Mid attack)
        p.update({
            "Oscillator 1/Waveform": WAVE_SAW,
            "Oscillator 2/Volume": 0.6,
            "Oscillator 2/Detune": -8.0,
            "Ladder Filter/Cutoff": 400.0,
            "Filter Env/Amount": 5.0,
            "Filter Env/Attack": 100.0 + random.random() * 100,
            "Filter Env/Decay": 500.0,
            "Filter Env/Sustain": 0.5,
            "Amp Env/Attack": 50.0,
            "Ladder Filter/KeyTrack": 0.8
        })
        
    elif category == "FLUTE":
        # Flute Recipe: Square/Pulse, high keytrack, subtle breath env
        p.update({
            "Oscillator 1/Waveform": WAVE_SQUARE,
            "Oscillator 1/Symmetry": 0.5,
            "Ladder Filter/Cutoff": 500.0,
            "Ladder Filter/KeyTrack": 1.0,
            "Filter Env/Amount": 2.0,
            "Filter Env/Attack": 10.0,
            "Filter Env/Decay": 200.0,
            "Filter Env/Sustain": 0.0,
            "Amp Env/Attack": 50.0,
            "LFO 1/Slot 1 Target": 1.0, # Vibrato
            "LFO 1/Slot 1 Amount": 0.08,
            "FX/Rvb Mix": 0.2
        })

    elif category == "PLUCK":
        # Pluck Recipe: Zero sustain, fast filter decay
        p.update({
            "Amp Env/Sustain": 0.0,
            "Amp Env/Decay": 300.0 + random.random() * 400,
            "Filter Env/Amount": 6.0,
            "Filter Env/Decay": 50.0 + random.random() * 150,
            "Filter Env/Sustain": 0.0,
            "Ladder Filter/Cutoff": 200.0,
            "Ladder Filter/Res": 0.5
        })

    return {"name": name, "parameters": p}

patch_names = [
    # BASS (20)
    "Classic Saw Bass", "Deep Sub", "Pulse Grit", "Acid House", "Moogish Sub",
    "Slap Digital", "FM Style Bass", "Growl Drive", "Snappy Short", "Wobble Bass",
    "Solid Bass", "Power Bass", "LoFi Punch", "Sub Organ", "Sync Bass",
    "Soft Sine Bass", "Phase Bass", "Biting Mid", "Vibe Sub", "Earthshaker",
    # LEADS (25)
    "Neon Solo", "Super Saw Solo", "Sync Scream", "Soft Flute Lead", "Whistle Lead",
    "Pulse Lead", "Square Lead", "Dreamy Solo", "Talky Lead", "Gritty Sync",
    "Unison Power", "Detuned Solo", "Harmonic Lead", "Pure Sine", "Nasty Drive",
    "Smooth Voyager", "Octave Jump", "Hyper Solo", "Gliding Lead", "Vibrato Solo",
    "Plastic Lead", "Metal Lead", "Stutter Lead", "Wide Solo", "Crystal Lead",
    # PADS (25)
    "Cloud Swell", "Vintage Strings", "Deep Space", "Glassy Pad", "Ethereal Breath",
    "Analog Warmth", "Phase Clouds", "Bright Horizon", "Dark Star", "Reso Pad",
    "Soft Organ", "Wide Unison", "Slow Motion", "Cinematic Swell", "Cyber Pad",
    "Hollow Pad", "Bright Shimmer", "Frozen Air", "Detuned Swell", "Modulated Air",
    "Harmonic Pad", "Vocal Pad", "Static Pad", "Nebula", "Glacier Pad",
    # PLUCKS (20)
    "Plastic Pluck", "Short Decay", "Filter Snap", "Glass Pluck", "Soft Mallet",
    "Analog Pluck", "Chiptune Snap", "Bouncy Pluck", "Bright Perc", "Clean Pluck",
    "Metal Perc", "Wood Pluck", "Fast Snap", "Sync Pluck", "Reso Pluck",
    "Delayed Pluck", "Sub Pluck", "Pulse Snap", "Hollow Pluck", "Digital Perc",
    # BRASS/KEYS (25)
    "80s Brass", "Synth Piano", "Soft EP", "Bright Brass", "Analog Keys",
    "Bell Keys", "Tine Piano", "Church Organ", "Rotary Organ", "Brite Organ",
    "Poly Synth", "Detuned Keys", "Soft Brass", "Hollow Keys", "Pluck Keys",
    "Vintage EP", "Dream Keys", "Crystal Keys", "Phase Keys", "Wide Brass",
    "Stab Brass", "Velvet Keys", "Electric Tine", "Mellow Brass", "Classic Poly",
    # FX/ARP (13)
    "Windy Noise", "Riser 1", "Downer 1", "Cyber Motion", "Random Steps",
    "Alien Chatter", "Arp Machine", "Rhythmic Mod", "Octave Jumper", "Bouncing Ball", 
    "Clockwork", "Digital Rain", "Final Wave"
]

docs_path = os.path.expanduser("~/Documents/NeonSynth/NeonJr/Banks/Factory")
os.makedirs(docs_path, exist_ok=True)

for i, name in enumerate(patch_names):
    cat = "PAD"
    if i < 20: cat = "BASS"
    elif i < 45: cat = "LEAD"
    elif i < 70: cat = "PAD"
    elif i < 90: cat = "PLUCK"
    elif i < 115: # BRASS/KEYS
        if "Brass" in name: cat = "BRASS"
        elif "Flute" in name: cat = "FLUTE"
        else: cat = "PAD" 
        if "Keys" in name or "Piano" in name or "EP" in name:
            cat = "PLUCK"
    else: cat = "PAD"
    
    patch_data = create_patch(name, cat)
    
    # Final manual overrides for specific names
    if "Arp" in name:
        patch_data["parameters"]["Arp/Arp On"] = 1.0
        patch_data["parameters"]["Arp/Rate Note"] = 2.0 # 1/16
        
    filename = f"patch_{i+1}.neon"
    with open(os.path.join(docs_path, filename), "w") as f:
        json.dump(patch_data, f, indent=4)

with open(os.path.join(docs_path, "index.txt"), "w") as f:
    f.write("\n".join(patch_names))

print(f"Generated 128 recipe-based patches in {docs_path}")
