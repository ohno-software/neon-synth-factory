#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "ModulationTargets.h"
#include <neon_ui_components/modules/LfoModule.h>
#include <neon_ui_components/modules/ArpModule.h>
#include <neon_ui_components/modules/FxModule.h>
#include <neon_ui_components/modules/ControlModule.h>
#include <neon_ui_components/modules/ModMatrixModule.h>
#include <neon_ui_components/modules/NoiseModule.h>
#include <neon_ui_components/modules/SubOscModule.h>

namespace neon
{
    NeonJrAudioProcessorEditor::NeonJrAudioProcessorEditor (NeonJrAudioProcessor& p)
        : AudioProcessorEditor (&p), audioProcessor (p)
    {
        setLookAndFeel (&lookAndFeel);

        auto theme = NeonRegistry::getTheme();

        addAndMakeVisible (selectionPanel);
        
        // Modules
        auto osc1 = std::make_unique<OscillatorModule> ("Oscillator 1", theme.oscillator);
        auto subOsc = std::make_unique<SubOscModule> ("Sub Osc", theme.oscillator);
        auto noise = std::make_unique<NoiseModule> ("Noise", theme.oscillator);
        auto filter = std::make_unique<LadderFilterModule> ("Ladder Filter", theme.filter);
        auto amp = std::make_unique<AmpModule> ("Amp Output", theme.amplifier);
        
        auto envPitch = std::make_unique<DahdsrModule> ("Pitch Env", theme.envelope, false, true);  // Include modulation parameters
        auto envFilter = std::make_unique<DahdsrModule> ("Filter Env", theme.envelope, false, true); // Include modulation parameters
        auto envMod = std::make_unique<DahdsrModule> ("Mod Env", theme.envelope, false);  // No modulation parameters
        auto envAmp = std::make_unique<DahdsrModule> ("Amp Env", theme.envelope, false);  // No modulation parameters

        auto lfo1 = std::make_unique<LfoModule> ("LFO 1", theme.modulation);
        auto lfo2 = std::make_unique<LfoModule> ("LFO 2", theme.modulation);

        auto arp = std::make_unique<ArpModule> ("Arp", theme.modulation);

        // Control Matrix Module (Now renamed to MOD)
        // 4-slot modulation matrix with [Source][Target][Amount]
        auto modMatrix = std::make_unique<ModMatrixModule> ("Mod", theme.modulation);
        auto targets = getModTargetNames();
        std::vector<juce::String> sources = {"None", "Velocity", "Aftertouch", "Note#", 
                                             "CC 1 (Mod Wheel)", "CC 11 (Expression)", "CC 2 (Breath)", 
                                             "CC 4 (Foot Pedal)", "CC 64 (Sustain)",
                                             "CC 16", "CC 17", "CC 18", "CC 19",
                                             "CC 80", "CC 81", "CC 82", "CC 83"};

        for (int i = 1; i <= 4; ++i)
        {
            modMatrix->addChoiceParameter ("Slot " + juce::String(i) + " Source", sources, 0);
            modMatrix->addChoiceParameter ("Slot " + juce::String(i) + " Target", targets, 0);
            modMatrix->addParameter       ("Slot " + juce::String(i) + " Amount", -100.0f, 100.0f, 0.0f);
            modMatrix->addChoiceParameter (" ", {" "}, 0); // Spacer for alignment
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

        modules.add (std::move (osc1));        // 0 - OSC
        modules.add (std::move (subOsc));      // 1 - SUB
        modules.add (std::move (noise));       // 2 - NOISE
        modules.add (std::move (envPitch));    // 3 - P-ENV
        modules.add (std::move (filter));      // 4 - FILTER
        modules.add (std::move (envFilter));   // 5 - F-ENV
        modules.add (std::move (amp));         // 6 - AMP
        modules.add (std::move (envAmp));      // 7 - A-ENV
        modules.add (std::move (lfo1));        // 8 - LFO1
        modules.add (std::move (lfo2));        // 9 - LFO2
        modules.add (std::move (modMatrix));   // 10 - MOD
        modules.add (std::move (envMod));      // 11 - M-ENV
        modules.add (std::move (fxModule));    // 12 - FX
        modules.add (std::move (ctrlModule));  // 13 - CTRL
        modules.add (std::move (arp));         // 14 - ARP
        modules.add (std::move (libModule));   // 15 - LIB (startup)

        for (auto* m : modules)
            addChildComponent (m);

        selectionPanel.onModuleChanged = [this] (int index) { setActiveModule (index); };

        setActiveModule (15); // Default to LIB
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
        
        selectionPanel.selectModuleByIndex (index, false);
        
        resized();
    }

    void NeonJrAudioProcessorEditor::paint (juce::Graphics& g)
    {
        g.fillAll (Colors::background);
    }

    void NeonJrAudioProcessorEditor::resized()
    {
        auto bounds = getLocalBounds();
        
        // Layout in 1/10ths:
        // 1/10 - Top Navigation (category buttons)
        // 5/10 - Module Display (1/2 of synth)
        // 1/10 - Parameters Row 1
        // 1/10 - Parameters Row 2
        // 1/10 - Parameter Navigation
        // 1/10 - Bottom Navigation (module buttons)
        
        float unitH = (float)bounds.getHeight() / 10.0f;
        
        // Top 1/10: Category navigation
        auto topNav = bounds.removeFromTop ((int)unitH);
        
        // Bottom 1/10: Module navigation  
        auto bottomNav = bounds.removeFromBottom ((int)unitH);
        
        // Selection panel gets full height of window, positioned to place rows correctly
        selectionPanel.setBounds (0, 0, getWidth(), getHeight());
        
        // The remaining 8/10 is for the active module (5/10 display + 3/10 parameters)
        for (auto* m : modules)
        {
            if (m->isVisible())
            {
                m->setBounds (bounds);
            }
        }
    }
}
