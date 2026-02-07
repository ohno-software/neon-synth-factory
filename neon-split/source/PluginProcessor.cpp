#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "ModulationTargets.h"
#include <neon_ui_components/core/NeonPatchManager.h>

namespace neon
{
    NeonSplitAudioProcessor::NeonSplitAudioProcessor()
        : AudioProcessor (BusesProperties().withOutput ("Output", juce::AudioChannelSet::stereo(), true))
    {
        PatchManager::getInstance().initialize ("NeonSplit");

        // Initialize modulation names for the UI components
        NeonRegistry::setTargetNames (getNeonSplitModTargetNames());
        NeonRegistry::setSourceNames (getNeonSplitCtrlSourceNames());
    }

    NeonSplitAudioProcessor::~NeonSplitAudioProcessor()
    {
    }

    void NeonSplitAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
    {
        signalPath.prepareToPlay (sampleRate, samplesPerBlock);
    }

    void NeonSplitAudioProcessor::releaseResources()
    {
        signalPath.releaseResources();
    }

    void NeonSplitAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
    {
        juce::ScopedNoDenormals noDenormals;

        for (auto i = getTotalNumInputChannels(); i < getTotalNumOutputChannels(); ++i)
            buffer.clear (i, 0, buffer.getNumSamples());

        // Inject on-screen keyboard events into the MIDI buffer
        keyboardState.processNextMidiBuffer (midiMessages, 0, buffer.getNumSamples(), true);

        // Track MIDI activity for UI
        if (!midiMessages.isEmpty())
            midiActivity = true;

        // Sync transport from host
        if (auto* ph = getPlayHead())
        {
            if (auto pos = ph->getPosition())
            {
                if (auto bpmValue = pos->getBpm())
                    signalPath.setBpm (*bpmValue);
                if (pos->getPpqPosition().hasValue())
                    signalPath.setPpqPosition (*pos->getPpqPosition());
                signalPath.setIsPlaying (pos->getIsPlaying());
            }
        }

        // Handle bank/program changes for patch manager
        for (const auto metadata : midiMessages)
        {
            auto msg = metadata.getMessage();
            if (msg.isController())
            {
                if (msg.getControllerNumber() == 0 || msg.getControllerNumber() == 32)
                    PatchManager::getInstance().selectBank (msg.getControllerValue());
            }
            else if (msg.isProgramChange())
            {
                PatchManager::getInstance().loadPatch (msg.getProgramChangeNumber());
            }
        }

        // Process audio: MIDI splitting + engine rendering happens inside
        signalPath.processBlock (buffer, midiMessages);
    }

    juce::AudioProcessorEditor* NeonSplitAudioProcessor::createEditor()
    {
        return new NeonSplitAudioProcessorEditor (*this);
    }

} // namespace neon

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new neon::NeonSplitAudioProcessor();
}
