#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace neon
{
    NeonTemplateAudioProcessor::NeonTemplateAudioProcessor()
        : AudioProcessor (BusesProperties()
                             .withOutput ("Output", juce::AudioChannelSet::stereo(), true))
    {
    }

    NeonTemplateAudioProcessor::~NeonTemplateAudioProcessor()
    {
    }

    void NeonTemplateAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
    {
        // No audio processing for template
    }

    void NeonTemplateAudioProcessor::releaseResources()
    {
    }

    void NeonTemplateAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
    {
        juce::ScopedNoDenormals noDenormals;
        
        // Check for MIDI activity
        if (!midiMessages.isEmpty())
            midiActivity.store (true);

        // Clear output buffer (no audio processing)
        buffer.clear();
    }

    juce::AudioProcessorEditor* NeonTemplateAudioProcessor::createEditor()
    {
        return new NeonTemplateAudioProcessorEditor (*this);
    }
}

// Plugin interface
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new neon::NeonTemplateAudioProcessor();
}
