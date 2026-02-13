#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "ModulationTargets.h"
#include <neon_ui_components/core/NeonPatchManager.h>

namespace neon
{
    NeonChipAudioProcessor::NeonChipAudioProcessor()
        : AudioProcessor (BusesProperties().withOutput ("Output", juce::AudioChannelSet::stereo(), true))
    {
        PatchManager::getInstance().initialize ("NeonChip");

        // Register modulation target / source names for UI components
        NeonRegistry::setTargetNames (getNeonChipModTargetNames());
        NeonRegistry::setSourceNames (getNeonChipCtrlSourceNames());
    }

    NeonChipAudioProcessor::~NeonChipAudioProcessor() = default;

    void NeonChipAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
    {
        signalPath.prepareToPlay (samplesPerBlock, sampleRate);
    }

    void NeonChipAudioProcessor::releaseResources()
    {
        signalPath.releaseResources();
    }

    void NeonChipAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
    {
        juce::ScopedNoDenormals noDenormals;
        auto totalNumInputChannels  = getTotalNumInputChannels();
        auto totalNumOutputChannels = getTotalNumOutputChannels();

        for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
            buffer.clear (i, 0, buffer.getNumSamples());

        keyboardState.processNextMidiBuffer (midiMessages, 0, buffer.getNumSamples(), true);

        // Get host BPM
        if (auto* ph = getPlayHead())
        {
            if (auto pos = ph->getPosition())
            {
                if (auto bpmValue = pos->getBpm())
                    signalPath.setBpm (*bpmValue);
            }
        }

        // Process MIDI
        if (!midiMessages.isEmpty())
            midiActivity = true;

        for (const auto metadata : midiMessages)
        {
            const auto msg = metadata.getMessage();
            if (msg.isNoteOn())
                signalPath.noteOn (msg.getNoteNumber(), msg.getFloatVelocity());
            else if (msg.isNoteOff())
                signalPath.noteOff (msg.getNoteNumber());
            else if (msg.isAllNotesOff() || msg.isAllSoundOff())
            {
                for (int i = 0; i < 128; ++i) signalPath.noteOff (i);
            }
            else if (msg.isPitchWheel())
                signalPath.setPitchWheel (juce::jmap ((float) msg.getPitchWheelValue(), 0.0f, 16383.0f, -1.0f, 1.0f));
            else if (msg.isAftertouch())
                signalPath.setPolyAftertouch (msg.getNoteNumber(), msg.getAfterTouchValue() / 127.0f);
            else if (msg.isChannelPressure())
                signalPath.setChannelAftertouch (msg.getChannelPressureValue() / 127.0f);
            else if (msg.isController())
            {
                if (msg.getControllerNumber() == 1)
                    signalPath.setModWheel (msg.getControllerValue() / 127.0f);
                else if (msg.getControllerNumber() == 0 || msg.getControllerNumber() == 32)
                    PatchManager::getInstance().selectBank (msg.getControllerValue());
            }
            else if (msg.isProgramChange())
            {
                PatchManager::getInstance().loadPatch (msg.getProgramChangeNumber());
            }
        }

        juce::AudioSourceChannelInfo info (&buffer, 0, buffer.getNumSamples());
        signalPath.getNextAudioBlock (info);
    }

    juce::AudioProcessorEditor* NeonChipAudioProcessor::createEditor()
    {
        return new NeonChipAudioProcessorEditor (*this);
    }

} // namespace neon

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new neon::NeonChipAudioProcessor();
}
