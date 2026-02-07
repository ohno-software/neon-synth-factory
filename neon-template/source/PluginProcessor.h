#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

namespace neon
{
    class NeonTemplateAudioProcessor : public juce::AudioProcessor
    {
    public:
        NeonTemplateAudioProcessor();
        ~NeonTemplateAudioProcessor() override;

        void prepareToPlay (double sampleRate, int samplesPerBlock) override {}
        void releaseResources() override {}

        void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override {}

        juce::AudioProcessorEditor* createEditor() override;
        bool hasEditor() const override { return true; }

        const juce::String getName() const override { return "Neon Template"; }

        double getTailLengthSeconds() const override { return 0.0; }

        bool acceptsMidi() const override { return true; }
        bool producesMidi() const override { return false; }
        bool isMidiEffect() const override { return false; }
        bool isSynth() const override { return false; }

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NeonTemplateAudioProcessor)
    };
}
