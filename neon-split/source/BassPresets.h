#pragma once

#include <JuceHeader.h>
#include "VoiceBase.h"

/**
 * Bass Preset Data Structure
 * Inspired by: Moog, SH-101, BassStation, ARP 2600
 */
struct BassPresetData
{
    juce::String name;
    VoiceBase::WaveformType waveform;
    
    // Synthesis parameters (not user-adjustable)
    float level = 0.8f;
    float attackTime = 0.01f;
    float releaseTime = 0.3f;
    float pulseWidth = 0.5f;
    
    // Multi-oscillator setup (3 osc: main, detuned, sub)
    float osc2Detune = 0.0f;      // Cents detune for osc 2 (-50 to +50)
    float osc2Level = 0.0f;        // Level of detuned osc 2
    float subOscLevel = 0.0f;      // Sub oscillator level (1 octave down)
    
    // Filter (lowpass)
    float filterCutoff = 1.0f;     // 0.0 to 1.0 (normalized)
    float filterResonance = 0.0f;  // 0.0 to 1.0
    float filterEnvAmount = 0.0f;  // How much envelope opens filter
    
    // FM parameters
    float modIndex = 2.0f;
    float modRatio = 2.0f;
};

/**
 * Static Bass Preset Table
 */
class BassPresets
{
public:
    static const BassPresetData& getPreset(int index);
    static int getNumPresets();
    static juce::StringArray getPresetNames();

private:
    static const std::array<BassPresetData, 16> presets;
};
