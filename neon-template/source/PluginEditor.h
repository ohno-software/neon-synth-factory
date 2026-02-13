#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include <neon_ui_components/neon_ui_components.h>
#include "PluginProcessor.h"

namespace neon
{
    class NeonTemplateAudioProcessorEditor : public juce::AudioProcessorEditor, public juce::Timer
    {
    public:
        NeonTemplateAudioProcessorEditor(NeonTemplateAudioProcessor&);
        ~NeonTemplateAudioProcessorEditor() override;

        void paint(juce::Graphics&) override;
        void resized() override;
        void timerCallback() override {}

    private:
        NeonTemplateAudioProcessor& audioProcessor;
        LookAndFeel lookAndFeel;
        ModuleSelectionPanel selectionPanel;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NeonTemplateAudioProcessorEditor)
    };
}
