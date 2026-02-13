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
    Neon777AudioProcessorEditor::Neon777AudioProcessorEditor (Neon777AudioProcessor& p)
        : AudioProcessorEditor (&p), audioProcessor (p)
    {
        setLookAndFeel (&lookAndFeel);

        auto theme = NeonRegistry::getTheme();

        selectionPanel.setCategoryNames ({ "OSC", "FILTER", "AMP", "M/FX", "MAIN" });
        selectionPanel.setButtonColors (juce::Colour (0xFFFF00FF), juce::Colour (0xFF808080));
        addAndMakeVisible (selectionPanel);
        
        std::vector<juce::String> fmRatios = {"0.5", "1.0", "1.5", "2.0", "3.0", "4.0", "5.0", "6.0", "7.0", "8.0", "9.0", "10.0", "11.0", "12.0", "14.0", "16.0"};

        // Modules
        auto osc1 = std::make_unique<ModuleBase> ("Oscillator 1", theme.oscillator);
        osc1->addChoiceParameter ("Ratio", fmRatios, 1); // 1.0 default
        osc1->addParameter ("Depth", 0.0f, 1.0f, 0.0f);
        osc1->addParameter ("Volume", 0.0f, 1.0f, 0.8f);
        osc1->addParameter ("Octave", -2.0f, 2.0f, 0.0f, false, 1.0f);
        osc1->addParameter ("Detune", -100.0f, 100.0f, 0.0f);
        osc1->addParameter ("Unison", 1.0f, 4.0f, 1.0f, false, 1.0f);
        osc1->addParameter ("USpread", 0.0f, 1.0f, 0.2f);
        osc1->addParameter ("Pan", -1.0f, 1.0f, 0.0f);
        
        osc1->addSpacer(); 
        osc1->addParameter ("KeySync", 0.0f, 1.0f, 1.0f, true);

        auto osc2 = std::make_unique<ModuleBase> ("Oscillator 2", theme.oscillator);
        osc2->addChoiceParameter ("Ratio", fmRatios, 3); // 2.0 default
        osc2->addParameter ("Depth", 0.0f, 1.0f, 0.0f);
        osc2->addParameter ("Volume", 0.0f, 1.0f, 0.8f);
        osc2->addParameter ("Octave", -2.0f, 2.0f, 0.0f, false, 1.0f);
        osc2->addParameter ("Detune", -100.0f, 100.0f, 0.0f);
        osc2->addParameter ("Unison", 1.0f, 4.0f, 1.0f, false, 1.0f);
        osc2->addParameter ("USpread", 0.0f, 1.0f, 0.2f);
        osc2->addParameter ("Pan", -1.0f, 1.0f, 0.0f);

        osc2->addSpacer();
        osc2->addParameter ("KeySync", 0.0f, 1.0f, 1.0f, true);

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

        modules.add (osc1.release());        // 0 - OSC1
        modules.add (osc2.release());        // 1 - OSC2
        modules.add (subOsc.release());      // 2 - SUB
        modules.add (noise.release());       // 3 - NOISE
        modules.add (envPitch.release());    // 4 - P-ENV
        modules.add (filter.release());      // 5 - FILTER
        modules.add (envFilter.release());   // 6 - F-ENV
        modules.add (amp.release());         // 7 - AMP
        modules.add (envAmp.release());      // 8 - A-ENV
        modules.add (lfo1.release());        // 9 - LFO1
        modules.add (lfo2.release());        // 10 - LFO2
        modules.add (modMatrix.release());   // 11 - MOD
        modules.add (envMod.release());      // 12 - M-ENV
        modules.add (fxModule.release());    // 13 - FX
        modules.add (ctrlModule.release());  // 14 - CTRL
        modules.add (arp.release());         // 15 - ARP
        modules.add (libModule.release());   // 16 - LIB (startup)

        for (auto* m : modules)
            addChildComponent (m);

        selectionPanel.setModuleNames ({
            "OSC 1", "OSC 2", "SUB", "NOISE", "P-ENV", "FILTER", "F-ENV", "AMP", "A-ENV",
            "LFO 1", "LFO 2", "MOD", "M-ENV", "FX", "CTRL", "ARP", "LIB"
        });
        selectionPanel.setCategoryModules (0, {0, 1, 2, 3, 4});
        selectionPanel.setCategoryModules (1, {5, 6});
        selectionPanel.setCategoryModules (2, {7, 8});
        selectionPanel.setCategoryModules (3, {9, 10, 11, 12, 13});
        selectionPanel.setCategoryModules (4, {14, 15, 16});

        selectionPanel.onModuleChanged = [this] (int index) { setActiveModule (index); };

        setActiveModule (16); // Default to LIB
        setSize (940, 840);
        startTimerHz (30);
    }

    Neon777AudioProcessorEditor::~Neon777AudioProcessorEditor()
    {
        stopTimer();
        setLookAndFeel (nullptr);
    }

    void Neon777AudioProcessorEditor::timerCallback()
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

    void Neon777AudioProcessorEditor::setActiveModule (int index)
    {
        if (index < 0 || index >= modules.size()) return;

        for (int i = 0; i < modules.size(); ++i)
            modules[i]->setVisible (i == index);
        
        selectionPanel.selectModuleByIndex (index, false);
        
        resized();
    }

    void Neon777AudioProcessorEditor::paint (juce::Graphics& g)
    {
        g.fillAll (Colors::background);
    }

    void Neon777AudioProcessorEditor::resized()
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
