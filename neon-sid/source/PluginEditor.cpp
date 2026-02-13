#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "SidModules.h"

namespace neon
{
    /**
     * SidSelectionPanel
     * Custom selection panel for Neon SID.
     * Categories: SID / FILTER / AMP / M-FX / MAIN
     */
    class SidSelectionPanel : public juce::Component
    {
    public:
        SidSelectionPanel()
        {
            // Top row: Category buttons
            for (int i = 0; i < 5; ++i)
            {
                auto btn = std::make_unique<juce::TextButton>();
                btn->setClickingTogglesState (true);
                btn->setRadioGroupId (7001);

                int catIndex = i;
                btn->onClick = [this, catIndex] {
                    setActiveCategory (catIndex);
                    updateButtonColors();
                    if (!categoryModules[catIndex].empty() && onModuleChanged)
                        onModuleChanged (categoryModules[catIndex][0]);
                };
                addAndMakeVisible (btn.get());
                categoryButtons.add (btn.release());
            }

            // Bottom row: Module buttons
            for (int i = 0; i < 8; ++i)
            {
                auto btn = std::make_unique<juce::TextButton>();
                btn->setClickingTogglesState (true);
                btn->setRadioGroupId (7002);

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
                moduleButtons.add (btn.release());
            }

            categoryButtons[0]->setButtonText ("SID");
            categoryButtons[1]->setButtonText ("FILTER");
            categoryButtons[2]->setButtonText ("AMP");
            categoryButtons[3]->setButtonText ("M-FX");
            categoryButtons[4]->setButtonText ("MAIN");

            // Mapping: SID -> Osc 1, 2, 3
            categoryModules[0] = { 0, 1, 2 }; // OSC 1, 2, 3
            categoryModules[1] = { 3 };       // Ladder Filter
            categoryModules[2] = { 4, 5 };    // Amp, Env
            categoryModules[3] = { };         // (Empty for now)
            categoryModules[4] = { 6 };       // Librarian

            setActiveCategory (4);
            categoryButtons[4]->setToggleState (true, juce::dontSendNotification);
            updateButtonColors();
        }

        void setActiveCategory (int categoryIndex)
        {
            if (categoryIndex < 0 || categoryIndex >= 5) return;
            currentCategory = categoryIndex;
            const auto& moduleList = categoryModules[categoryIndex];

            for (int i = 0; i < 8; ++i) currentModuleIndices[i] = -1;

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
                    if (moduleList[i] == globalModuleIndex) { targetCategory = cat; targetButton = i; break; }
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
                for (int i = 0; i < 8; ++i) moduleButtons[i]->setToggleState (false, juce::dontSendNotification);
                moduleButtons[targetButton]->setToggleState (true, sendNotification ? juce::sendNotification : juce::dontSendNotification);
                updateButtonColors();
            }
        }

        std::function<void (int)> onModuleChanged;

        void resized() override
        {
            auto bounds = getLocalBounds();
            float unitH = (float) bounds.getHeight() / 10.0f;
            int slotWidth = bounds.getWidth() / 8;
            for (int i = 0; i < 5; ++i) categoryButtons[i]->setBounds (i * slotWidth, 0, slotWidth, (int) unitH);
            int bottomY = (int) (unitH * 9);
            for (int i = 0; i < 8; ++i) moduleButtons[i]->setBounds (i * slotWidth, bottomY, slotWidth, (int) unitH);
        }

