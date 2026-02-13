#pragma once

#include <JuceHeader.h>
#include <array>
#include "VoiceBase.h"
#include "HighPassFilter.h"
#include "SyncDelay.h"
#include "BassPresets.h"

/**
 * Bass Voice Engine - Lower split zone
 * Preset-only synthesis inspired by Moog, SH-101, BassStation, ARP 2600
 * 3-oscillator architecture with resonant lowpass filter
 * MONOPHONIC - new notes cut off previous notes (no legato)
 */
class BassEngine : public VoiceBase
{
public:
    //==============================================================================
    BassEngine();
    ~BassEngine() override = default;
    
    //==============================================================================
    void prepare(double sampleRate, int samplesPerBlock) override;
    void reset() override;
    
    //==============================================================================
    // Monophonic note handling - override base class
    void noteOn(int noteNumber, float velocity) override;
    
    //==============================================================================
    void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages);
    
    //==============================================================================
    // Volume control
    void setVolume(float vol) { volume = juce::jlimit(0.0f, 1.0f, vol); }
    float getVolume() const { return volume; }
    
    //==============================================================================
    // Preset management
    void setPreset(int presetIndex);
    int getPreset() const { return currentPreset; }
    juce::String getPresetName() const;
    int getNumPresets() const;
    void nextPreset();
    void previousPreset();
    
    //==============================================================================
    // Pattern control
    void setPatternEnabled(bool enabled) { patternEnabled = enabled; }
    bool isPatternEnabled() const { return patternEnabled; }
    void setPattern(int patternIndex);
    int getPattern() const { return currentPattern; }
    
    //==============================================================================
    // Effects
    void setLPFCutoff(float hz);
    float getLPFCutoff() const { return lpfCutoff; }
    
    void setDelayEnabled(bool enabled) { delayEnabled = enabled; }
    bool isDelayEnabled() const { return delayEnabled; }
    void setDelayTime(int syncIndex) { delay.setSyncTime(syncIndex); }
    void setDelayMix(float mix) { delay.setMix(mix); }
    
    //==============================================================================
    void setTempo(double bpm) { currentTempo = bpm; delay.setTempo(bpm); }

private:
    //==============================================================================
    void handleMidiEvent(const juce::MidiMessage& message);
    float renderVoices();
    void applyPresetParameters();
    
    // Simple 2-pole lowpass filter (per-voice state)
    struct LPFState
    {
        float z1 = 0.0f;
        float z2 = 0.0f;
    };
    float processLPF(float input, float cutoff, float resonance, LPFState& state);
    
    //==============================================================================
    int currentPreset = 0;
    BassPresetData presetData;
    float volume = 0.8f;
    
    //==============================================================================
    bool patternEnabled = false;
    int currentPattern = 0;
    
    //==============================================================================
    juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>> lpFilter;
    SyncDelay delay;
    float lpfCutoff = 20000.0f;
    bool delayEnabled = false;
    
    //==============================================================================
    double currentTempo = 120.0;
    
    //==============================================================================
    // Per-voice state (only voice 0 used for monophonic)
    std::array<float, maxVoices> modPhases;      // FM modulator phases
    std::array<float, maxVoices> osc2Phases;     // Detuned oscillator phases
    std::array<float, maxVoices> subPhases;      // Sub oscillator phases
    std::array<LPFState, maxVoices> lpfStates;   // Per-voice filter state
    std::array<float, maxVoices> filterEnvs;     // Per-voice filter envelope
    
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BassEngine)
};
