/*
  ==============================================================================

    NeonQwenSynthVoice.cpp
    Created: 31 Jan 2026 3:53:36pm
    Author:  Adrian

  ==============================================================================
*/

#include "NeonQwenSynthVoice.h"
#include "NeonQwenSynthProcessor.h"

//==============================================================================
NeonQwenSynthVoice::NeonQwenSynthVoice()
{
}

bool NeonQwenSynthVoice::canPlaySound (juce::SynthesiserSound* sound)
{
    return dynamic_cast<NeonQwenSynthSound*> (sound) != nullptr;
}

void NeonQwenSynthVoice::startNote (int midiNoteNumber, float velocity,
                                    juce::SynthesiserSound*, int currentPitchWheelPosition)
{
    currentMidiNote = midiNoteNumber;
    
    // Calculate frequency from MIDI note number
    auto cyclesPerSecond = juce::MidiMessage::getMidiNoteInHertz(midiNoteNumber);
    currentFrequency = (float)cyclesPerSecond;
    
    // Set up initial parameters
    level = velocity * 0.15f;
    tailOff = 0.0;
    
    // Initialize frequency smoothing
    targetFrequency = currentFrequency;
    currentFrequencySmoothed = currentFrequency;
    
    updateAngleDelta();
}

void NeonQwenSynthVoice::stopNote (float velocity, bool allowTailOff)
{
    if (allowTailOff)
    {
        // Start a tail-off by setting this flag. The render callback will pick up
        // this and do a fade out, calling clearCurrentNote() when it's finished.
        if (juce::approximatelyEqual(tailOff, 0.0))
            tailOff = 1.0;
    }
    else
    {
        // We're being told to stop playing immediately, so reset everything.
        clearCurrentNote();
        angleDelta = 0.0;
    }
}

void NeonQwenSynthVoice::pitchWheelMoved (int newValue)
{
    // Handle pitch wheel changes - convert to semitones
    auto pitchWheel = (newValue - 8192) / 8192.0f;
    // Convert to semitone change (±2 octaves)
    auto semitoneChange = pitchWheel * 24.0f;
    auto frequencyRatio = std::pow(2.0f, semitoneChange / 12.0f);
    auto newFrequency = currentFrequency * frequencyRatio;
    
    targetFrequency = newFrequency;
}

void NeonQwenSynthVoice::controllerMoved (int controllerNumber, int newValue)
{
    // Handle MIDI controller changes
    // For example, controller 1 could be modulation wheel
    if (controllerNumber == 1)
    {
        // Modulation wheel affects waveform shape or other parameters
        // This is a simple example - you can expand this based on your needs
    }
}

void NeonQwenSynthVoice::updateAngleDelta()
{
    auto sampleRate = getSampleRate();
    
    if (sampleRate > 0.0)
    {
        auto cyclesPerSample = currentFrequencySmoothed / sampleRate;
        angleDelta = cyclesPerSample * juce::MathConstants<double>::twoPi;
    }
}

void NeonQwenSynthVoice::renderNextBlock (juce::AudioBuffer<float>& outputBuffer,
                                          int startSample,
                                          int numSamples)
{
    if (! juce::approximatelyEqual(angleDelta, 0.0))
    {
        if (tailOff > 0.0)
        {
            // Tail off the sound
            while (--numSamples >= 0)
            {
                auto currentSample = getSampleForWaveform(currentAngle) * level * tailOff;
                
                for (int i = outputBuffer.getNumChannels(); --i >= 0;)
                    outputBuffer.addSample(i, startSample, currentSample);
                
                currentAngle += angleDelta;
                ++startSample;
                
                tailOff *= 0.99f;
                
                if (tailOff <= 0.005)
                {
                    clearCurrentNote();
                    angleDelta = 0.0;
                    break;
                }
            }
        }
        else
        {
            // Normal rendering without tail off
            while (--numSamples >= 0)
            {
                auto currentSample = getSampleForWaveform(currentAngle) * level;
                
                for (int i = outputBuffer.getNumChannels(); --i >= 0;)
                    outputBuffer.addSample(i, startSample, currentSample);
                
                currentAngle += angleDelta;
                ++startSample;
            }
        }
    }
}

float NeonQwenSynthVoice::getSampleForWaveform(double angle)
{
    switch (waveformType)
    {
        case sineWave:
            return (float)std::sin(angle);
            
        case squareWave:
            return (angle < juce::MathConstants<double>::pi) ? 1.0f : -1.0f;
            
        case sawtoothWave:
            // Sawtooth wave: 2 * (x / 2π) - 1
            return (float)(2.0 * (angle / juce::MathConstants<double>::twoPi) - 1.0);
            
        case triangleWave:
        {
            // Triangle wave: 
            // For a full cycle from 0 to 2π, the triangle wave goes up then down
            double normalizedAngle = angle / juce::MathConstants<double>::twoPi;
            return (float)(1.0 - 4.0 * std::abs(std::fmod(normalizedAngle + 0.25, 1.0) - 0.5));
        }
            
        default:
            return (float)std::sin(angle);
    }
}