        void updateButtonColors()
        {
            auto c64_blue = juce::Colour (0xFF4040FF);
            auto grey = juce::Colour (0xFF808080);
            for (auto* btn : categoryButtons)
            {
                if (btn->getToggleState()) { btn->setColour (juce::TextButton::buttonColourId, juce::Colours::white.withAlpha (0.9f)); btn->setColour (juce::TextButton::textColourOnId, c64_blue); btn->setColour (juce::TextButton::textColourOffId, c64_blue); }
                else { btn->setColour (juce::TextButton::buttonColourId, juce::Colours::transparentBlack); btn->setColour (juce::TextButton::textColourOnId, grey); btn->setColour (juce::TextButton::textColourOffId, grey); }
                btn->repaint();
            }
            for (auto* btn : moduleButtons)
            {
                if (btn->getToggleState()) { btn->setColour (juce::TextButton::buttonColourId, juce::Colours::white.withAlpha (0.9f)); btn->setColour (juce::TextButton::textColourOnId, c64_blue); btn->setColour (juce::TextButton::textColourOffId, c64_blue); }
                else { btn->setColour (juce::TextButton::buttonColourId, juce::Colours::transparentBlack); btn->setColour (juce::TextButton::textColourOnId, grey); btn->setColour (juce::TextButton::textColourOffId, grey); }
                btn->repaint();
            }
        }

    private:
        juce::OwnedArray<juce::TextButton> categoryButtons;
        juce::OwnedArray<juce::TextButton> moduleButtons;
        int currentCategory = 4;
        std::map<int, std::vector<int>> categoryModules;
        int currentModuleIndices[8] = { -1, -1, -1, -1, -1, -1, -1, -1 };

        juce::String getModuleName (int index)
        {
            const char* names[] = { "OSC 1", "OSC 2", "OSC 3", "FILTER", "AMP", "A-ENV", "LIB" };
            if (index >= 0 && index < 7) return names[index];
            return "???";
        }
    };

    NeonSidAudioProcessorEditor::NeonSidAudioProcessorEditor (NeonSidAudioProcessor& p)
        : AudioProcessorEditor (&p), audioProcessor (p)
    {
        setLookAndFeel (&lookAndFeel);
        auto theme = NeonRegistry::getTheme();

        auto* sidPanel = new SidSelectionPanel();
        selectionPanelComponent.reset (sidPanel);
        addAndMakeVisible (selectionPanelComponent.get());
        sidPanel->onModuleChanged = [this] (int index) { setActiveModule (index); };

        modules.add (std::make_unique<SidOscModule> ("Osc 1", theme.oscillator)); // 0
        modules.add (std::make_unique<SidOscModule> ("Osc 2", theme.oscillator)); // 1
        modules.add (std::make_unique<SidOscModule> ("Osc 3", theme.oscillator)); // 2
        modules.add (std::make_unique<LadderFilterModule> ("Filter", theme.filter)); // 3
        modules.add (std::make_unique<AmpModule> ("Amp Output", theme.amplifier)); // 4
        modules.add (std::make_unique<DahdsrModule> ("Amp Env", theme.envelope, false)); // 5
        modules.add (std::make_unique<LibrarianModule> ("Librarian", theme.background.brighter())); // 6

        for (auto* m : modules) addChildComponent (m);

        setActiveModule (6);
        setSize (940, 840);
        startTimerHz (30);
    }

    NeonSidAudioProcessorEditor::~NeonSidAudioProcessorEditor() { stopTimer(); setLookAndFeel (nullptr); }

    void NeonSidAudioProcessorEditor::setActiveModule (int index)
    {
        if (index < 0 || index >= modules.size()) return;
        activeModuleIndex = index;
        for (int i = 0; i < modules.size(); ++i) modules[i]->setVisible (i == index);
        if (auto* sidPanel = dynamic_cast<SidSelectionPanel*> (selectionPanelComponent.get()))
            sidPanel->selectModuleByIndex (index, false);
        resized();
    }

    void NeonSidAudioProcessorEditor::paint (juce::Graphics& g) { g.fillAll (NeonRegistry::getTheme().background); }

    void NeonSidAudioProcessorEditor::resized()
    {
        auto bounds = getLocalBounds();
        float unitH = (float) bounds.getHeight() / 10.0f;
        auto topNav = bounds.removeFromTop ((int) unitH);
        auto bottomNav = bounds.removeFromBottom ((int) unitH);
        selectionPanelComponent->setBounds (getLocalBounds());
        if (activeModuleIndex >= 0 && activeModuleIndex < modules.size()) modules[activeModuleIndex]->setBounds (bounds);
    }

    void NeonSidAudioProcessorEditor::timerCallback() {}
}
