#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "ModulationTargets.h"
#include "SplitModules.h"
#include <neon_ui_components/modules/ControlModule.h>
#include <neon_ui_components/modules/FxModule.h>

namespace neon
{
    NeonSplitAudioProcessorEditor::NeonSplitAudioProcessorEditor (NeonSplitAudioProcessor& p)
        : AudioProcessorEditor (&p), audioProcessor (p)
    {
        setLookAndFeel (&lookAndFeel);

        auto theme = NeonRegistry::getTheme();

        selectionPanel.setCategoryNames ({ "BASS", "PAD", "ARP", "DRUM", "MAIN" });
        selectionPanel.setButtonColors (juce::Colour (0xFFFF00FF), juce::Colour (0xFF808080));
        selectionPanel.setModuleNames ({ "BASS", "PAD", "ARP", "DRUMS", "SPLIT", "LIB" });
        selectionPanel.setCategoryModules (0, { 0 });
        selectionPanel.setCategoryModules (1, { 1 });
        selectionPanel.setCategoryModules (2, { 2 });
        selectionPanel.setCategoryModules (3, { 3 });
        selectionPanel.setCategoryModules (4, { 4, 5 });
        selectionPanel.onModuleChanged = [this] (int index) { setActiveModule (index); };
        addAndMakeVisible (selectionPanel);

        auto bass = std::make_unique<BassModule> ("Bass", juce::Colour (0xFF0088FF));
        auto pad = std::make_unique<PadModule> ("Pad", juce::Colour (0xFF00CCAA));
        auto arp = std::make_unique<ArpSplitModule> ("Arp", juce::Colour (0xFFFFAA00));
        auto drums = std::make_unique<DrumModule> ("Drums", juce::Colour (0xFFFF4488));
        auto splitCtrl = std::make_unique<SplitControlModule> ("Split", theme.indicator);
        auto libModule = std::make_unique<LibrarianModule> ("Librarian", theme.background.brighter());

        modules.add (bass.release());
        modules.add (pad.release());
        modules.add (arp.release());
        modules.add (drums.release());
        modules.add (splitCtrl.release());
        modules.add (libModule.release());

        for (auto* m : modules)
            addChildComponent (m);

        setActiveModule (5);
        setSize (940, 840);
        startTimerHz (30);
    }

    NeonSplitAudioProcessorEditor::~NeonSplitAudioProcessorEditor()
    {
        stopTimer();
        setLookAndFeel (nullptr);
    }

    void NeonSplitAudioProcessorEditor::timerCallback()
    {
        bool midiIsActive = audioProcessor.midiActivity.exchange (false);
        int activeVoices = audioProcessor.getSignalPath().getActiveVoicesCount();

        for (auto* m : modules)
        {
            m->setMidiActive (midiIsActive);
            m->setVoiceCount (activeVoices);
        }
    }

    void NeonSplitAudioProcessorEditor::setActiveModule (int index)
    {
        if (index < 0 || index >= modules.size())
            return;

        for (int i = 0; i < modules.size(); ++i)
            modules[i]->setVisible (i == index);

        selectionPanel.selectModuleByIndex (index, false);
        resized();
    }

    void NeonSplitAudioProcessorEditor::paint (juce::Graphics& g)
    {
        g.fillAll (Colors::background);
    }

    void NeonSplitAudioProcessorEditor::resized()
    {
        auto bounds = getLocalBounds();
        float unitH = (float) bounds.getHeight() / 10.0f;

        bounds.removeFromTop ((int) unitH);
        bounds.removeFromBottom ((int) unitH);

        selectionPanel.setBounds (getLocalBounds());

        for (auto* m : modules)
        {
            if (m->isVisible())
                m->setBounds (bounds);
        }
    }
}
