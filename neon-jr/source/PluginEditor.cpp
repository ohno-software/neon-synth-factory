#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "ModulationTargets.h"
#include <neon_ui_components/modules/LfoModule.h>
#include <neon_ui_components/modules/ArpModule.h>
#include <neon_ui_components/modules/FxModule.h>
#include <neon_ui_components/modules/ControlModule.h>
#include <neon_ui_components/modules/ModMatrixModule.h>

namespace neon
{
    NeonJrAudioProcessorEditor::NeonJrAudioProcessorEditor (NeonJrAudioProcessor& p)
        : AudioProcessorEditor (&p), audioProcessor (p)
    {
        setLookAndFeel (&lookAndFeel);

        auto theme = NeonRegistry::getTheme();

        addAndMakeVisible (topSelectionPanel);
        addAndMakeVisible (bottomSelectionPanel);
        
        // Modules
        auto osc1 = std::make_unique<OscillatorModule> ("Oscillator 1", theme.oscillator);
        auto osc2 = std::make_unique<OscillatorModule> ("Oscillator 2", theme.oscillator);
        auto filter = std::make_unique<LadderFilterModule> ("Ladder Filter", theme.filter);
        auto amp = std::make_unique<AmpModule> ("Amp Output", theme.amplifier);
        
        auto envPitch = std::make_unique<DahdsrModule> ("Pitch Env", theme.envelope, false, true);  // Include modulation parameters
        auto envFilter = std::make_unique<DahdsrModule> ("Filter Env", theme.envelope, false, true); // Include modulation parameters
        auto envMod = std::make_unique<DahdsrModule> ("Mod Env", theme.envelope, false);  // No modulation parameters
        auto envAmp = std::make_unique<DahdsrModule> ("Amp Env", theme.envelope, false);  // No modulation parameters

        auto lfo1 = std::make_unique<LfoModule> ("LFO 1", theme.modulation);
        auto lfo2 = std::make_unique<LfoModule> ("LFO 2", theme.modulation);
        auto lfo3 = std::make_unique<LfoModule> ("LFO 3", theme.modulation);

        auto arp = std::make_unique<ArpModule> ("Arp", theme.modulation);

        // Control Matrix Module (Now renamed to MOD)
        // Expanded to 16 slots (4 pages), no Source parameter (uses Mod Env implicitly)
        auto modMatrix = std::make_unique<ModMatrixModule> ("Mod", theme.modulation);
        auto targets = getModTargetNames();

        for (int i = 1; i <= 16; ++i)
        {
            modMatrix->addChoiceParameter ("Slot " + juce::String(i) + " Target", targets, 0);
            modMatrix->addParameter       ("Slot " + juce::String(i) + " Amount", -100.0f, 100.0f, 0.0f);
        }

        // FX Module
        auto fxModule = std::make_unique<FxModule> ("FX", theme.effects);

        // Global Controls Module
        auto ctrlModule = std::make_unique<ControlModule> ("Control", theme.indicator);

        // Librarian Module
        auto libModule = std::make_unique<LibrarianModule> ("Librarian", theme.background.brighter());

        // ... (existing code continues)

        // Page 1 has 7 cards (6 DAHDSR + 1 Mode). 
        // We add one empty parameter to serve as a spacer to perfectly align the 4 Slot-pairs on Page 2.
        envMod->addChoiceParameter (" ", {" "}, 0); 

        for (int i = 1; i <= 4; ++i)
        {
            envMod->addChoiceParameter ("Slot " + juce::String(i) + " Target", targets, 0);
            envMod->addParameter       ("Slot " + juce::String(i) + " Amount", -100.0f, 100.0f, 0.0f);
        }

        modules.add (std::move (osc1));
        modules.add (std::move (osc2));
        modules.add (std::move (filter));
        modules.add (std::move (amp));
        modules.add (std::move (envPitch));
        modules.add (std::move (envFilter));
        modules.add (std::move (envMod));
        modules.add (std::move (envAmp));
        modules.add (std::move (lfo1));
        modules.add (std::move (lfo2));
        modules.add (std::move (lfo3));
        modules.add (std::move (arp));
        modules.add (std::move (modMatrix));
        modules.add (std::move (fxModule));
        modules.add (std::move (ctrlModule));
        modules.add (std::move (libModule));

        topSelectionPanel.setModuleNames ({
            "OSC 1", "OSC 2", "FILTER", "AMP", "P-ENV", "F-ENV", "M-ENV", "A-ENV"
        });

        bottomSelectionPanel.setModuleNames ({
            "LFO 1", "LFO 2", "LFO 3", "ARP", "MOD", "FX", "CTRL", "LIB"
        });

        for (auto* m : modules)
            addChildComponent (m);

        topSelectionPanel.onModuleChanged = [this] (int index) { setActiveModule (index); };
        bottomSelectionPanel.onModuleChanged = [this] (int index) { setActiveModule (index + 8); };

        setActiveModule (0);
        setSize (940, 840);
        startTimerHz (30);
    }

    NeonJrAudioProcessorEditor::~NeonJrAudioProcessorEditor()
    {
        stopTimer();
        setLookAndFeel (nullptr);
    }

    void NeonJrAudioProcessorEditor::timerCallback()
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
                ctrl->updateMeters (pb, mw, 0.0f); // Aftertouch is now per-voice, show 0 for global display
        }
    }

    void NeonJrAudioProcessorEditor::setActiveModule (int index)
    {
        if (index < 0 || index >= modules.size()) return;

        for (int i = 0; i < modules.size(); ++i)
            modules[i]->setVisible (i == index);
        
        // Synchronize selection between the two panels
        if (index < 8)
        {
            topSelectionPanel.selectIndex (index, false);
            bottomSelectionPanel.selectIndex (-1, false);
        }
        else
        {
            topSelectionPanel.selectIndex (-1, false);
            bottomSelectionPanel.selectIndex (index - 8, false);
        }
        
        resized();
    }

    void NeonJrAudioProcessorEditor::paint (juce::Graphics& g)
    {
        g.fillAll (Colors::background);
    }

    void NeonJrAudioProcessorEditor::resized()
    {
        auto bounds = getLocalBounds();
        
        // Main Synth Area (940px wide)
        auto synthArea = bounds;
        
        // 1/12 units for the vertical layout
        float unitH = (float)synthArea.getHeight() / 12.0f;
        
        // Top 1/12: Top Buttons
        topSelectionPanel.setBounds (synthArea.removeFromTop ((int)unitH));
        
        // Bottom 1/12: Bottom Buttons
        bottomSelectionPanel.setBounds (synthArea.removeFromBottom ((int)unitH));
        
        // The remaining 10/12 is for the active module
        for (auto* m : modules)
        {
            if (m->isVisible())
            {
                m->setBounds (synthArea);
            }
        }
    }
}
