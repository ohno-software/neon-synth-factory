/*
  ==============================================================================

    NeonQwenSynthVoice.h
    Created: 31 Jan 2026 3:53:36pm
    Author:  Adrian

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/**
*/
class NeonQwenSynthVoice : public juce::SynthesiserVoice
{
public:
    //==============================================================================
    NeonQwenSynthVoice();
    
    bool canPlaySound (juce::SynthesiserSound* sound) override;
    
    void startNote (int midiNoteNumber, float velocity,
                    juce::SynthesiserSound*, int currentPitchWheelPosition) override;
    
    void stopNote (float velocity, bool allowTailOff) override;
    
    void pitchWheelMoved (int newValue) override;
    void controllerMoved (int controllerNumber, int newValue) override;
    
    void renderNextBlock (juce::AudioBuffer<float>& outputBuffer,
                          int startSample,
                          int numSamples) override;

private:
    //==============================================================================
    double currentAngle = 0.0, angleDelta = 0.0, level = 0.0, tailOff = 0.0;
    int currentMidiNote = 0;
    float currentFrequency = 0.0f;
    
    // Waveform type
    enum WaveformType
    {
        sineWave,
        squareWave,
        sawtoothWave,
        triangleWave
    };
    
    WaveformType waveformType = sineWave;
    
    // Set waveform type
    void setWaveform(WaveformType type) { waveformType = type; }
    
    // Smoothed frequency for pitch changes
    float targetFrequency = 0.0f;
    float currentFrequencySmoothed = 0.0f;
    
    // Parameter smoothing
    void updateAngleDelta();
    
    // Waveform generation functions
    float getSampleForWaveform(double angle);
};