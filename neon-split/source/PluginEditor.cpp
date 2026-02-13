#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "ModulationTargets.h"
#include "SplitModules.h"
#include <neon_ui_components/modules/ControlModule.h>
#include <neon_ui_components/modules/FxModule.h>

namespace neon
{
    /**
     * SplitSelectionPanel
     * Custom module selection panel for Neon Split.
     * Categories: BASS / PAD / ARP / DRUM / MAIN
     */
    class SplitSelectionPanel : public juce::Component
    {
    public:
        SplitSelectionPanel()
        {
            // Top row: 8 slots for category buttons (5 categories)
            for (int i = 0; i < 8; ++i)
            {
                auto btn = std::make_unique<juce::TextButton>();
                btn->setClickingTogglesState (true);
                btn->setRadioGroupId (4001);

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
                btn->setRadioGroupId (4002);

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
            categoryButtons[0]->setButtonText ("BASS");
            categoryButtons[1]->setButtonText ("PAD");
            categoryButtons[2]->setButtonText ("ARP");
            categoryButtons[3]->setButtonText ("DRUM");
            categoryButtons[4]->setButtonText ("MAIN");

            // BASS -> Bass(0)
            categoryModules[0] = { 0 };
            // PAD -> Pad(1)
            categoryModules[1] = { 1 };
            // ARP -> Arp(2)
            categoryModules[2] = { 2 };
            // DRUM -> Drums(3)
            categoryModules[3] = { 3 };
            // MAIN -> Split Ctrl(4), LIB(5)
            categoryModules[4] = { 4, 5 };

            setActiveCategory (4); // Start on MAIN
            categoryButtons[4]->setToggleState (true, juce::dontSendNotification);
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
                "BASS",                  // 0
                "PAD",                   // 1
                "ARP",                   // 2
                "DRUMS",                 // 3
                "SPLIT", "LIB"          // 4-5
            };
            if (index >= 0 && index < 6) return names[index];
            return "???";
        }
    };

    // ============================================================
    // Editor implementation
    // ============================================================
    NeonSplitAudioProcessorEditor::NeonSplitAudioProcessorEditor (NeonSplitAudioProcessor& p)
        : AudioProcessorEditor (&p), audioProcessor (p)
    {
        setLookAndFeel (&lookAndFeel);

        auto theme = NeonRegistry::getTheme();

        // Use custom Split selection panel
        auto* splitPanel = new SplitSelectionPanel();
        selectionPanelComponent.reset (splitPanel);
        addAndMakeVisible (selectionPanelComponent.get());

        // === Create all modules ===

        // 0: Bass engine (params + step sequencer grid in display)
        auto bass = std::make_unique<BassModule> ("Bass", juce::Colour (0xFF0088FF));

        // 1: Pad engine
        auto pad = std::make_unique<PadModule> ("Pad", juce::Colour (0xFF00CCAA));

        // 2: Arpeggiator
        auto arp = std::make_unique<ArpSplitModule> ("Arp", juce::Colour (0xFFFFAA00));

        // 3: Drums (params + 3×16 step grid in display)
        auto drums = std::make_unique<DrumModule> ("Drums", juce::Colour (0xFFFF4488));

        // 4: Split / global controls
        auto splitCtrl = std::make_unique<SplitControlModule> ("Split", theme.indicator);

        // 5: Librarian
        auto libModule = std::make_unique<LibrarianModule> ("Librarian", theme.background.brighter());

        modules.add (bass.release());       // 0 - BASS
        modules.add (pad.release());        // 1 - PAD
        modules.add (arp.release());        // 2 - ARP
        modules.add (drums.release());      // 3 - DRUMS
        modules.add (splitCtrl.release());  // 4 - SPLIT
        modules.add (libModule.release());  // 5 - LIB

        for (auto* m : modules)
            addChildComponent (m);

        splitPanel->onModuleChanged = [this] (int index) { setActiveModule (index); };

        setActiveModule (5); // Default to LIB
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
        if (index < 0 || index >= modules.size()) return;

        for (int i = 0; i < modules.size(); ++i)
            modules[i]->setVisible (i == index);

        if (auto* splitPanel = dynamic_cast<SplitSelectionPanel*> (selectionPanelComponent.get()))
            splitPanel->selectModuleByIndex (index, false);

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
