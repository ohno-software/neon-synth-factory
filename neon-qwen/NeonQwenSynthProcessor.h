/*
  ==============================================================================

    NeonQwenSynthProcessor.h
    Created: 31 Jan 2026 3:51:06pm
    Author:  Adrian

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "NeonQwenSynthVoice.h"
#include "NeonQwenSynthSound.h"

//==============================================================================
/**
*/
class NeonQwenSynthProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    NeonQwenSynthProcessor();
    ~NeonQwenSynthProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JUCE_LINUX
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    // Accessors for parameters
    float getVolume() const { return volume; }
    void setVolume(float newVolume) { volume = newVolume; }

    float getAttack() const { return attack; }
    void setAttack(float newAttack) { attack = newAttack; }

    float getRelease() const { return release; }
    void setRelease(float newRelease) { release = newRelease; }

private:
    //==============================================================================
    juce::Synthesiser synth;
    
    // Parameters
    float volume;
    float attack;
    float release;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NeonQwenSynthProcessor)
};