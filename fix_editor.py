#!/usr/bin/env python3
import os

cpp_content = '''#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace neon
{
    NeonFMAudioProcessorEditor::NeonFMAudioProcessorEditor (NeonFMAudioProcessor& p)
        : AudioProcessorEditor (&p), audioProcessor (p)
    {
        setLookAndFeel (&lookAndFeel);

        // Create 8 top buttons (radio group 1) - blank, no labels
        for (int i = 0; i < 8; ++i)
        {
            auto btn = std::make_unique<juce::TextButton>();
            btn->setClickingTogglesState(true);
            btn->setRadioGroupId(1); // Group 1: Top buttons
            btn->setButtonText("");   // Blank button - no text
            
            // Set style to make it look like a button without label
            btn->setColour(juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
            btn->setColour(juce::TextButton::textColourOffId, juce::Colours::grey);
            btn->setColour(juce::TextButton::textColourOnId, juce::Colours::white);
            
            // Store pointer for later use
            topButtons[i] = std::move(btn);
            addAndMakeVisible(topButtons[i].get());
        }

        // Create 8 bottom buttons (radio group 2) - blank, no labels
        for (int i = 0; i < 8; ++i)
        {
            auto btn = std::make_unique<juce::TextButton>();
            btn->setClickingTogglesState(true);
            btn->setRadioGroupId(2); // Group 2: Bottom buttons (separate from top)
            btn->setButtonText("");   // Blank button - no text
            
            // Set style to make it look like a button without label
            btn->setColour(juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
            btn->setColour(juce::TextButton::textColourOffId, juce::Colours::grey);
            btn->setColour(juce::TextButton::textColourOnId, juce::Colours::white);
            
            // Store pointer for later use
            bottomButtons[i] = std::move(btn);
            addAndMakeVisible(bottomButtons[i].get());
        }

        // Select first top button by default
        if (topButtons.size() > 0)
            topButtons[0]->setToggleState(true, juce::dontSendNotification);

        // Select first bottom button by default
        if (bottomButtons.size() > 0)
            bottomButtons[0]->setToggleState(true, juce::dontSendNotification);

        setSize(940, 600);  // Smaller height since no display/parameter areas
        startTimerHz(30);
    }

    NeonFMAudioProcessorEditor::~NeonFMAudioProcessorEditor()
    {
        stopTimer();
        setLookAndFeel(nullptr);
    }

    void NeonFMAudioProcessorEditor::timerCallback()
    {
        // Placeholder for timer callback
    }

    void NeonFMAudioProcessorEditor::paint (juce::Graphics& g)
    {
        g.fillAll (juce::Colours::black);
    }

    void NeonFMAudioProcessorEditor::resized()
    {
        auto bounds = getLocalBounds();
        
        // Layout in 10ths:
        // Top 1/10: Category buttons (8 slots)
        // Middle 7/10: Blank display area
        // Bottom 1/10: Module buttons (8 slots)
        
        float unitH = (float)bounds.getHeight() / 10.0f;
        int slotWidth = bounds.getWidth() / 8;
        
        // Top row: Category buttons at y=0 (first 1/10)
        for (int i = 0; i < 8; ++i)
            topButtons[i]->setBounds(i * slotWidth, 0, slotWidth, (int)unitH);
        
        // Bottom row: Module buttons at y=9/10 (last 1/10)
        int bottomY = (int)(unitH * 9);
        for (int i = 0; i < 8; ++i)
            bottomButtons[i]->setBounds(i * slotWidth, bottomY, slotWidth, (int)unitH);
    }
}
'''

os.makedirs('neon-fm/source', exist_ok=True)
with open('neon-fm/source/PluginEditor.cpp', 'w') as f:
    f.write(cpp_content)

print("PluginEditor.cpp written successfully")