#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "SidSignalPath.h"

namespace neon
{
    class NeonSidAudioProcessor : public juce::AudioProcessor
    {
    public:
        NeonSidAudioProcessor();
        ~NeonSidAudioProcessor() override;

        void prepareToPlay (double sampleRate, int samplesPerBlock) override;
        void releaseResources() override;

        void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

        juce::AudioProcessorEditor* createEditor() override;
        bool hasEditor() const override { return true; }

        const juce::String getName() const override { return "Neon SID"; }

        double getTailLengthSeconds() const override { return 0.0; }

        bool acceptsMidi() const override { return true; }
        bool producesMidi() const override { return false; }
        bool isMidiEffect() const override { return false; }

        int getNumPrograms() override { return 1; }
        int getCurrentProgram() override { return 0; }
        void setCurrentProgram (int) override {}
        const juce::String getProgramName (int) override { return {}; }
        void changeProgramName (int, const juce::String&) override {}
        void getStateInformation (juce::MemoryBlock&) override {}
        void setStateInformation (const void*, int) override {}

        juce::MidiKeyboardState& getKeyboardState() { return keyboardState; }

    private:
        SidSignalPath signalPath;
        juce::MidiKeyboardState keyboardState;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NeonSidAudioProcessor)
    };
}

