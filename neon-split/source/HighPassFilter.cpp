#include "HighPassFilter.h"

//==============================================================================
void HighPassFilter::prepare(double sr, int /*samplesPerBlock*/)
{
    sampleRate = sr;
    reset();
    updateCoefficients();
}

void HighPassFilter::reset()
{
    x1.fill(0.0f);
    x2.fill(0.0f);
    y1.fill(0.0f);
    y2.fill(0.0f);
}

//==============================================================================
void HighPassFilter::setCutoff(float hz)
{
    cutoffHz = juce::jlimit(20.0f, 500.0f, hz);
    updateCoefficients();
}

//==============================================================================
void HighPassFilter::updateCoefficients()
{
    // Butterworth 2-pole high-pass filter
    float omega = juce::MathConstants<float>::twoPi * cutoffHz / static_cast<float>(sampleRate);
    float cosOmega = std::cos(omega);
    float sinOmega = std::sin(omega);
    float alpha = sinOmega / (2.0f * 0.707f); // Q = 0.707 for Butterworth
    
    float b0 = (1.0f + cosOmega) / 2.0f;
    float b1Raw = -(1.0f + cosOmega);
    float b2Raw = (1.0f + cosOmega) / 2.0f;
    float a0Raw = 1.0f + alpha;
    float a1Raw = -2.0f * cosOmega;
    float a2Raw = 1.0f - alpha;
    
    // Normalize
    a0 = b0 / a0Raw;
    a1 = b1Raw / a0Raw;
    a2 = b2Raw / a0Raw;
    b1 = a1Raw / a0Raw;
    b2 = a2Raw / a0Raw;
}

//==============================================================================
void HighPassFilter::processBlock(juce::AudioBuffer<float>& buffer)
{
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
    {
        auto* data = buffer.getWritePointer(channel);
        int ch = juce::jmin(channel, 1); // Max 2 channels for state
        
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            float input = data[sample];
            
            // Direct Form II Transposed
            float output = a0 * input + a1 * x1[ch] + a2 * x2[ch] - b1 * y1[ch] - b2 * y2[ch];
            
            // Update state
            x2[ch] = x1[ch];
            x1[ch] = input;
            y2[ch] = y1[ch];
            y1[ch] = output;
            
            data[sample] = output;
        }
    }
}
