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
        NeonTemplateAudioProcessorEditor (NeonTemplateAudioProcessor&);
        ~NeonTemplateAudioProcessorEditor() override;

        void paint (juce::Graphics&) override;
        void resized() override;
        void timerCallback() override;

        void setActiveGroup (int groupIndex);
        void updateButtonColors();

    private:
        NeonTemplateAudioProcessor& audioProcessor;

        LookAndFeel lookAndFeel;

        // Top buttons (1-8)
        juce::OwnedArray<juce::TextButton> topButtons;
        
        // Bottom buttons (8 groups of 8 buttons each)
        juce::OwnedArray<juce::TextButton> bottomButtons;
        
        // Display area
        juce::Label moduleNameDisplay;
        juce::Label patchNameDisplay;
        juce::Label midiIndicator;
        juce::Label parameterNameDisplay;
        juce::Label parameterValueDisplay;
        
        // Parameter cards (2 rows of 4)
        std::vector<std::unique_ptr<ParameterCard>> parameterCards;
        std::vector<std::unique_ptr<ManagedParameter>> parameters;
        
        // Current state
        int currentGroup = 0;
        int activeBottomButton = -1;
        int currentParameterIndex = 0;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NeonTemplateAudioProcessorEditor)
    };
}
