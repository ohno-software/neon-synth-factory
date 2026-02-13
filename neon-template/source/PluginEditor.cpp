#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace neon
{
    NeonTemplateAudioProcessorEditor::NeonTemplateAudioProcessorEditor(NeonTemplateAudioProcessor& p)
        : AudioProcessorEditor(&p), audioProcessor(p)
    {
        setLookAndFeel(&lookAndFeel);

        selectionPanel.setCategoryNames ({ "BASS", "PAD", "ARP", "DRUM", "MAIN" });
        selectionPanel.setButtonColors (juce::Colour (0xFFFF00FF), juce::Colour (0xFF808080));

        std::vector<juce::String> moduleNames;
        moduleNames.reserve (40);
        for (int group = 1; group <= 5; ++group)
        {
            for (char suffix = 'a'; suffix <= 'h'; ++suffix)
                moduleNames.push_back (juce::String (group) + juce::String::charToString (suffix));
        }
        selectionPanel.setModuleNames (moduleNames);
        selectionPanel.setCategoryModules (0, { 0, 1, 2, 3, 4, 5, 6, 7 });
        selectionPanel.setCategoryModules (1, { 8, 9, 10, 11, 12, 13, 14, 15 });
        selectionPanel.setCategoryModules (2, { 16, 17, 18, 19, 20, 21, 22, 23 });
        selectionPanel.setCategoryModules (3, { 24, 25, 26, 27, 28, 29, 30, 31 });
        selectionPanel.setCategoryModules (4, { 32, 33, 34, 35, 36, 37, 38, 39 });
        selectionPanel.setActiveCategory (4);

        addAndMakeVisible (selectionPanel);

        setSize(940, 840);
    }

    NeonTemplateAudioProcessorEditor::~NeonTemplateAudioProcessorEditor()
    {
        stopTimer();
        setLookAndFeel(nullptr);
    }

    void NeonTemplateAudioProcessorEditor::paint(juce::Graphics& g)
    {
        g.fillAll(Colors::background);
    }

    void NeonTemplateAudioProcessorEditor::resized()
    {
        auto bounds = getLocalBounds();
        selectionPanel.setBounds(bounds);
    }
}
