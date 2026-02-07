#include "PadPresets.h"

using WaveformType = VoiceBase::WaveformType;

//==============================================================================
const std::array<PadPresetData, 16> PadPresets::presets = {{
    // Solina-inspired string ensembles
    {
        "Solina Strings",
        "Analog Saw",
        WaveformType::AnalogSaw,
        0.7f,   // level
        0.4f,   // attack
        1.2f,   // release
        0.5f,   // pulse width
        4,      // num oscillators
        {-15.0f, -7.0f, 7.0f, 15.0f},  // detune (cents)
        {1.0f, 1.0f, 1.0f, 1.0f},      // osc levels
        0.0f,   // mod index
        0.0f    // mod ratio
    },
    {
        "Solina Warm",
        "Analog Saw",
        WaveformType::AnalogSaw,
        0.65f,
        0.5f,
        1.5f,
        0.5f,
        4,
        {-20.0f, -10.0f, 10.0f, 20.0f},
        {0.9f, 1.0f, 1.0f, 0.9f},
        0.0f,
        0.0f
    },
    
    // Juno-inspired pads
    {
        "Juno Lush",
        "Analog Saw",
        WaveformType::AnalogSaw,
        0.75f,
        0.35f,
        1.0f,
        0.5f,
        4,
        {-12.0f, -5.0f, 5.0f, 12.0f},
        {1.0f, 1.0f, 1.0f, 1.0f},
        0.0f,
        0.0f
    },
    {
        "Juno Pulse",
        "Analog Pulse",
        WaveformType::AnalogPulse,
        0.7f,
        0.3f,
        0.9f,
        0.4f,
        4,
        {-10.0f, -3.0f, 3.0f, 10.0f},
        {1.0f, 0.8f, 0.8f, 1.0f},
        0.0f,
        0.0f
    },
    {
        "Juno PWM",
        "Analog Pulse",
        WaveformType::AnalogPulse,
        0.7f,
        0.4f,
        1.1f,
        0.3f,   // narrow pulse
        4,
        {-8.0f, -2.0f, 2.0f, 8.0f},
        {1.0f, 1.0f, 1.0f, 1.0f},
        0.0f,
        0.0f
    },
    {
        "Juno Brass",
        "Analog Saw",
        WaveformType::AnalogSaw,
        0.8f,
        0.15f,
        0.7f,
        0.5f,
        4,
        {-7.0f, -3.0f, 3.0f, 7.0f},
        {1.0f, 0.9f, 0.9f, 1.0f},
        0.0f,
        0.0f
    },
    
    // FM Pads
    {
        "DX Shimmer",
        "FM",
        WaveformType::FM,
        0.65f,
        0.5f,
        1.5f,
        0.5f,
        2,
        {-5.0f, 5.0f, 0.0f, 0.0f},
        {1.0f, 1.0f, 0.0f, 0.0f},
        2.0f,
        2.0f
    },
    {
        "FM Bell Pad",
        "FM",
        WaveformType::FM,
        0.6f,
        0.3f,
        2.0f,
        0.5f,
        2,
        {-7.0f, 7.0f, 0.0f, 0.0f},
        {1.0f, 1.0f, 0.0f, 0.0f},
        3.5f,
        3.0f
    },
    {
        "FM Glass",
        "FM",
        WaveformType::FM,
        0.55f,
        0.25f,
        1.8f,
        0.5f,
        2,
        {-3.0f, 3.0f, 0.0f, 0.0f},
        {1.0f, 1.0f, 0.0f, 0.0f},
        4.0f,
        4.0f
    },
    {
        "FM Ethereal",
        "FM",
        WaveformType::FM,
        0.6f,
        0.6f,
        2.5f,
        0.5f,
        2,
        {-10.0f, 10.0f, 0.0f, 0.0f},
        {1.0f, 1.0f, 0.0f, 0.0f},
        1.5f,
        1.5f
    },
    
    // Fairlight-inspired textures
    {
        "Fairlight Choir",
        "Analog Sine",
        WaveformType::AnalogSine,
        0.7f,
        0.6f,
        2.0f,
        0.5f,
        4,
        {-25.0f, -8.0f, 8.0f, 25.0f},
        {0.8f, 1.0f, 1.0f, 0.8f},
        0.0f,
        0.0f
    },
    {
        "Fairlight Vox",
        "Analog Pulse",
        WaveformType::AnalogPulse,
        0.65f,
        0.5f,
        1.5f,
        0.45f,
        4,
        {-18.0f, -6.0f, 6.0f, 18.0f},
        {0.9f, 1.0f, 1.0f, 0.9f},
        0.0f,
        0.0f
    },
    {
        "Fairlight Dreams",
        "Analog Saw",
        WaveformType::AnalogSaw,
        0.6f,
        0.7f,
        3.0f,
        0.5f,
        4,
        {-30.0f, -12.0f, 12.0f, 30.0f},
        {0.7f, 1.0f, 1.0f, 0.7f},
        0.0f,
        0.0f
    },
    
    // Classic synthwave
    {
        "Neon Nights",
        "Analog Saw",
        WaveformType::AnalogSaw,
        0.75f,
        0.25f,
        0.8f,
        0.5f,
        4,
        {-10.0f, -4.0f, 4.0f, 10.0f},
        {1.0f, 1.0f, 1.0f, 1.0f},
        0.0f,
        0.0f
    },
    {
        "Retro Wave",
        "Analog Pulse",
        WaveformType::AnalogPulse,
        0.7f,
        0.3f,
        1.0f,
        0.5f,
        4,
        {-12.0f, -5.0f, 5.0f, 12.0f},
        {1.0f, 0.9f, 0.9f, 1.0f},
        0.0f,
        0.0f
    },
    {
        "Sunset Drive",
        "Analog Sine",
        WaveformType::AnalogSine,
        0.65f,
        0.4f,
        1.3f,
        0.5f,
        4,
        {-8.0f, -3.0f, 3.0f, 8.0f},
        {1.0f, 1.0f, 1.0f, 1.0f},
        0.0f,
        0.0f
    }
}};

//==============================================================================
const PadPresetData& PadPresets::getPreset(int index)
{
    int clampedIndex = juce::jlimit(0, static_cast<int>(presets.size()) - 1, index);
    return presets[static_cast<size_t>(clampedIndex)];
}

int PadPresets::getNumPresets()
{
    return static_cast<int>(presets.size());
}

juce::StringArray PadPresets::getPresetNames()
{
    juce::StringArray names;
    for (const auto& preset : presets)
    {
        names.add(preset.name);
    }
    return names;
}
