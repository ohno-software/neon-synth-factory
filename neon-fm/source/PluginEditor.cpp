#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "ModulationTargets.h"
#include "FmModules.h"
#include <neon_ui_components/modules/LfoModule.h>
#include <neon_ui_components/modules/FxModule.h>
#include <neon_ui_components/modules/ControlModule.h>

namespace neon
{
    /**
     * FmSelectionPanel
     * Custom module selection panel for Neon FM.
     * Remaps categories: FM / FILTER / AMP / M-FX / MAIN
     */
    class FmSelectionPanel : public juce::Component
    {
    public:
        FmSelectionPanel()
        {
            // Top row: 8 slots for category buttons (5 categories)
            for (int i = 0; i < 8; ++i)
            {
                auto btn = std::make_unique<juce::TextButton>();
                btn->setClickingTogglesState (true);
                btn->setRadioGroupId (3001);

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
                btn->setRadioGroupId (3002);

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

            // Category names for FM synth
            categoryButtons[0]->setButtonText ("FM");
            categoryButtons[1]->setButtonText ("FILTER");
            categoryButtons[2]->setButtonText ("AMP");
            categoryButtons[3]->setButtonText ("M/FX");
            categoryButtons[4]->setButtonText ("MAIN");

            // FM -> ALGO(0), OP1(1), OP2(2), OP3(3), OP4(4)
            categoryModules[0] = { 0, 1, 2, 3, 4 };
            // FILTER -> FILTER(5)
            categoryModules[1] = { 5 };
            // AMP -> AMP(6), A-ENV(7)
            categoryModules[2] = { 6, 7 };
            // M/FX -> LFO1(8), LFO2(9), FX(10)
            categoryModules[3] = { 8, 9, 10 };
            // MAIN -> CTRL(11), LIB(12)
            categoryModules[4] = { 11, 12 };

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

            int bottomY = (int)(unitH * 9);
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
                "ALGO", "OP 1", "OP 2", "OP 3", "OP 4",   // FM (0-4)
                "FILTER",                                     // FILTER (5)
                "AMP", "A-ENV",                              // AMP (6-7)
                "LFO 1", "LFO 2", "FX",                     // M/FX (8-10)
                "CTRL", "LIB"                                // MAIN (11-12)
            };
            if (index >= 0 && index < 13) return names[index];
            return "???";
        }
    };

    // ============================================================
    // Editor implementation
    // ============================================================
    NeonFmAudioProcessorEditor::NeonFmAudioProcessorEditor (NeonFmAudioProcessor& p)
        : AudioProcessorEditor (&p), audioProcessor (p)
    {
        setLookAndFeel (&lookAndFeel);

        auto theme = NeonRegistry::getTheme();

        // Use custom FM selection panel
        auto* fmPanel = new FmSelectionPanel();
        selectionPanelComponent.reset (fmPanel);
        addAndMakeVisible (selectionPanelComponent.get());

        // === Create all modules ===

        // 0: Algorithm selector
        auto algo = std::make_unique<FmAlgorithmModule> ("Algorithm", theme.oscillator);

        // 1-4: Operators
        auto op1 = std::make_unique<FmOperatorModule> ("Op 1", juce::Colour (0xFF00DDFF), 0);  // Cyan
        auto op2 = std::make_unique<FmOperatorModule> ("Op 2", juce::Colour (0xFF00FF88), 1);  // Green
        auto op3 = std::make_unique<FmOperatorModule> ("Op 3", juce::Colour (0xFFFFAA00), 2);  // Amber
        auto op4 = std::make_unique<FmOperatorModule> ("Op 4", juce::Colour (0xFFFF4488), 3);  // Pink

        // 5: Filter
        auto filter = std::make_unique<FmFilterModule> ("Filter", theme.filter);

        // 6: Amp
        auto amp = std::make_unique<AmpModule> ("Amp Output", theme.amplifier);

        // 7: Amp Envelope
        auto envAmp = std::make_unique<DahdsrModule> ("Amp Env", theme.envelope, false);

        // 8-9: LFOs
        auto lfo1 = std::make_unique<LfoModule> ("LFO 1", theme.modulation);
        auto lfo2 = std::make_unique<LfoModule> ("LFO 2", theme.modulation);

        // 10: FX
        auto fxModule = std::make_unique<FxModule> ("FX", theme.effects);

        // 11: Control
        auto ctrlModule = std::make_unique<ControlModule> ("Control", theme.indicator);

        // 12: Librarian
        auto libModule = std::make_unique<LibrarianModule> ("Librarian", theme.background.brighter());

        // Add all modules
        modules.add (std::move (algo));       // 0 - ALGO
        modules.add (std::move (op1));        // 1 - OP1
        modules.add (std::move (op2));        // 2 - OP2
        modules.add (std::move (op3));        // 3 - OP3
        modules.add (std::move (op4));        // 4 - OP4
        modules.add (std::move (filter));     // 5 - FILTER
        modules.add (std::move (amp));        // 6 - AMP
        modules.add (std::move (envAmp));     // 7 - A-ENV
        modules.add (std::move (lfo1));       // 8 - LFO1
        modules.add (std::move (lfo2));       // 9 - LFO2
        modules.add (std::move (fxModule));   // 10 - FX
        modules.add (std::move (ctrlModule)); // 11 - CTRL
        modules.add (std::move (libModule));  // 12 - LIB

        for (auto* m : modules)
            addChildComponent (m);

        fmPanel->onModuleChanged = [this] (int index) { setActiveModule (index); };

        setActiveModule (0); // Default to Algorithm view
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
        if (index < 0 || index >= modules.size()) return;

        for (int i = 0; i < modules.size(); ++i)
            modules[i]->setVisible (i == index);

        if (auto* fmPanel = dynamic_cast<FmSelectionPanel*> (selectionPanelComponent.get()))
            fmPanel->selectModuleByIndex (index, false);

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
