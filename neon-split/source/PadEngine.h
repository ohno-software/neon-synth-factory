#pragma once

#include <JuceHeader.h>
#include "VoiceBase.h"
#include "Chorus.h"
#include "SyncDelay.h"
#include "Reverb.h"
#include "PadPresets.h"

/**
 * Pad Voice Engine - Upper split zone
 * Preset-only synthesis inspired by Solina, Juno, FM pads, Fairlight
 */
class PadEngine : public VoiceBase
{
public:
    //==============================================================================
    PadEngine();
    ~PadEngine() override = default;
    
    //==============================================================================
    void prepare(double sampleRate, int samplesPerBlock) override;
    void reset() override;
    
    //==============================================================================
    void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages);
    
    //==============================================================================
    // Preset management
    void setPreset(int presetIndex);
    int getPreset() const { return currentPreset; }
    juce::String getPresetName() const;
    int getNumPresets() const;
    void nextPreset();
    void previousPreset();
    
    //==============================================================================
    // Effects - Chorus
    void setChorusType(int type) { chorusType = juce::jlimit(0, 2, type); chorus.setType(type); }
    int getChorusType() const { return chorusType; }
    void setChorusMix(float mix) { chorus.setMix(mix); }
    
    // Effects - Delay
    void setDelayEnabled(bool enabled) { delayEnabled = enabled; }
    bool isDelayEnabled() const { return delayEnabled; }
    void setDelayTime(int syncIndex) { delay.setSyncTime(syncIndex); }
    void setDelayMix(float mix) { delay.setMix(mix); }
    
    // Effects - Reverb
    void setReverbEnabled(bool enabled) { reverbEnabled = enabled; }
    bool isReverbEnabled() const { return reverbEnabled; }
    void setReverbTime(float seconds) { reverb.setTime(seconds); }
    void setReverbMix(float mix) { reverb.setMix(mix); }
    
    //==============================================================================
    void setTempo(double bpm) { currentTempo = bpm; delay.setTempo(bpm); }
    
    //==============================================================================
    // Volume control
    void setVolume(float vol) { volume = juce::jlimit(0.0f, 1.0f, vol); }
    float getVolume() const { return volume; }

private:
    //==============================================================================
    void handleMidiEvent(const juce::MidiMessage& message);
    float renderVoices();
    void applyPresetParameters();
    
    //==============================================================================
    int currentPreset = 0;
    PadPresetData presetData;
    
    //==============================================================================
    Chorus chorus;
    SyncDelay delay;
    NeonReverb reverb;
    
    int chorusType = 0;
    bool delayEnabled = false;
    bool reverbEnabled = true;
    float volume = 0.8f;
    
    //==============================================================================
    double currentTempo = 120.0;
    
    //==============================================================================
    // Multi-oscillator state per voice
    struct VoiceState
    {
        std::array<float, 4> oscPhases = {0.0f, 0.0f, 0.0f, 0.0f};
        std::array<float, 4> modPhases = {0.0f, 0.0f, 0.0f, 0.0f};
    };
    std::array<VoiceState, maxVoices> voiceStates;
    
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PadEngine)
};
