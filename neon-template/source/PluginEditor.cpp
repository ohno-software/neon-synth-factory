#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace neon
{
    NeonTemplateAudioProcessorEditor::NeonTemplateAudioProcessorEditor (NeonTemplateAudioProcessor& p)
        : AudioProcessorEditor (&p), audioProcessor (p)
    {
        setLookAndFeel (&lookAndFeel);

        auto theme = NeonRegistry::getTheme();

        // Create 8 parameters for the 2 rows (parameter1-parameter8)
        for (int i = 0; i < 8; ++i)
        {
            auto param = std::make_unique<ManagedParameter> (
                "parameter" + juce::String (i + 1),
                1.0f, 127.0f, 64.0f, false, true
            );
            parameters.push_back (std::move (param));
        }

        // Create 8 parameter cards (2 rows of 4)
        for (int i = 0; i < 8; ++i)
        {
            auto card = std::make_unique<ParameterCard> (
                *parameters[i],
                theme.oscillator
            );
            
            // Set up callback to update display when parameter is adjusted
            int paramIndex = i;
            card->onValueChanged = [this, paramIndex] (float value) {
                currentParameterIndex = paramIndex;
                parameterNameDisplay.setText (parameters[paramIndex]->getName(), juce::dontSendNotification);
            };
            
            parameterCards.push_back (std::move (card));
            addAndMakeVisible (parameterCards.back().get());
        }

        // Create 8 top buttons (numbered 1-8)
        for (int i = 0; i < 8; ++i)
        {
            auto button = new juce::TextButton (juce::String (i + 1));
            button->setClickingTogglesState (true);
            button->setRadioGroupId (1001);
            
            button->onClick = [this, i]() { setActiveGroup (i); };
            
            topButtons.add (button);
            addAndMakeVisible (button);
        }

        // Create 64 bottom buttons (8 groups of 8)
        for (int group = 0; group < 8; ++group)
        {
            for (int btn = 0; btn < 8; ++btn)
            {
                char letter = 'a' + btn;
                auto buttonName = juce::String (group + 1) + juce::String::charToString (letter);
                
                auto button = new juce::TextButton (buttonName);
                button->setClickingTogglesState (true);
                
                int buttonIndex = group * 8 + btn;
                button->onClick = [this, buttonIndex, buttonName]() {
                    activeBottomButton = buttonIndex;
                    moduleNameDisplay.setText (buttonName, juce::dontSendNotification);
                    updateButtonColors();
                };
                
                bottomButtons.add (button);
                addChildComponent (button); // Initially hidden
            }
        }

        // Setup display labels
        moduleNameDisplay.setText ("1a", juce::dontSendNotification);
        moduleNameDisplay.setJustificationType (juce::Justification::centredLeft);
        moduleNameDisplay.setColour (juce::Label::textColourId, Colors::textBright);
        moduleNameDisplay.setFont (juce::FontOptions (18.0f).withStyle ("Bold"));
        addAndMakeVisible (moduleNameDisplay);

        patchNameDisplay.setText ("Default", juce::dontSendNotification);
        patchNameDisplay.setJustificationType (juce::Justification::centred);
        patchNameDisplay.setColour (juce::Label::textColourId, Colors::textBright);
        patchNameDisplay.setFont (juce::FontOptions (16.0f));
        addAndMakeVisible (patchNameDisplay);

        midiIndicator.setText ("MIDI", juce::dontSendNotification);
        midiIndicator.setJustificationType (juce::Justification::centredRight);
        midiIndicator.setColour (juce::Label::textColourId, Colors::textDim);
        midiIndicator.setFont (juce::FontOptions (14.0f));
        addAndMakeVisible (midiIndicator);

        parameterNameDisplay.setText ("parameter1", juce::dontSendNotification);
        parameterNameDisplay.setJustificationType (juce::Justification::centredLeft);
        parameterNameDisplay.setColour (juce::Label::textColourId, Colors::textBright);
        parameterNameDisplay.setFont (juce::FontOptions (14.0f));
        addAndMakeVisible (parameterNameDisplay);

        parameterValueDisplay.setText ("64", juce::dontSendNotification);
        parameterValueDisplay.setJustificationType (juce::Justification::centredRight);
        parameterValueDisplay.setColour (juce::Label::textColourId, Colors::textBright);
        parameterValueDisplay.setFont (juce::FontOptions (16.0f).withStyle ("Bold"));
        addAndMakeVisible (parameterValueDisplay);

        // Set default group
        setActiveGroup (0);
        topButtons[0]->setToggleState (true, juce::dontSendNotification);
        updateButtonColors();

        setSize (940, 840);
        startTimerHz (30);
    }

    NeonTemplateAudioProcessorEditor::~NeonTemplateAudioProcessorEditor()
    {
        stopTimer();
        setLookAndFeel (nullptr);
    }

    void NeonTemplateAudioProcessorEditor::timerCallback()
    {
        bool midiIsActive = audioProcessor.midiActivity.exchange (false);
        midiIndicator.setColour (juce::Label::textColourId, 
                                midiIsActive ? Colors::textBright : Colors::textDim);
        
        // Update parameter value display for currently selected parameter
        if (currentParameterIndex >= 0 && currentParameterIndex < parameters.size())
        {
            auto& param = parameters[currentParameterIndex];
            parameterNameDisplay.setText (param->getName(), juce::dontSendNotification);
            parameterValueDisplay.setText (juce::String ((int) param->getValue()), juce::dontSendNotification);
        }
    }

    void NeonTemplateAudioProcessorEditor::setActiveGroup (int groupIndex)
    {
        if (groupIndex < 0 || groupIndex >= 8) return;

        currentGroup = groupIndex;

        // Hide all bottom buttons
        for (auto* btn : bottomButtons)
            btn->setVisible (false);

        // Show buttons for current group
        for (int i = 0; i < 8; ++i)
        {
            int buttonIndex = currentGroup * 8 + i;
            bottomButtons[buttonIndex]->setVisible (true);
        }

        // Update module name display to first button of group
        char letter = 'a';
        auto moduleName = juce::String (groupIndex + 1) + juce::String::charToString (letter);
        moduleNameDisplay.setText (moduleName, juce::dontSendNotification);
        activeBottomButton = currentGroup * 8; // First button of group

        updateButtonColors();
        resized();
    }

    void NeonTemplateAudioProcessorEditor::updateButtonColors()
    {
        auto magenta = juce::Colour (0xFFFF00FF); // Bright magenta
        auto grey = juce::Colour (0xFF808080); // Grey for unselected
        
        // Update top button colors
        for (auto* btn : topButtons)
        {
            if (btn->getToggleState())
            {
                btn->setColour (juce::TextButton::buttonColourId, juce::Colours::white.withAlpha (0.95f));
                btn->setColour (juce::TextButton::textColourOnId, magenta);
                btn->setColour (juce::TextButton::textColourOffId, magenta);
            }
            else
            {
                btn->setColour (juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
                btn->setColour (juce::TextButton::textColourOnId, grey);
                btn->setColour (juce::TextButton::textColourOffId, grey);
            }
            btn->repaint();
        }
        
        // Update bottom button colors
        for (int i = 0; i < bottomButtons.size(); ++i)
        {
            auto* btn = bottomButtons[i];
            if (i == activeBottomButton)
            {
                btn->setColour (juce::TextButton::buttonColourId, juce::Colours::white.withAlpha (0.95f));
                btn->setColour (juce::TextButton::textColourOnId, magenta);
                btn->setColour (juce::TextButton::textColourOffId, magenta);
            }
            else
            {
                btn->setColour (juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
                btn->setColour (juce::TextButton::textColourOnId, grey);
                btn->setColour (juce::TextButton::textColourOffId, grey);
            }
            btn->repaint();
        }
    }

    void NeonTemplateAudioProcessorEditor::paint (juce::Graphics& g)
    {
        g.fillAll (Colors::background);
    }

    void NeonTemplateAudioProcessorEditor::resized()
    {
        auto bounds = getLocalBounds();
        
        // Use same layout as neon-jr: divide height into 1/10ths
        float unitH = (float)bounds.getHeight() / 10.0f;
        int slotWidth = bounds.getWidth() / 8;
        
        // Top buttons (first 1/10 of height)
        for (int i = 0; i < 8; ++i)
        {
            topButtons[i]->setBounds (i * slotWidth, 0, slotWidth, (int)unitH);
        }
        
        // Display area (5/10 of height, starting at 1/10)
        auto displayArea = juce::Rectangle<int> (0, (int)unitH, getWidth(), (int)(unitH * 5));
        
        // Top line of display: Module name, Patch name, MIDI indicator
        auto displayTopLine = displayArea.removeFromTop (30).reduced (10, 5);
        moduleNameDisplay.setBounds (displayTopLine.removeFromLeft (150));
        midiIndicator.setBounds (displayTopLine.removeFromRight (100));
        patchNameDisplay.setBounds (displayTopLine);

        // Bottom line of display: Parameter name, Value
        auto displayBottomLine = displayArea.removeFromBottom (30).reduced (10, 5);
        parameterNameDisplay.setBounds (displayBottomLine.removeFromLeft (displayBottomLine.getWidth() / 2));
        parameterValueDisplay.setBounds (displayBottomLine);

        // Two rows of parameters (2/10 of height total, at 6/10 and 7/10)
        int paramRowHeight = (int)unitH;
        int paramY = (int)(unitH * 6);
        int cardWidth = getWidth() / 4;
        
        // First row (parameters 1-4) at 6/10
        for (int i = 0; i < 4; ++i)
        {
            parameterCards[i]->setBounds (i * cardWidth, paramY, cardWidth, paramRowHeight);
        }

        // Second row (parameters 5-8) at 7/10
        paramY = (int)(unitH * 7);
        for (int i = 4; i < 8; ++i)
        {
            parameterCards[i]->setBounds ((i - 4) * cardWidth, paramY, cardWidth, paramRowHeight);
        }

        // Bottom buttons (last 1/10 of height, at 9/10)
        int bottomY = (int)(unitH * 9);
        for (int i = 0; i < 8; ++i)
        {
            int buttonIndex = currentGroup * 8 + i;
            if (buttonIndex < bottomButtons.size())
            {
                bottomButtons[buttonIndex]->setBounds (i * slotWidth, bottomY, slotWidth, (int)unitH);
            }
        }
    }
}
