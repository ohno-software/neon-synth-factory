#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <neon_ui_components/core/NeonPatchManager.h>

namespace neon
{
    NeonSidAudioProcessor::NeonSidAudioProcessor()
        : AudioProcessor (BusesProperties().withOutput ("Output", juce::AudioChannelSet::stereo(), true))
    {
        PatchManager::getInstance().initialize ("NeonSid");
    }

    NeonSidAudioProcessor::~NeonSidAudioProcessor() = default;

    void NeonSidAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
    {
        signalPath.prepareToPlay (samplesPerBlock, sampleRate);
    }

    void NeonSidAudioProcessor::releaseResources()
    {
        signalPath.releaseResources();
    }

    void NeonSidAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
    {
        juce::ScopedNoDenormals noDenormals;
        auto totalNumInputChannels  = getTotalNumInputChannels();
        auto totalNumOutputChannels = getTotalNumOutputChannels();

        for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
            buffer.clear (i, 0, buffer.getNumSamples());

        keyboardState.processNextMidiBuffer (midiMessages, 0, buffer.getNumSamples(), true);

        for (const auto metadata : midiMessages)
        {
            const auto msg = metadata.getMessage();
            if (msg.isNoteOn())
                signalPath.noteOn (msg.getNoteNumber(), msg.getFloatVelocity());
            else if (msg.isNoteOff())
                signalPath.noteOff (msg.getNoteNumber());
            else if (msg.isPitchWheel())
                signalPath.setPitchWheel (juce::jmap ((float)msg.getPitchWheelValue(), 0.0f, 16383.0f, -1.0f, 1.0f));
            else if (msg.isController() && msg.getControllerNumber() == 1)
                signalPath.setModWheel (msg.getControllerValue() / 127.0f);
        }

        juce::AudioSourceChannelInfo info (&buffer, 0, buffer.getNumSamples());
        signalPath.getNextAudioBlock (info);
    }

    juce::AudioProcessorEditor* NeonSidAudioProcessor::createEditor()
    {
        return new NeonSidAudioProcessorEditor (*this);
    }
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new neon::NeonSidAudioProcessor();
}

