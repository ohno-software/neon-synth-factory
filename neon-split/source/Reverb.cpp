#include "Reverb.h"

//==============================================================================
void NeonReverb::prepare(double sr, int samplesPerBlock)
{
    sampleRate = sr;
    
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sr;
    spec.maximumBlockSize = static_cast<juce::uint32>(samplesPerBlock);
    spec.numChannels = 2;
    
    reverb.prepare(spec);
    updateParameters();
}

void NeonReverb::reset()
{
    reverb.reset();
}

//==============================================================================
void NeonReverb::setTime(float seconds)
{
    reverbTime = juce::jlimit(0.1f, 10.0f, seconds);
    updateParameters();
}

void NeonReverb::updateParameters()
{
    // Map time (0.1-10s) to room size (0-1)
    // Also affects damping for longer/shorter decays
    float normalizedTime = (reverbTime - 0.1f) / 9.9f;
    
    reverbParams.roomSize = 0.3f + normalizedTime * 0.6f;
    reverbParams.damping = 0.3f + (1.0f - normalizedTime) * 0.4f;
    reverbParams.width = 0.8f;
    reverbParams.wetLevel = wetDry;
    reverbParams.dryLevel = 1.0f - wetDry;
    reverbParams.freezeMode = 0.0f;
    
    reverb.setParameters(reverbParams);
}

//==============================================================================
void NeonReverb::processBlock(juce::AudioBuffer<float>& buffer)
{
    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);
    reverb.process(context);
}
