#pragma once

#include <JuceHeader.h>
#include "VoiceBase.h"
#include "SyncDelay.h"
#include "Reverb.h"
#include "ArpPresets.h"

/**
 * Arpeggiator Voice Engine - Upper split zone, layered with pad
 * Independent sound engine with its own waveform selection
 * Plays in parallel with the pad voice
 */
class ArpEngine : public VoiceBase
{
public:
    //==============================================================================
    enum class ArpPattern
    {
        Up,
        Down,
        UpDown,
        SynthwaveGate,
        Random
    };
    
    //==============================================================================
    ArpEngine();
    ~ArpEngine() override = default;
    
    //==============================================================================
    void prepare(double sampleRate, int samplesPerBlock) override;
    void reset() override;
    
    //==============================================================================
    void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages,
                      double tempo, double ppqPosition);
    
    //==============================================================================
    // Enable/Disable
    void setEnabled(bool enabled) { arpEnabled = enabled; }
    bool isEnabled() const { return arpEnabled; }
    
    //==============================================================================
    // Waveform selection
    void setWaveform(int waveformIndex);
    int getWaveform() const { return static_cast<int>(currentWaveform); }
    juce::String getWaveformName() const;
    
    //==============================================================================
    // Pattern selection
    void setPattern(int patternIndex);
    int getPattern() const { return static_cast<int>(currentPattern); }
    juce::String getPatternName() const;
    
    //==============================================================================
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
    
    // Effects - Filter
    void setFilterCutoff(float hz);
    void setResonanceEnabled(bool enabled);
    
    //==============================================================================
    // Volume control
    void setVolume(float vol) { volume = juce::jlimit(0.0f, 1.0f, vol); }
    float getVolume() const { return volume; }

private:
    //==============================================================================
    void handleMidiEvent(const juce::MidiMessage& message);
    void updateArpeggiator(double tempo, double ppqPosition);
    float renderCurrentNote();
    int getNextArpNote();
    
    //==============================================================================
    bool arpEnabled = false;
    WaveformType currentWaveform = WaveformType::AnalogSaw;
    ArpPattern currentPattern = ArpPattern::Up;
    
    //==============================================================================
    // Held notes for arpeggiator
    std::vector<int> heldNotes;
    int currentArpIndex = 0;
    int currentArpNote = -1;
    bool arpDirectionUp = true;
    
    //==============================================================================
    // Timing
    double lastPpqPosition = 0.0;
    double stepLength = 0.25; // 16th notes
    double lastStepPpq = 0.0;
    
    //==============================================================================
    // Current note synthesis
    float currentPhase = 0.0f;
    float modPhase = 0.0f;
    float currentEnvelope = 0.0f;
    float currentFrequency = 0.0f;
    
    //==============================================================================
    // Effects
    SyncDelay delay;
    NeonReverb reverb;
    bool delayEnabled = false;
    bool reverbEnabled = false;
    
    juce::dsp::IIR::Filter<float> filter;
    float filterCutoff = 20000.0f;
    bool resonanceEnabled = false;
    
    //==============================================================================
    // Gate for synthwave pattern
    float gateEnvelope = 0.0f;
    float volume = 0.8f;
    
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ArpEngine)
};
