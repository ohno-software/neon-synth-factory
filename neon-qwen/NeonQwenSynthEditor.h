/*
  ==============================================================================

    NeonQwenSynthEditor.h
    Created: 31 Jan 2026 3:55:25pm
    Author:  Adrian

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "NeonQwenSynthProcessor.h"

//==============================================================================
/**
*/
class NeonQwenSynthEditor  : public juce::AudioProcessorEditor,
                             public juce::Slider::Listener,
                             public juce::Button::Listener
{
public:
    //==============================================================================
    NeonQwenSynthEditor (NeonQwenSynthProcessor&);
    ~NeonQwenSynthEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

    void sliderValueChanged(juce::Slider* slider) override;
    void buttonClicked(juce::Button* button) override;

private:
    //==============================================================================
    NeonQwenSynthProcessor& processor;
    
    // Sliders for parameters
    juce::Slider volumeSlider;
    juce::Label volumeLabel;
    
    juce::Slider attackSlider;
    juce::Label attackLabel;
    
    juce::Slider releaseSlider;
    juce::Label releaseLabel;
    
    // Waveform selection buttons
    juce::TextButton sineButton;
    juce::TextButton squareButton;
    juce::TextButton sawtoothButton;
    juce::TextButton triangleButton;
    
    juce::Label waveformLabel;
    
    // Background colour for the UI
    juce::Colour backgroundColor;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NeonQwenSynthEditor)
};