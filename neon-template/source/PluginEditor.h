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
        class TemplateSelectionPanel : public juce::Component
        {
        public:
            TemplateSelectionPanel()
            {
                // Top row: 8 category buttons (5 categories, 3 empty)
                for (int i = 0; i < 8; ++i)
                {
                    auto btn = std::make_unique<juce::TextButton>();
                    btn->setClickingTogglesState(true);
                    btn->setRadioGroupId(4001);

                    if (i < 5)
                    {
                        int catIndex = i;
                        btn->onClick = [this, catIndex]() {
                            setActiveCategory(catIndex);
                            updateButtonColors();
                        };
                        addAndMakeVisible(btn.get());
                    }
                    else
                    {
                        btn->setVisible(false); // Empty slots
                        addChildComponent(btn.get());
                    }

                    categoryButtons.add(std::move(btn));
                }

                // Bottom row: 8 module buttons (all empty)
                for (int i = 0; i < 8; ++i)
                {
                    auto btn = std::make_unique<juce::TextButton>();
                    btn->setClickingTogglesState(true);
                    btn->setRadioGroupId(4002);

                    // No module logic — just visual placeholders
                    btn->onClick = [this, i]() {
                        for (int j = 0; j < 8; ++j)
                            if (j != i) moduleButtons[j]->setToggleState(false, juce::dontSendNotification);
                        moduleButtons[i]->setToggleState(true, juce::dontSendNotification);
                        updateButtonColors();
                    };

                    addChildComponent(btn.get());
                    moduleButtons.add(std::move(btn));
                }

                // Set category button labels (matching neon-split)
                categoryButtons[0]->setButtonText("BASS");
                categoryButtons[1]->setButtonText("PAD");
                categoryButtons[2]->setButtonText("ARP");
                categoryButtons[3]->setButtonText("DRUM");
                categoryButtons[4]->setButtonText("MAIN");

                // Default to MAIN (index 4)
                setActiveCategory(4);
                categoryButtons[4]->setToggleState(true, juce::dontSendNotification);
                updateButtonColors();
            }

            void setActiveCategory(int categoryIndex)
            {
                if (categoryIndex < 0 || categoryIndex >= 5) return;
                currentCategory = categoryIndex;
                resized();
            }

            void updateButtonColors()
            {
                auto magenta = juce::Colour(0xFFFF00FF);
                auto grey = juce::Colour(0xFF808080);

                for (auto* btn : categoryButtons)
                {
                    if (btn->getToggleState())
                    {
                        btn->setColour(juce::TextButton::buttonColourId, juce::Colours::white.withAlpha(0.95f));
                        btn->setColour(juce::TextButton::textColourOnId, magenta);
                        btn->setColour(juce::TextButton::textColourOffId, magenta);
                    }
                    else
                    {
                        btn->setColour(juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
                        btn->setColour(juce::TextButton::textColourOnId, grey);
                        btn->setColour(juce::TextButton::textColourOffId, grey);
                    }
                    btn->repaint();
                }

                for (auto* btn : moduleButtons)
                {
                    if (btn->getToggleState())
                    {
                        btn->setColour(juce::TextButton::buttonColourId, juce::Colours::white.withAlpha(0.95f));
                        btn->setColour(juce::TextButton::textColourOnId, magenta);
                        btn->setColour(juce::TextButton::textColourOffId, magenta);
                    }
                    else
                    {
                        btn->setColour(juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
                        btn->setColour(juce::TextButton::textColourOnId, grey);
                        btn->setColour(juce::TextButton::textColourOffId, grey);
                    }
                    btn->repaint();
                }
            }

            void resized() override
            {
                auto bounds = getLocalBounds();
                float unitH = (float)bounds.getHeight() / 10.0f;
                int slotWidth = bounds.getWidth() / 8;

                // Top row: category buttons
                for (int i = 0; i < 8; ++i)
                    categoryButtons[i]->setBounds(i * slotWidth, 0, slotWidth, (int)unitH);

                // Bottom row: module buttons
                int bottomY = (int)(unitH * 9);
                for (int i = 0; i < 8; ++i)
                    moduleButtons[i]->setBounds(i * slotWidth, bottomY, slotWidth, (int)unitH);
            }

        private:
            juce::OwnedArray<juce::TextButton> categoryButtons;
            juce::OwnedArray<juce::TextButton> moduleButtons;
            int currentCategory = 0;
        };

        NeonTemplateAudioProcessor& audioProcessor;
        LookAndFeel lookAndFeel;
        std::unique_ptr<TemplateSelectionPanel> selectionPanelComponent;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NeonTemplateAudioProcessorEditor)
    };
}
