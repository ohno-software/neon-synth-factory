#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "ModulationTargets.h"
#include "ChipModules.h"
#include <neon_ui_components/modules/AmpModule.h>
#include <neon_ui_components/modules/DahdsrModule.h>
#include <neon_ui_components/modules/LfoModule.h>
#include <neon_ui_components/modules/FxModule.h>
#include <neon_ui_components/modules/ControlModule.h>

namespace neon
{
    NeonChipAudioProcessorEditor::NeonChipAudioProcessorEditor (NeonChipAudioProcessor& p)
        : AudioProcessorEditor (&p), audioProcessor (p)
    {
        setLookAndFeel (&lookAndFeel);

        auto theme = NeonRegistry::getTheme();

        selectionPanel.setCategoryNames ({ "CHIP", "FILTER", "AMP", "M/FX", "MAIN" });
        selectionPanel.setButtonColors (juce::Colour (0xFFFF00FF), juce::Colour (0xFF808080));
        selectionPanel.setModuleNames ({ "CHIP OSC", "FILTER", "AMP", "A-ENV", "LFO 1", "LFO 2", "FX", "CTRL", "LIB" });
        selectionPanel.setCategoryModules (0, { 0 });
        selectionPanel.setCategoryModules (1, { 1 });
        selectionPanel.setCategoryModules (2, { 2, 3 });
        selectionPanel.setCategoryModules (3, { 4, 5, 6 });
        selectionPanel.setCategoryModules (4, { 7, 8 });
        selectionPanel.onModuleChanged = [this] (int index) { setActiveModule (index); };
        addAndMakeVisible (selectionPanel);

        auto chipOsc = std::make_unique<ChipOscModule> ("Chip Osc", theme.oscillator);
        auto filter = std::make_unique<ChipFilterModule> ("Filter", theme.filter);
        auto amp = std::make_unique<AmpModule> ("Amp", theme.amplifier);
        auto envAmp = std::make_unique<DahdsrModule> ("Amp Env", theme.envelope, false);
        auto lfo1 = std::make_unique<LfoModule> ("LFO 1", theme.modulation);
        auto lfo2 = std::make_unique<LfoModule> ("LFO 2", theme.modulation);
        auto fxModule = std::make_unique<FxModule> ("FX", theme.effects);
        auto ctrlModule = std::make_unique<ControlModule> ("Control", theme.indicator);
        auto libModule = std::make_unique<LibrarianModule> ("Librarian", theme.background.brighter());

        modules.add (chipOsc.release());
        modules.add (filter.release());
        modules.add (amp.release());
        modules.add (envAmp.release());
        modules.add (lfo1.release());
        modules.add (lfo2.release());
        modules.add (fxModule.release());
        modules.add (ctrlModule.release());
        modules.add (libModule.release());

        for (auto* m : modules)
            addChildComponent (m);

        setActiveModule (0);
        setSize (940, 840);
        startTimerHz (30);
    }

    NeonChipAudioProcessorEditor::~NeonChipAudioProcessorEditor()
    {
        stopTimer();
        setLookAndFeel (nullptr);
    }

    void NeonChipAudioProcessorEditor::timerCallback()
    {
        bool midiIsActive = audioProcessor.midiActivity.exchange (false);
        int activeVoices = audioProcessor.getSignalPath().getActiveVoicesCount();

        float pb = audioProcessor.getSignalPath().getPitchWheel();
        float mw = audioProcessor.getSignalPath().getModWheel();

        for (auto* m : modules)
        {
            m->setMidiActive (midiIsActive);
            m->setVoiceCount (activeVoices);

            if (auto* ctrl = dynamic_cast<ControlModule*> (m))
                ctrl->updateMeters (pb, mw, 0.0f);
        }
    }

    void NeonChipAudioProcessorEditor::setActiveModule (int index)
    {
        if (index < 0 || index >= modules.size())
            return;

        for (int i = 0; i < modules.size(); ++i)
            modules[i]->setVisible (i == index);

        selectionPanel.selectModuleByIndex (index, false);
        resized();
    }

    void NeonChipAudioProcessorEditor::paint (juce::Graphics& g)
    {
        g.fillAll (Colors::background);
    }

    void NeonChipAudioProcessorEditor::resized()
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
