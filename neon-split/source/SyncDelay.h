#pragma once

#include <JuceHeader.h>

/**
 * Tempo-synced Delay Effect
 * Time controlled via numeric display (sync divisions)
 * Wet/Dry controlled via knob or slider
 */
class SyncDelay
{
public:
    //==============================================================================
    enum class SyncTime
    {
        Sixteenth,  // 1/16
        Eighth,     // 1/8
        Quarter,    // 1/4
        Half,       // 1/2
        Whole       // 1/1
    };
    
    //==============================================================================
    SyncDelay();
    ~SyncDelay() = default;
    
    //==============================================================================
    void prepare(double sampleRate, int samplesPerBlock);
    void reset();
    
    //==============================================================================
    void setTempo(double bpm) { tempo = bpm; updateDelayTime(); }
    void setSyncTime(int syncIndex);
    int getSyncTime() const { return static_cast<int>(syncTime); }
    
    void setMix(float mix) { wetDry = juce::jlimit(0.0f, 1.0f, mix); }
    float getMix() const { return wetDry; }
    
    void setFeedback(float fb) { feedback = juce::jlimit(0.0f, 0.9f, fb); }
    
    //==============================================================================
    void processBlock(juce::AudioBuffer<float>& buffer);
    
    //==============================================================================
    static juce::StringArray getSyncTimeNames();

private:
    //==============================================================================
    void updateDelayTime();
    
    //==============================================================================
    double sampleRate = 44100.0;
    double tempo = 120.0;
    SyncTime syncTime = SyncTime::Eighth;
    
    float wetDry = 0.3f;
    float feedback = 0.4f;
    
    //==============================================================================
    // Delay buffer (stereo)
    std::vector<float> delayBufferL;
    std::vector<float> delayBufferR;
    int writePosition = 0;
    int delayInSamples = 0;
    int maxDelaySamples = 0;
    
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SyncDelay)
};
