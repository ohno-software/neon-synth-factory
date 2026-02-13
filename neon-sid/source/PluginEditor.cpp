#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "SidModules.h"

namespace neon
{
    NeonSidAudioProcessorEditor::NeonSidAudioProcessorEditor (NeonSidAudioProcessor& p)
        : AudioProcessorEditor (&p), audioProcessor (p)
    {
        setLookAndFeel (&lookAndFeel);
        auto theme = NeonRegistry::getTheme();

        selectionPanel.setCategoryNames ({ "SID", "FILTER", "AMP", "M-FX", "MAIN" });
        selectionPanel.setButtonColors (juce::Colour (0xFF4040FF), juce::Colour (0xFF808080));
        selectionPanel.setModuleNames ({ "OSC 1", "OSC 2", "OSC 3", "FILTER", "AMP", "A-ENV", "LIB" });

        selectionPanel.setCategoryModules (0, { 0, 1, 2 });
        selectionPanel.setCategoryModules (1, { 3 });
        selectionPanel.setCategoryModules (2, { 4, 5 });
        selectionPanel.setCategoryModules (3, { });
        selectionPanel.setCategoryModules (4, { 6 });

        selectionPanel.onModuleChanged = [this] (int index) { setActiveModule (index); };
        addAndMakeVisible (selectionPanel);

        modules.add (std::make_unique<SidOscModule> ("Osc 1", theme.oscillator));
        modules.add (std::make_unique<SidOscModule> ("Osc 2", theme.oscillator));
        modules.add (std::make_unique<SidOscModule> ("Osc 3", theme.oscillator));
        modules.add (std::make_unique<LadderFilterModule> ("Filter", theme.filter));
        modules.add (std::make_unique<AmpModule> ("Amp Output", theme.amplifier));
        modules.add (std::make_unique<DahdsrModule> ("Amp Env", theme.envelope, false));
        modules.add (std::make_unique<LibrarianModule> ("Librarian", theme.background.brighter()));

        for (auto* m : modules)
            addChildComponent (m);

        setActiveModule (6);
        setSize (940, 840);
        startTimerHz (30);
    }

    NeonSidAudioProcessorEditor::~NeonSidAudioProcessorEditor()
    {
        stopTimer();
        setLookAndFeel (nullptr);
    }

    void NeonSidAudioProcessorEditor::setActiveModule (int index)
    {
        if (index < 0 || index >= modules.size())
            return;

        activeModuleIndex = index;
        for (int i = 0; i < modules.size(); ++i)
            modules[i]->setVisible (i == index);

        selectionPanel.selectModuleByIndex (index, false);
        resized();
    }

    void NeonSidAudioProcessorEditor::paint (juce::Graphics& g)
    {
        g.fillAll (NeonRegistry::getTheme().background);
    }

    void NeonSidAudioProcessorEditor::resized()
    {
        auto bounds = getLocalBounds();
        float unitH = (float) bounds.getHeight() / 10.0f;

        bounds.removeFromTop ((int) unitH);
        bounds.removeFromBottom ((int) unitH);

        selectionPanel.setBounds (getLocalBounds());

        if (activeModuleIndex >= 0 && activeModuleIndex < modules.size())
            modules[activeModuleIndex]->setBounds (bounds);
    }

    void NeonSidAudioProcessorEditor::timerCallback() {}
}
