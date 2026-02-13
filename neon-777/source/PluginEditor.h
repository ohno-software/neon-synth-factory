#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include <neon_ui_components/neon_ui_components.h>
#include "PluginProcessor.h"

namespace neon
{
    class Neon777AudioProcessorEditor : public juce::AudioProcessorEditor, public juce::Timer
    {
    public:
        Neon777AudioProcessorEditor (Neon777AudioProcessor&);
        ~Neon777AudioProcessorEditor() override;

        void paint (juce::Graphics&) override;
        void resized() override;
        void timerCallback() override;

        void setActiveModule (int index);

    private:
        Neon777AudioProcessor& audioProcessor;

        LookAndFeel lookAndFeel;
        
        NeonDebugPanel debugPanel;
        ModuleSelectionPanel selectionPanel;
        juce::OwnedArray<ModuleBase> modules;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Neon777AudioProcessorEditor)
    };
}
