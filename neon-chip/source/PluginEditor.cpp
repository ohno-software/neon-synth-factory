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
    /**
     * ChipSelectionPanel
     * Custom module selection panel for Neon Chip.
     * Categories: CHIP / FILTER / AMP / M-FX / MAIN
     */
    class ChipSelectionPanel : public juce::Component
    {
    public:
        ChipSelectionPanel()
        {
            // Top row: 8 slots for category buttons (5 active)
            for (int i = 0; i < 8; ++i)
            {
                auto btn = std::make_unique<juce::TextButton>();
                btn->setClickingTogglesState (true);
                btn->setRadioGroupId (5001);

                if (i < 5)
                {
                    int catIndex = i;
                    btn->onClick = [this, catIndex] {
                        setActiveCategory (catIndex);
                        updateButtonColors();
                        if (!categoryModules[catIndex].empty() && onModuleChanged)
                            onModuleChanged (categoryModules[catIndex][0]);
                    };
                    addAndMakeVisible (btn.get());
                }
                else
                {
                    btn->setVisible (false);
                    addChildComponent (btn.get());
                }

                categoryButtons.add (std::move (btn));
            }

            // Bottom row: 8 slots for module buttons
            for (int i = 0; i < 8; ++i)
            {
                auto btn = std::make_unique<juce::TextButton>();
                btn->setClickingTogglesState (true);
                btn->setRadioGroupId (5002);

                int slotIndex = i;
                btn->onClick = [this, slotIndex] {
                    if (currentModuleIndices[slotIndex] >= 0)
                    {
                        for (int j = 0; j < 8; ++j)
                            if (j != slotIndex)
                                moduleButtons[j]->setToggleState (false, juce::dontSendNotification);
                        moduleButtons[slotIndex]->setToggleState (true, juce::dontSendNotification);
                        updateButtonColors();
                        if (onModuleChanged)
                            onModuleChanged (currentModuleIndices[slotIndex]);
                    }
                };
                addChildComponent (btn.get());
                moduleButtons.add (std::move (btn));
            }

            // Category names
            categoryButtons[0]->setButtonText ("CHIP");
            categoryButtons[1]->setButtonText ("FILTER");
            categoryButtons[2]->setButtonText ("AMP");
            categoryButtons[3]->setButtonText ("M/FX");
            categoryButtons[4]->setButtonText ("MAIN");

            // CHIP -> OSC(0)
            categoryModules[0] = { 0 };
            // FILTER -> FILTER(1)
            categoryModules[1] = { 1 };
            // AMP -> AMP(2), A-ENV(3)
            categoryModules[2] = { 2, 3 };
            // M/FX -> LFO1(4), LFO2(5), FX(6)
            categoryModules[3] = { 4, 5, 6 };
            // MAIN -> CTRL(7), LIB(8)
            categoryModules[4] = { 7, 8 };

            setActiveCategory (0);
            categoryButtons[0]->setToggleState (true, juce::dontSendNotification);
            updateButtonColors();
        }

        void setActiveCategory (int categoryIndex)
        {
            if (categoryIndex < 0 || categoryIndex >= 5) return;

            currentCategory = categoryIndex;
            const auto& moduleList = categoryModules[categoryIndex];

            for (int i = 0; i < 8; ++i)
                currentModuleIndices[i] = -1;

            for (int i = 0; i < 8; ++i)
            {
                if (i < (int) moduleList.size())
                {
                    int moduleIdx = moduleList[i];
                    currentModuleIndices[i] = moduleIdx;
                    moduleButtons[i]->setButtonText (getModuleName (moduleIdx));
                    moduleButtons[i]->setVisible (true);
                }
                else
                {
                    moduleButtons[i]->setVisible (false);
                }
                moduleButtons[i]->setToggleState (false, juce::dontSendNotification);
            }

            if (!moduleList.empty())
            {
                moduleButtons[0]->setToggleState (true, juce::dontSendNotification);
                updateButtonColors();
            }

            resized();
        }

        void selectModuleByIndex (int globalModuleIndex, bool sendNotification)
        {
            int targetCategory = -1;
            int targetButton = -1;

            for (int cat = 0; cat < 5; ++cat)
            {
                const auto& moduleList = categoryModules[cat];
                for (int i = 0; i < (int) moduleList.size(); ++i)
                {
                    if (moduleList[i] == globalModuleIndex)
                    {
                        targetCategory = cat;
                        targetButton = i;
                        break;
                    }
                }
                if (targetCategory >= 0) break;
            }

            if (targetCategory >= 0)
            {
                if (currentCategory != targetCategory)
                {
                    categoryButtons[targetCategory]->setToggleState (true, juce::dontSendNotification);
                    setActiveCategory (targetCategory);
                }

                for (int i = 0; i < 8; ++i)
                    moduleButtons[i]->setToggleState (false, juce::dontSendNotification);

                moduleButtons[targetButton]->setToggleState (true,
                    sendNotification ? juce::sendNotification : juce::dontSendNotification);
                updateButtonColors();
            }
        }

        std::function<void (int)> onModuleChanged;

        void resized() override
        {
            auto bounds = getLocalBounds();
            float unitH = (float) bounds.getHeight() / 10.0f;
            int slotWidth = bounds.getWidth() / 8;

            for (int i = 0; i < 8; ++i)
                categoryButtons[i]->setBounds (i * slotWidth, 0, slotWidth, (int) unitH);

            int bottomY = (int) (unitH * 9);
            for (int i = 0; i < 8; ++i)
                moduleButtons[i]->setBounds (i * slotWidth, bottomY, slotWidth, (int) unitH);
        }

        void updateButtonColors()
        {
            auto magenta = juce::Colour (0xFFFF00FF);
            auto grey = juce::Colour (0xFF808080);

            for (auto* btn : categoryButtons)
            {
                if (btn->getToggleState())
                {
                    btn->setColour (juce::TextButton::buttonColourId, juce::Colours::white.withAlpha (0.95f));
                    btn->setColour (juce::TextButton::textColourOnId, magenta);
                    btn->setColour (juce::TextButton::textColourOffId, magenta);
                }
                else
                {
                    btn->setColour (juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
                    btn->setColour (juce::TextButton::textColourOnId, grey);
                    btn->setColour (juce::TextButton::textColourOffId, grey);
                }
                btn->repaint();
            }

            for (auto* btn : moduleButtons)
            {
                if (btn->getToggleState())
                {
                    btn->setColour (juce::TextButton::buttonColourId, juce::Colours::white.withAlpha (0.95f));
                    btn->setColour (juce::TextButton::textColourOnId, magenta);
                    btn->setColour (juce::TextButton::textColourOffId, magenta);
                }
                else
                {
                    btn->setColour (juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
                    btn->setColour (juce::TextButton::textColourOnId, grey);
                    btn->setColour (juce::TextButton::textColourOffId, grey);
                }
                btn->repaint();
            }
        }

    private:
        juce::OwnedArray<juce::TextButton> categoryButtons;
        juce::OwnedArray<juce::TextButton> moduleButtons;

        int currentCategory = 0;
        std::map<int, std::vector<int>> categoryModules;
        int currentModuleIndices[8] = { -1, -1, -1, -1, -1, -1, -1, -1 };

        juce::String getModuleName (int index)
        {
            const char* names[] = {
                "CHIP OSC",                             // 0
                "FILTER",                               // 1
                "AMP", "A-ENV",                         // 2-3
                "LFO 1", "LFO 2", "FX",                // 4-6
                "CTRL", "LIB"                           // 7-8
            };
            if (index >= 0 && index < 9) return names[index];
            return "???";
        }
    };

    // ============================================================
    // Editor implementation
    // ============================================================
    NeonChipAudioProcessorEditor::NeonChipAudioProcessorEditor (NeonChipAudioProcessor& p)
        : AudioProcessorEditor (&p), audioProcessor (p)
    {
        setLookAndFeel (&lookAndFeel);

        auto theme = NeonRegistry::getTheme();

        // Selection panel
        auto* chipPanel = new ChipSelectionPanel();
        selectionPanelComponent.reset (chipPanel);
        addAndMakeVisible (selectionPanelComponent.get());

        // === Create all modules ===

        // 0: Chip oscillator (custom module)
        auto chipOsc = std::make_unique<ChipOscModule> ("Chip Osc", theme.oscillator);

        // 1: Filter (custom chip filter module)
        auto filter = std::make_unique<ChipFilterModule> ("Filter", theme.filter);

        // 2: Amp
        auto amp = std::make_unique<AmpModule> ("Amp", theme.amplifier);

        // 3: Amp Envelope
        auto envAmp = std::make_unique<DahdsrModule> ("Amp Env", theme.envelope, false);

        // 4-5: LFOs
        auto lfo1 = std::make_unique<LfoModule> ("LFO 1", theme.modulation);
        auto lfo2 = std::make_unique<LfoModule> ("LFO 2", theme.modulation);

        // 6: FX
        auto fxModule = std::make_unique<FxModule> ("FX", theme.effects);

        // 7: Control
        auto ctrlModule = std::make_unique<ControlModule> ("Control", theme.indicator);

        // 8: Librarian
        auto libModule = std::make_unique<LibrarianModule> ("Librarian", theme.background.brighter());

        // Add modules
        modules.add (std::move (chipOsc));     // 0 - CHIP OSC
        modules.add (std::move (filter));      // 1 - FILTER
        modules.add (std::move (amp));         // 2 - AMP
        modules.add (std::move (envAmp));      // 3 - A-ENV
        modules.add (std::move (lfo1));        // 4 - LFO1
        modules.add (std::move (lfo2));        // 5 - LFO2
        modules.add (std::move (fxModule));    // 6 - FX
        modules.add (std::move (ctrlModule));  // 7 - CTRL
        modules.add (std::move (libModule));   // 8 - LIB

        for (auto* m : modules)
            addChildComponent (m);

        chipPanel->onModuleChanged = [this] (int index) { setActiveModule (index); };

        setActiveModule (0);  // Default to Chip Osc view
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
        if (index < 0 || index >= modules.size()) return;

        for (int i = 0; i < modules.size(); ++i)
            modules[i]->setVisible (i == index);

        if (auto* chipPanel = dynamic_cast<ChipSelectionPanel*> (selectionPanelComponent.get()))
            chipPanel->selectModuleByIndex (index, false);

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

        auto topNav = bounds.removeFromTop ((int) unitH);
        auto bottomNav = bounds.removeFromBottom ((int) unitH);

        selectionPanelComponent->setBounds (0, 0, getWidth(), getHeight());

        for (auto* m : modules)
        {
            if (m->isVisible())
                m->setBounds (bounds);
        }
    }

} // namespace neon
