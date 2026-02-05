# Script to create neon-fm plugin files without auto-formatting issues

import os

os.makedirs('neon-fm/source', exist_ok=True)

# FMProcessor.h content
h_content = """#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include "FMSynth.h"

namespace fm {

class FMProcessorEditor;

class FMProcessor : public juce::AudioProcessor {
public:
    FMProcessor();
    ~FMProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "Neon FM"; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    FMSynth& getSynth() { return synth; }

private:
    FMSynth synth;
    juce::MidiKeyboardState keyboardState;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FMProcessor)
};

} // namespace fm
"""

with open('neon-fm/source/FMProcessor.h', 'w') as f:
    f.write(h_content)

print('FMProcessor.h created successfully')

# FMProcessor.cpp content  
cpp_content = """#include "FMProcessor.h"
#include <juce_core/juce_core.h>

namespace fm {

FMProcessor::FMProcessor()
    : AudioProcessor(BusesProperties().withOutput("Output", juce::AudioChannelSet::stereo(), true))
{
}

FMProcessor::~FMProcessor() = default;

void FMProcessor::prepareToPlay(double sampleRate, int samplesPerBlock) {
    synth.prepareToPlay(sampleRate, samplesPerBlock);
}

void FMProcessor::releaseResources() {
    // Nothing to release
}

void FMProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi) {
    juce::ScopedNoDenormals noDenormals;
    
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    // Process MIDI
    keyboardState.processNextMidiBuffer(midi, 0, buffer.getNumSamples(), true);

    if (!midi.isEmpty()) {
        for (const auto metadata : midi) {
            const auto msg = metadata.getMessage();
            if (msg.isNoteOn())
                synth.noteOn(msg.getNoteNumber(), msg.getFloatVelocity());
            else if (msg.isNoteOff())
                synth.noteOff(msg.getFloatVelocity());
        }
    }

    synth.renderNextBlock(buffer, 0, buffer.getNumSamples());
}

juce::AudioProcessorEditor* FMProcessor::createEditor() {
    return new FMProcessorEditor(*this);
}

void FMProcessor::getStateInformation(juce::MemoryBlock& destData) override {
    juce::XmlElement xml("NeonFmState");
    
    for (int i = 0; i < 6; ++i) {
        xml.setAttribute("op" + juce::String(i) + "_ratio", synth.getOperatorRatio(i));
        xml.setAttribute("op" + juce::String(i) + "_level", synth.getOperatorLevel(i));
        xml.setAttribute("op" + juce::String(i) + "_feedback", synth.getOperatorFeedback(i));
    }
    
    xml.setAttribute("output_gain", synth.getOutputGain());
    
    copyXmlToBinary(xml, destData);
}

void FMProcessor::setStateInformation(const void* data, int sizeInBytes) override {
    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, sizeInBytes));
    
    if (xml && xml->hasTagName("NeonFmState")) {
        for (int i = 0; i < 6; ++i) {
            synth.setOperatorRatio(i, (float)xml->getDoubleAttribute("op" + juce::String(i) + "_ratio", 1.0));
            synth.setOperatorLevel(i, (float)xml->getDoubleAttribute("op" + juce::String(i) + "_level", 0.5));
            synth.setOperatorFeedback(i, (float)xml->getDoubleAttribute("op" + juce::String(i) + "_feedback", 0.0));
        }
        
        synth.setOutputGain((float)xml->getDoubleAttribute("output_gain", 1.0));
    }
}

} // namespace fm

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new fm::FMProcessor();
}
"""

with open('neon-fm/source/FMProcessor.cpp', 'w') as f:
    f.write(cpp_content)

print('FMProcessor.cpp created successfully')

# PluginEditor.h content
editor_h = """#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <neon_ui_components/neon_ui_components.h>
#include "FMProcessor.h"

namespace fm {

class FMProcessorEditor : public juce::AudioProcessorEditor, public juce::Timer {
public:
    FMProcessorEditor(FMProcessor&);
    ~FMProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;

private:
    FMProcessor& audioProcessor;

    neon::LookAndFeel lookAndFeel;
    
    std::vector<std::unique_ptr<neon::ModuleBase>> modules;
    neon::ModuleSelectionPanel selectionPanel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FMProcessorEditor)
};

} // namespace fm
"""

with open('neon-fm/source/PluginEditor.h', 'w') as f:
    f.write(editor_h)

print('PluginEditor.h created successfully')

# PluginEditor.cpp content  
editor_cpp = """#include "PluginEditor.h"
#include <neon_ui_components/modules/LfoModule.h>
#include <neon_ui_components/modules/OscillatorModule.h>
#include <neon_ui_components/modules/DahdsrModule.h>
#include <neon_ui_components/modules/AmpModule.h>

namespace fm {

FMProcessorEditor::FMProcessorEditor(FMProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    setLookAndFeel(&lookAndFeel);

    auto theme = neon::NeonRegistry::getTheme();

    addAndMakeVisible(selectionPanel);
    
    // 6 Operator Modules (OscillatorModule can be used as a base for operators)
    for (int i = 1; i <= 6; ++i) {
        auto opModule = std::make_unique<neon::OscillatorModule>("Operator " + juce::String(i), theme.oscillator);
        modules.push_back(std::move(opModule));
        addChildComponent(modules.back().get());
    }
    
    // Output Module
    auto outputModule = std::make_unique<neon::AmpModule>("Output", theme.amplifier);
    modules.push_back(std::move(outputModule));
    addChildComponent(modules.back().get());

    for (auto* m : modules)
        m->setVisible(false);
        
    if (!modules.empty())
        modules[0]->setVisible(true);

    selectionPanel.onModuleChanged = [this](int index) {
        for (size_t i = 0; i < modules.size(); ++i)
            modules[i]->setVisible(i == index);
        resized();
    };
    
    // Show first operator by default
    selectionPanel.selectModuleByIndex(0, false);

    setSize(940, 840);
    startTimerHz(30);
}

FMProcessorEditor::~FMProcessorEditor() {
    stopTimer();
    setLookAndFeel(nullptr);
}

void FMProcessorEditor::timerCallback() {
    // Update UI with current state
    bool midiIsActive = audioProcessor.keyboardState.getMidiKeyStates().anyKeysPressed();
    
    for (auto* m : modules) {
        m->setMidiActive(midiIsActive);
    }
}

void FMProcessorEditor::paint(juce::Graphics& g) {
    g.fillAll(neon::Colors::background);
}

void FMProcessorEditor::resized() {
    auto bounds = getLocalBounds();
    
    // Layout in 1/10ths:
    float unitH = (float)bounds.getHeight() / 10.0f;
    
    // Top 1/10: Category navigation
    auto topNav = bounds.removeFromTop((int)unitH);
    
    // Bottom 1/10: Module navigation  
    auto bottomNav = bounds.removeFromBottom((int)unitH);
    
    // Selection panel gets full height
    selectionPanel.setBounds(0, 0, getWidth(), getHeight());
    
    // Active module
    for (auto* m : modules) {
        if (m->isVisible()) {
            m->setBounds(bounds);
        }
    }
}

} // namespace fm

juce::AudioProcessorEditor* FMProcessor::createEditor() {
    return new fm::FMProcessorEditor(*this);
}
"""

with open('neon-fm/source/PluginEditor.cpp', 'w') as f:
    f.write(editor_cpp)

print('PluginEditor.h and PluginEditor.cpp created successfully')
print('All neon-fm files created!')