#pragma once

#include <JuceHeader.h>

/**
 * High-Pass Filter for Bass Engine
 * Controlled via numeric display only (no knob)
 */
class HighPassFilter
{
public:
    //==============================================================================
    HighPassFilter() = default;
    ~HighPassFilter() = default;
    
    //==============================================================================
    void prepare(double sampleRate, int samplesPerBlock);
    void reset();
    
    //==============================================================================
    void setCutoff(float hz);
    float getCutoff() const { return cutoffHz; }
    
    //==============================================================================
    void processBlock(juce::AudioBuffer<float>& buffer);

private:
    //==============================================================================
    void updateCoefficients();
    
    //==============================================================================
    double sampleRate = 44100.0;
    float cutoffHz = 40.0f;
    
    //==============================================================================
    // Simple 2-pole highpass filter state
    std::array<float, 2> x1 = {0.0f, 0.0f}; // Previous input
    std::array<float, 2> x2 = {0.0f, 0.0f}; // Previous previous input
    std::array<float, 2> y1 = {0.0f, 0.0f}; // Previous output
    std::array<float, 2> y2 = {0.0f, 0.0f}; // Previous previous output
    
    // Coefficients
    float a0 = 1.0f, a1 = 0.0f, a2 = 0.0f;
    float b1 = 0.0f, b2 = 0.0f;
    
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(HighPassFilter)
};
