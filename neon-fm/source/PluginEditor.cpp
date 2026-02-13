#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "ModulationTargets.h"
#include "FmModules.h"
#include <neon_ui_components/modules/LfoModule.h>
#include <neon_ui_components/modules/FxModule.h>
#include <neon_ui_components/modules/ControlModule.h>

namespace neon
{
    NeonFmAudioProcessorEditor::NeonFmAudioProcessorEditor (NeonFmAudioProcessor& p)
        : AudioProcessorEditor (&p), audioProcessor (p)
    {
        setLookAndFeel (&lookAndFeel);

        auto theme = NeonRegistry::getTheme();

        selectionPanel.setCategoryNames ({ "FM", "FILTER", "AMP", "M/FX", "MAIN" });
        selectionPanel.setButtonColors (juce::Colour (0xFFFF00FF), juce::Colour (0xFF808080));
        selectionPanel.setModuleNames ({
            "ALGO", "OP 1", "OP 2", "OP 3", "OP 4",
            "FILTER", "F-ENV",
            "AMP", "A-ENV",
            "LFO 1", "LFO 2", "FX",
            "CTRL", "LIB"
        });
        selectionPanel.setCategoryModules (0, { 0, 1, 2, 3, 4 });
        selectionPanel.setCategoryModules (1, { 5, 6 });
        selectionPanel.setCategoryModules (2, { 7, 8 });
        selectionPanel.setCategoryModules (3, { 9, 10, 11 });
        selectionPanel.setCategoryModules (4, { 12, 13 });
        selectionPanel.onModuleChanged = [this] (int index) { setActiveModule (index); };
        addAndMakeVisible (selectionPanel);

        auto algo = std::make_unique<FmAlgorithmModule> ("Algorithm", theme.oscillator);
        auto op1 = std::make_unique<FmOperatorModule> ("Op 1", juce::Colour (0xFF00DDFF), 0);
        auto op2 = std::make_unique<FmOperatorModule> ("Op 2", juce::Colour (0xFF00FF88), 1);
        auto op3 = std::make_unique<FmOperatorModule> ("Op 3", juce::Colour (0xFFFFAA00), 2);
        auto op4 = std::make_unique<FmOperatorModule> ("Op 4", juce::Colour (0xFFFF4488), 3);
        auto filter = std::make_unique<FmFilterModule> ("Filter", theme.filter);
        auto filterEnv = std::make_unique<FmFilterEnvModule> ("Filter Env", theme.filter);
        auto amp = std::make_unique<AmpModule> ("Amp Output", theme.amplifier);
        auto envAmp = std::make_unique<DahdsrModule> ("Amp Env", theme.envelope, false);
        auto lfo1 = std::make_unique<LfoModule> ("LFO 1", theme.modulation);
        auto lfo2 = std::make_unique<LfoModule> ("LFO 2", theme.modulation);
        auto fxModule = std::make_unique<FxModule> ("FX", theme.effects);
        auto ctrlModule = std::make_unique<ControlModule> ("Control", theme.indicator);
        auto libModule = std::make_unique<LibrarianModule> ("Librarian", theme.background.brighter());

        modules.add (algo.release());
        modules.add (op1.release());
        modules.add (op2.release());
        modules.add (op3.release());
        modules.add (op4.release());
        modules.add (filter.release());
        modules.add (filterEnv.release());
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

    NeonFmAudioProcessorEditor::~NeonFmAudioProcessorEditor()
    {
        stopTimer();
        setLookAndFeel (nullptr);
    }

    void NeonFmAudioProcessorEditor::timerCallback()
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

    void NeonFmAudioProcessorEditor::setActiveModule (int index)
    {
        if (index < 0 || index >= modules.size())
            return;

        for (int i = 0; i < modules.size(); ++i)
            modules[i]->setVisible (i == index);

        selectionPanel.selectModuleByIndex (index, false);
        resized();
    }

    void NeonFmAudioProcessorEditor::paint (juce::Graphics& g)
    {
        g.fillAll (Colors::background);
    }

    void NeonFmAudioProcessorEditor::resized()
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
