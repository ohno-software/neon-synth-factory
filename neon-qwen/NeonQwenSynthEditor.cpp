/*
  ==============================================================================

    NeonQwenSynthEditor.cpp
    Created: 31 Jan 2026 3:55:25pm
    Author:  Adrian

  ==============================================================================
*/

#include "NeonQwenSynthEditor.h"

//==============================================================================
NeonQwenSynthEditor::NeonQwenSynthEditor (NeonQwenSynthProcessor& p)
    : AudioProcessorEditor (&p),
      processor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (400, 300);
    
    // Set background color
    backgroundColor = juce::Colour(0xFF1a1a1a); // Dark gray background
    
    // Volume slider
    addAndMakeVisible(volumeSlider);
    volumeSlider.setRange(0.0, 1.0, 0.01);
    volumeSlider.setValue(p.getVolume());
    volumeSlider.addListener(this);
    volumeSlider.setSliderStyle(juce::Slider::LinearVertical);
    volumeSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 80, 20);
    
    addAndMakeVisible(volumeLabel);
    volumeLabel.setText("Volume", juce::dontSendNotification);
    volumeLabel.attachToComponent(&volumeSlider, false);
    volumeLabel.setJustificationType(juce::Justification::centred);
    
    // Attack slider
    addAndMakeVisible(attackSlider);
    attackSlider.setRange(0.0, 1.0, 0.01);
    attackSlider.setValue(p.getAttack());
    attackSlider.addListener(this);
    attackSlider.setSliderStyle(juce::Slider::LinearVertical);
    attackSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 80, 20);
    
    addAndMakeVisible(attackLabel);
    attackLabel.setText("Attack", juce::dontSendNotification);
    attackLabel.attachToComponent(&attackSlider, false);
    attackLabel.setJustificationType(juce::Justification::centred);
    
    // Release slider
    addAndMakeVisible(releaseSlider);
    releaseSlider.setRange(0.0, 1.0, 0.01);
    releaseSlider.setValue(p.getRelease());
    releaseSlider.addListener(this);
    releaseSlider.setSliderStyle(juce::Slider::LinearVertical);
    releaseSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 80, 20);
    
    addAndMakeVisible(releaseLabel);
    releaseLabel.setText("Release", juce::dontSendNotification);
    releaseLabel.attachToComponent(&releaseSlider, false);
    releaseLabel.setJustificationType(juce::Justification::centred);
    
    // Waveform buttons
    addAndMakeVisible(sineButton);
    sineButton.setButtonText("Sine");
    sineButton.addListener(this);
    sineButton.setToggleState(true, juce::dontSendNotification);
    
    addAndMakeVisible(squareButton);
    squareButton.setButtonText("Square");
    squareButton.addListener(this);
    
    addAndMakeVisible(sawtoothButton);
    sawtoothButton.setButtonText("Sawtooth");
    sawtoothButton.addListener(this);
    
    addAndMakeVisible(triangleButton);
    triangleButton.setButtonText("Triangle");
    triangleButton.addListener(this);
    
    addAndMakeVisible(waveformLabel);
    waveformLabel.setText("Waveform", juce::dontSendNotification);
    waveformLabel.setJustificationType(juce::Justification::centred);
}

NeonQwenSynthEditor::~NeonQwenSynthEditor()
{
}

//==============================================================================
void NeonQwenSynthEditor::paint (juce::Graphics& g)
{
    // Fill background
    g.fillAll(backgroundColor);
    
    // Draw title
    g.setColour(juce::Colours::white);
    g.setFont(16.0f);
    g.drawText("Neon Qwen Synth", getLocalBounds(), juce::Justification::centredTop, true);
}

void NeonQwenSynthEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // controls, as well as their relative sizes.
    
    auto area = getLocalBounds();
    
    // Title at the top
    auto titleArea = area.removeFromTop(30);
    
    // Create a margin around the controls
    area.reduce(10, 10);
    
    // Arrange controls in columns
    int columnWidth = area.getWidth() / 3;
    int controlHeight = 200;
    
    // Volume control (left column)
    auto volumeArea = area.removeFromLeft(columnWidth);
    volumeArea.removeFromTop(30); // Space for label
    volumeArea.setHeight(controlHeight);
    volumeSlider.setBounds(volumeArea);
    
    // Attack control (middle column)
    auto attackArea = area.removeFromLeft(columnWidth);
    attackArea.removeFromTop(30); // Space for label
    attackArea.setHeight(controlHeight);
    attackSlider.setBounds(attackArea);
    
    // Release control (right column)
    auto releaseArea = area.removeFromLeft(columnWidth);
    releaseArea.removeFromTop(30); // Space for label
    releaseArea.setHeight(controlHeight);
    releaseSlider.setBounds(releaseArea);
    
    // Waveform buttons at the bottom
    auto buttonArea = area.removeFromBottom(40);
    int buttonWidth = buttonArea.getWidth() / 4;
    
    sineButton.setBounds(buttonArea.removeFromLeft(buttonWidth));
    squareButton.setBounds(buttonArea.removeFromLeft(buttonWidth));
    sawtoothButton.setBounds(buttonArea.removeFromLeft(buttonWidth));
    triangleButton.setBounds(buttonArea);
}

void NeonQwenSynthEditor::sliderValueChanged(juce::Slider* slider)
{
    if (slider == &volumeSlider)
    {
        processor.setVolume((float)volumeSlider.getValue());
    }
    else if (slider == &attackSlider)
    {
        processor.setAttack((float)attackSlider.getValue());
    }
    else if (slider == &releaseSlider)
    {
        processor.setRelease((float)releaseSlider.getValue());
    }
}

void NeonQwenSynthEditor::buttonClicked(juce::Button* button)
{
    // Handle waveform selection buttons
    if (button == &sineButton)
    {
        sineButton.setToggleState(true, juce::dontSendNotification);
        squareButton.setToggleState(false, juce::dontSendNotification);
        sawtoothButton.setToggleState(false, juce::dontSendNotification);
        triangleButton.setToggleState(false, juce::dontSendNotification);
        // Note: In a real implementation, we would need to pass this to the voice
    }
    else if (button == &squareButton)
    {
        sineButton.setToggleState(false, juce::dontSendNotification);
        squareButton.setToggleState(true, juce::dontSendNotification);
        sawtoothButton.setToggleState(false, juce::dontSendNotification);
        triangleButton.setToggleState(false, juce::dontSendNotification);
    }
    else if (button == &sawtoothButton)
    {
        sineButton.setToggleState(false, juce::dontSendNotification);
        squareButton.setToggleState(false, juce::dontSendNotification);
        sawtoothButton.setToggleState(true, juce::dontSendNotification);
        triangleButton.setToggleState(false, juce::dontSendNotification);
    }
    else if (button == &triangleButton)
    {
        sineButton.setToggleState(false, juce::dontSendNotification);
        squareButton.setToggleState(false, juce::dontSendNotification);
        sawtoothButton.setToggleState(false, juce::dontSendNotification);
        triangleButton.setToggleState(true, juce::dontSendNotification);
    }
}
