#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace neon
{
    NeonTemplateAudioProcessorEditor::NeonTemplateAudioProcessorEditor(NeonTemplateAudioProcessor& p)
        : AudioProcessorEditor(&p), audioProcessor(p)
    {
        setLookAndFeel(&lookAndFeel);

        // Create and attach the selection panel
        selectionPanelComponent = std::make_unique<TemplateSelectionPanel>();
        addAndMakeVisible(selectionPanelComponent.get());

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
        selectionPanelComponent->setBounds(bounds);
    }
}
