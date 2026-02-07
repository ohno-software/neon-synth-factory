#pragma once

#include <JuceHeader.h>

/**
 * Reverb Effect for Pad and Arp engines
 * Time/Size controlled via numeric display (seconds)
 * Wet/Dry controlled via knob or slider
 */
class NeonReverb
{
public:
    //==============================================================================
    NeonReverb() = default;
    ~NeonReverb() = default;
    
    //==============================================================================
    void prepare(double sampleRate, int samplesPerBlock);
    void reset();
    
    //==============================================================================
    void setTime(float seconds);
    float getTime() const { return reverbTime; }
    
    void setMix(float mix) { wetDry = juce::jlimit(0.0f, 1.0f, mix); updateParameters(); }
    float getMix() const { return wetDry; }
    
    //==============================================================================
    void processBlock(juce::AudioBuffer<float>& buffer);

private:
    //==============================================================================
    void updateParameters();
    
    //==============================================================================
    juce::dsp::Reverb reverb;
    juce::dsp::Reverb::Parameters reverbParams;
    
    float reverbTime = 2.5f; // seconds
    float wetDry = 0.35f;
    
    double sampleRate = 44100.0;
    
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NeonReverb)
};
