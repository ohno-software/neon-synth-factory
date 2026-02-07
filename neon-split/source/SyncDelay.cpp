#include "SyncDelay.h"

//==============================================================================
SyncDelay::SyncDelay()
{
}

//==============================================================================
void SyncDelay::prepare(double sr, int /*samplesPerBlock*/)
{
    sampleRate = sr;
    
    // Max delay = 2 bars at 60 BPM = 8 seconds
    maxDelaySamples = static_cast<int>(sampleRate * 8.0);
    
    delayBufferL.resize(maxDelaySamples, 0.0f);
    delayBufferR.resize(maxDelaySamples, 0.0f);
    
    reset();
    updateDelayTime();
}

void SyncDelay::reset()
{
    std::fill(delayBufferL.begin(), delayBufferL.end(), 0.0f);
    std::fill(delayBufferR.begin(), delayBufferR.end(), 0.0f);
    writePosition = 0;
}

//==============================================================================
void SyncDelay::setSyncTime(int syncIndex)
{
    syncTime = static_cast<SyncTime>(juce::jlimit(0, 4, syncIndex));
    updateDelayTime();
}

void SyncDelay::updateDelayTime()
{
    // Calculate delay time based on tempo and sync division
    double beatsPerSecond = tempo / 60.0;
    double beatDuration = 1.0 / beatsPerSecond;
    
    double delayBeats = 1.0;
    switch (syncTime)
    {
        case SyncTime::Sixteenth: delayBeats = 0.25; break;
        case SyncTime::Eighth:    delayBeats = 0.5; break;
        case SyncTime::Quarter:   delayBeats = 1.0; break;
        case SyncTime::Half:      delayBeats = 2.0; break;
        case SyncTime::Whole:     delayBeats = 4.0; break;
    }
    
    double delaySeconds = beatDuration * delayBeats;
    delayInSamples = static_cast<int>(delaySeconds * sampleRate);
    delayInSamples = juce::jlimit(1, maxDelaySamples - 1, delayInSamples);
}

//==============================================================================
void SyncDelay::processBlock(juce::AudioBuffer<float>& buffer)
{
    if (buffer.getNumChannels() < 1)
        return;
    
    auto* leftChannel = buffer.getWritePointer(0);
    auto* rightChannel = buffer.getNumChannels() > 1 ? buffer.getWritePointer(1) : nullptr;
    
    for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
    {
        // Read position
        int readPosition = writePosition - delayInSamples;
        if (readPosition < 0)
            readPosition += maxDelaySamples;
        
        // Read delayed samples
        float delayedL = delayBufferL[readPosition];
        float delayedR = rightChannel ? delayBufferR[readPosition] : delayedL;
        
        // Get dry input
        float dryL = leftChannel[sample];
        float dryR = rightChannel ? rightChannel[sample] : dryL;
        
        // Write to delay buffer with feedback
        delayBufferL[writePosition] = dryL + delayedL * feedback;
        if (rightChannel)
            delayBufferR[writePosition] = dryR + delayedR * feedback;
        
        // Mix wet/dry
        leftChannel[sample] = dryL * (1.0f - wetDry) + delayedL * wetDry;
        if (rightChannel)
            rightChannel[sample] = dryR * (1.0f - wetDry) + delayedR * wetDry;
        
        // Advance write position
        writePosition++;
        if (writePosition >= maxDelaySamples)
            writePosition = 0;
    }
}

//==============================================================================
juce::StringArray SyncDelay::getSyncTimeNames()
{
    return {"1/16", "1/8", "1/4", "1/2", "1/1"};
}
