#include "Chorus.h"

//==============================================================================
void Chorus::prepare(double sr, int /*samplesPerBlock*/)
{
    sampleRate = sr;
    
    maxDelaySamples = static_cast<int>(sr * maxDelayMs / 1000.0);
    delayLineL.resize(maxDelaySamples, 0.0f);
    delayLineR.resize(maxDelaySamples, 0.0f);
    
    reset();
    updateParameters();
}

void Chorus::reset()
{
    std::fill(delayLineL.begin(), delayLineL.end(), 0.0f);
    std::fill(delayLineR.begin(), delayLineR.end(), 0.0f);
    writePos = 0;
    lfoPhase1 = 0.0f;
    lfoPhase2 = 0.33f;
    lfoPhase3 = 0.66f;
}

//==============================================================================
void Chorus::setType(int type)
{
    chorusType = static_cast<ChorusType>(juce::jlimit(0, 2, type));
    updateParameters();
}

void Chorus::updateParameters()
{
    switch (chorusType)
    {
        case ChorusType::TypeI:
            // Subtle ensemble - Solina style
            params.rate = 0.5f;
            params.depth = 0.002f;  // 2ms modulation
            params.baseDelay = 7.0f;
            params.spread = 0.3f;
            break;
            
        case ChorusType::TypeII:
            // Classic Juno-style
            params.rate = 0.8f;
            params.depth = 0.004f;  // 4ms modulation
            params.baseDelay = 10.0f;
            params.spread = 0.5f;
            break;
            
        case ChorusType::TypeIII:
            // Deep/lush
            params.rate = 0.3f;
            params.depth = 0.008f;  // 8ms modulation
            params.baseDelay = 15.0f;
            params.spread = 0.8f;
            break;
    }
}

//==============================================================================
void Chorus::processBlock(juce::AudioBuffer<float>& buffer)
{
    if (buffer.getNumChannels() < 1)
        return;
    
    auto* leftChannel = buffer.getWritePointer(0);
    auto* rightChannel = buffer.getNumChannels() > 1 ? buffer.getWritePointer(1) : nullptr;
    
    float lfoIncrement = params.rate / static_cast<float>(sampleRate);
    float baseDelaySamples = params.baseDelay * static_cast<float>(sampleRate) / 1000.0f;
    float depthSamples = params.depth * static_cast<float>(sampleRate);
    
    for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
    {
        float dryL = leftChannel[sample];
        float dryR = rightChannel ? rightChannel[sample] : dryL;
        
        // Write to delay lines
        delayLineL[writePos] = dryL;
        delayLineR[writePos] = dryR;
        
        // Calculate modulated delay times using 3 LFOs
        float mod1 = std::sin(lfoPhase1 * juce::MathConstants<float>::twoPi);
        float mod2 = std::sin(lfoPhase2 * juce::MathConstants<float>::twoPi);
        float mod3 = std::sin(lfoPhase3 * juce::MathConstants<float>::twoPi);
        
        // Left channel uses LFO 1 and 2
        float delayL1 = baseDelaySamples + mod1 * depthSamples;
        float delayL2 = baseDelaySamples + mod2 * depthSamples * 0.7f;
        
        // Right channel uses LFO 2 and 3 (offset for stereo)
        float delayR1 = baseDelaySamples + mod2 * depthSamples;
        float delayR2 = baseDelaySamples + mod3 * depthSamples * 0.7f;
        
        // Read with linear interpolation
        auto readInterpolated = [this](const std::vector<float>& delayLine, float delaySamples) -> float
        {
            float readPos = static_cast<float>(writePos) - delaySamples;
            while (readPos < 0.0f)
                readPos += static_cast<float>(maxDelaySamples);
            
            int readIndex = static_cast<int>(readPos);
            float frac = readPos - static_cast<float>(readIndex);
            
            int nextIndex = (readIndex + 1) % maxDelaySamples;
            readIndex = readIndex % maxDelaySamples;
            
            return delayLine[readIndex] * (1.0f - frac) + delayLine[nextIndex] * frac;
        };
        
        float wetL = (readInterpolated(delayLineL, delayL1) + readInterpolated(delayLineL, delayL2)) * 0.5f;
        float wetR = (readInterpolated(delayLineR, delayR1) + readInterpolated(delayLineR, delayR2)) * 0.5f;
        
        // Apply stereo spread
        float spreadL = wetL * (1.0f - params.spread * 0.5f) + wetR * params.spread * 0.5f;
        float spreadR = wetR * (1.0f - params.spread * 0.5f) + wetL * params.spread * 0.5f;
        
        // Mix wet/dry
        leftChannel[sample] = dryL * (1.0f - wetDry) + spreadL * wetDry;
        if (rightChannel)
            rightChannel[sample] = dryR * (1.0f - wetDry) + spreadR * wetDry;
        
        // Advance LFOs
        lfoPhase1 += lfoIncrement;
        lfoPhase2 += lfoIncrement * 1.1f; // Slightly different rate
        lfoPhase3 += lfoIncrement * 0.9f;
        
        if (lfoPhase1 >= 1.0f) lfoPhase1 -= 1.0f;
        if (lfoPhase2 >= 1.0f) lfoPhase2 -= 1.0f;
        if (lfoPhase3 >= 1.0f) lfoPhase3 -= 1.0f;
        
        // Advance write position
        writePos++;
        if (writePos >= maxDelaySamples)
            writePos = 0;
    }
}

//==============================================================================
juce::StringArray Chorus::getTypeNames()
{
    return {"Type I", "Type II", "Type III"};
}
