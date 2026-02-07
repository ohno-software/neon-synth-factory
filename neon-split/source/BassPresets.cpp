#include "BassPresets.h"

using WaveformType = VoiceBase::WaveformType;

//==============================================================================
// Preset format:
// { name, waveform, level, attack, release, pulseWidth,
//   osc2Detune, osc2Level, subOscLevel,
//   filterCutoff, filterResonance, filterEnvAmount,
//   modIndex, modRatio }
//
// MONOPHONIC bass - short releases for tight playing

const std::array<BassPresetData, 16> BassPresets::presets = {{
    // DEFAULT: Classic synthwave bass - 3 osc detuned saw with filtered attack
    {
        "Synthwave Bass",
        WaveformType::AnalogSaw,
        0.8f,   // level
        0.002f, // attack - quick snap
        0.1f,   // release - SHORT
        0.5f,   // pulse width (unused)
        12.0f,  // osc2 detune cents (slightly sharp)
        0.7f,   // osc2 level
        0.4f,   // sub osc level
        0.5f,   // filter cutoff (50%)
        0.25f,  // filter resonance (slight)
        0.3f,   // filter env amount (opens on attack)
        0.0f,   // mod index (unused)
        0.0f    // mod ratio (unused)
    },
    
    // BIG SAW - longer release for pad-like bass
    {
        "Big Saw",
        WaveformType::AnalogSaw,
        0.85f,
        0.005f,  // slower attack
        0.8f,   // LONG release - pad-like
        0.5f,
        15.0f,  // wider detune
        0.8f,   // strong osc2
        0.5f,   // big sub
        0.55f,  // warmer
        0.15f,  // gentle resonance
        0.2f,   // subtle env
        0.0f,
        0.0f
    },
    
    // Moog-inspired
    {
        "Moog Thunder",
        WaveformType::AnalogSaw,
        0.85f,
        0.001f,
        0.12f,  // short
        0.5f,
        8.0f,
        0.6f,
        0.35f,
        0.55f,
        0.2f,
        0.25f,
        0.0f,
        0.0f
    },
    {
        "Moog Punch",
        WaveformType::AnalogPulse,
        0.8f,
        0.0005f,
        0.08f,  // very short - punchy
        0.4f,
        6.0f,
        0.5f,
        0.25f,
        0.45f,
        0.35f,
        0.4f,
        0.0f,
        0.0f
    },
    {
        "Moog Sub",
        WaveformType::AnalogSaw,
        0.85f,
        0.002f,
        0.15f,  // short
        0.5f,
        10.0f,
        0.5f,
        0.6f,
        0.4f,
        0.15f,
        0.2f,
        0.0f,
        0.0f
    },
    
    // SH-101 inspired
    {
        "101 Acid",
        WaveformType::AnalogSaw,
        0.8f,
        0.0001f,
        0.08f,  // very short - staccato acid
        0.5f,
        0.0f,
        0.0f,
        0.0f,
        0.35f,
        0.6f,
        0.5f,
        0.0f,
        0.0f
    },
    {
        "101 Squelch",
        WaveformType::AnalogPulse,
        0.75f,
        0.0005f,
        0.1f,   // short
        0.25f,
        0.0f,
        0.0f,
        0.0f,
        0.3f,
        0.7f,
        0.55f,
        0.0f,
        0.0f
    },
    {
        "101 Rubber",
        WaveformType::AnalogSaw,
        0.7f,
        0.005f,
        0.18f,  // slightly longer for rubber feel
        0.5f,
        15.0f,
        0.65f,
        0.4f,
        0.5f,
        0.2f,
        0.15f,
        0.0f,
        0.0f
    },
    
    // BassStation inspired
    {
        "Station Growl",
        WaveformType::AnalogSaw,
        0.85f,
        0.0001f,
        0.1f,   // short
        0.5f,
        7.0f,
        0.8f,
        0.3f,
        0.55f,
        0.3f,
        0.35f,
        0.0f,
        0.0f
    },
    {
        "Station Tight",
        WaveformType::AnalogPulse,
        0.75f,
        0.0001f,
        0.05f,  // super short - staccato
        0.5f,
        5.0f,
        0.4f,
        0.0f,
        0.6f,
        0.4f,
        0.5f,
        0.0f,
        0.0f
    },
    
    // ARP 2600 inspired
    {
        "ARP Fat",
        WaveformType::AnalogSaw,
        0.85f,
        0.002f,
        0.15f,  // short
        0.5f,
        -10.0f,
        0.75f,
        0.5f,
        0.5f,
        0.2f,
        0.25f,
        0.0f,
        0.0f
    },
    {
        "ARP Buzz",
        WaveformType::AnalogPulse,
        0.8f,
        0.0005f,
        0.1f,   // short
        0.3f,
        8.0f,
        0.6f,
        0.25f,
        0.65f,
        0.15f,
        0.2f,
        0.0f,
        0.0f
    },
    {
        "ARP Deep",
        WaveformType::AnalogSaw,
        0.8f,
        0.004f,
        0.12f,  // short
        0.5f,
        6.0f,
        0.5f,
        0.65f,
        0.35f,
        0.1f,
        0.1f,
        0.0f,
        0.0f
    },
    
    // FM Bass
    {
        "FM Slap",
        WaveformType::FM,
        0.75f,
        0.0001f,
        0.06f,  // very short - slappy
        0.5f,
        0.0f,
        0.0f,
        0.0f,
        0.7f,
        0.1f,
        0.0f,
        4.0f,
        2.0f
    },
    {
        "FM Electric",
        WaveformType::FM,
        0.8f,
        0.0005f,
        0.1f,   // short
        0.5f,
        0.0f,
        0.0f,
        0.2f,
        0.6f,
        0.15f,
        0.0f,
        3.0f,
        3.0f
    },
    {
        "FM Metallic",
        WaveformType::FM,
        0.7f,
        0.0005f,
        0.08f,  // short
        0.5f,
        0.0f,
        0.0f,
        0.0f,
        0.75f,
        0.2f,
        0.0f,
        5.0f,
        4.0f
    }
}};

//==============================================================================
const BassPresetData& BassPresets::getPreset(int index)
{
    int clampedIndex = juce::jlimit(0, static_cast<int>(presets.size()) - 1, index);
    return presets[static_cast<size_t>(clampedIndex)];
}

int BassPresets::getNumPresets()
{
    return static_cast<int>(presets.size());
}

juce::StringArray BassPresets::getPresetNames()
{
    juce::StringArray names;
    for (const auto& preset : presets)
    {
        names.add(preset.name);
    }
    return names;
}

