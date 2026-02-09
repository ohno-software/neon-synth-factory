#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace neon
{
    NeonTemplateAudioProcessor::NeonTemplateAudioProcessor()
        : AudioProcessor(BusesProperties().withOutput("Output", juce::AudioChannelSet::stereo(), true))
    {
    }

    NeonTemplateAudioProcessor::~NeonTemplateAudioProcessor() = default;

    juce::AudioProcessorEditor* NeonTemplateAudioProcessor::createEditor()
    {
        return new NeonTemplateAudioProcessorEditor(*this);
    }
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new neon::NeonTemplateAudioProcessor();
}
