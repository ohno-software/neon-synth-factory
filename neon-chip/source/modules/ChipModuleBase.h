#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include <neon_ui_components/modules/ModuleBase.h>

namespace neon
{
    class ChipModuleBase : public ModuleBase
    {
    public:
        ChipModuleBase(const juce::String& name, const juce::Colour& color) 
            : ModuleBase(name, color)
        {
        }

        virtual ~ChipModuleBase() = default;

        // Virtual methods for chip-specific implementations
        virtual void setSampleRate(double sampleRate) { }
        virtual void prepareToPlay(int samplesPerBlock) { }
        virtual void processBlock(juce::AudioBuffer<float>& buffer, int numChannels) { }
        
    protected:
        double currentSampleRate = 44100.0;
        int currentSamplesPerBlock = 512;
    };
}