#pragma once

#include <JuceHeader.h>
#include "VoiceBase.h"

/**
 * Pad Preset Data Structure
 * Inspired by: Solina ensemble, Juno pads, FM pads, Fairlight textures
 * Waveform/engine tags: FM, Analog Pulse, Analog Saw, Analog Sine
 */
struct PadPresetData
{
    juce::String name;
    juce::String engineTag; // "FM", "Analog Pulse", "Analog Saw", "Analog Sine"
    VoiceBase::WaveformType waveform;
    
    // Synthesis parameters (not user-adjustable)
    float level = 0.7f;
    float attackTime = 0.3f;
    float releaseTime = 1.0f;
    float pulseWidth = 0.5f;
    
    // Multi-oscillator parameters
    int numOscillators = 4;
    std::array<float, 4> detuneAmounts = {-12.0f, -5.0f, 5.0f, 12.0f}; // cents
    std::array<float, 4> oscLevels = {1.0f, 1.0f, 1.0f, 1.0f};
    
    // FM parameters
    float modIndex = 1.5f;
    float modRatio = 2.0f;
};

/**
 * Static Pad Preset Table
 */
class PadPresets
{
public:
    static const PadPresetData& getPreset(int index);
    static int getNumPresets();
    static juce::StringArray getPresetNames();

private:
    static const std::array<PadPresetData, 16> presets;
};
