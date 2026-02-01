#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "ModulationTargets.h"
#include <neon_ui_components/core/NeonPatchManager.h>

namespace neon
{
    NeonJrAudioProcessor::NeonJrAudioProcessor()
        : AudioProcessor (BusesProperties().withOutput ("Output", juce::AudioChannelSet::stereo(), true))
    {
        PatchManager::getInstance().initialize ("NeonJr");

        // Initialize modulation names for the UI components
        NeonRegistry::setTargetNames (getNeonJrModTargetNames());
        NeonRegistry::setSourceNames (getNeonJrCtrlSourceNames());
    }

    NeonJrAudioProcessor::~NeonJrAudioProcessor()
    {
    }

    void NeonJrAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
    {
        signalPath.prepareToPlay (samplesPerBlock, sampleRate);
    }

    void NeonJrAudioProcessor::releaseResources()
    {
        signalPath.releaseResources();
    }

    void NeonJrAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
    {
        juce::ScopedNoDenormals noDenormals;
        auto totalNumInputChannels  = getTotalNumInputChannels();
        auto totalNumOutputChannels = getTotalNumOutputChannels();

        for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
            buffer.clear (i, 0, buffer.getNumSamples());

        // Sync keyboard state to midi buffer
        keyboardState.processNextMidiBuffer (midiMessages, 0, buffer.getNumSamples(), true);

        // Get Playhead info for LFO sync
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
            else if (msg.isAllNotesOff())
            {
                for (int i = 0; i < 128; ++i) signalPath.noteOff(i);
            }
            else if (msg.isAllSoundOff())
            {
                for (int i = 0; i < 128; ++i) signalPath.noteOff(i);
            }
            else if (msg.isPitchWheel())
                signalPath.setPitchWheel (juce::jmap ((float)msg.getPitchWheelValue(), 0.0f, 16383.0f, -1.0f, 1.0f));
            else if (msg.isAftertouch())
                signalPath.setAftertouch (msg.getAfterTouchValue() / 127.0f);
            else if (msg.isChannelPressure())
                signalPath.setAftertouch (msg.getChannelPressureValue() / 127.0f);
            else if (msg.isController())
            {
                if (msg.getControllerNumber() == 1) // Mod Wheel
                    signalPath.setModWheel (msg.getControllerValue() / 127.0f);
                else if (msg.getControllerNumber() == 0 || msg.getControllerNumber() == 32)
                {
                    // Bank Select (CC 0 and 32 are standard for bank MSB/LSB)
                    // We'll treat them as a combined bank index if needed, 
                    // but for now let's just use the value.
                    PatchManager::getInstance().selectBank (msg.getControllerValue());
                }
            }
            else if (msg.isProgramChange())
            {
                PatchManager::getInstance().loadPatch (msg.getProgramChangeNumber());
            }
        }

        juce::AudioSourceChannelInfo info (&buffer, 0, buffer.getNumSamples());
        signalPath.getNextAudioBlock (info);
    }

    juce::AudioProcessorEditor* NeonJrAudioProcessor::createEditor()
    {
        return new NeonJrAudioProcessorEditor (*this);
    }
} // namespace neon

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new neon::NeonJrAudioProcessor();
}
