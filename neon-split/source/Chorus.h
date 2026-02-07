#pragma once

#include <JuceHeader.h>

/**
 * Chorus Effect for Pad engine
 * Exactly 3 types selected via square radio buttons
 * Wet/Dry only control
 */
class Chorus
{
public:
    //==============================================================================
    enum class ChorusType
    {
        TypeI,   // Subtle ensemble
        TypeII,  // Classic Juno-style
        TypeIII  // Deep/lush
    };
    
    //==============================================================================
    Chorus() = default;
    ~Chorus() = default;
    
    //==============================================================================
    void prepare(double sampleRate, int samplesPerBlock);
    void reset();
    
    //==============================================================================
    void setType(int type);
    int getType() const { return static_cast<int>(chorusType); }
    
    void setMix(float mix) { wetDry = juce::jlimit(0.0f, 1.0f, mix); }
    float getMix() const { return wetDry; }
    
    //==============================================================================
    void processBlock(juce::AudioBuffer<float>& buffer);
    
    //==============================================================================
    static juce::StringArray getTypeNames();

private:
    //==============================================================================
    void updateParameters();
    
    //==============================================================================
    double sampleRate = 44100.0;
    ChorusType chorusType = ChorusType::TypeI;
    float wetDry = 0.5f;
    
    //==============================================================================
    // Simple delay-line chorus implementation
    static constexpr int maxDelayMs = 30;
    std::vector<float> delayLineL;
    std::vector<float> delayLineR;
    int writePos = 0;
    int maxDelaySamples = 0;
    
    // LFO state
    float lfoPhase1 = 0.0f;
    float lfoPhase2 = 0.0f;
    float lfoPhase3 = 0.0f;
    
    // Per-type parameters
    struct ChorusParams
    {
        float rate;       // LFO rate Hz
        float depth;      // Delay modulation depth
        float baseDelay;  // Base delay in ms
        float spread;     // Stereo spread
    };
    ChorusParams params;
    
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Chorus)
};
