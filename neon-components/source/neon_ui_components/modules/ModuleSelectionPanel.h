#pragma once

#include "../core/NeonLookAndFeel.h"

namespace neon
{
    /**
     * @class ModuleSelectionPanel
     * @brief Two-row module selection with category buttons (top) and module buttons (bottom).
     */
    class ModuleSelectionPanel : public juce::Component
    {
    public:
        ModuleSelectionPanel()
        {
            // Top row: 8 slots for category buttons (5 categories + 3 blanks)
            for (int i = 0; i < 8; ++i)
            {
                auto btn = std::make_unique<juce::TextButton>();
                btn->setClickingTogglesState (true);
                btn->setRadioGroupId (2001);
                
                if (i < 5) // Only first 5 are actual categories
                {
                    int catIndex = i;
                    btn->onClick = [this, catIndex] { 
                        setActiveCategory (catIndex);
                        updateButtonColors();
                        // Trigger module display for first module in category
                        if (!categoryModules[catIndex].empty() && onModuleChanged)
                            onModuleChanged (categoryModules[catIndex][0]);
                    };
                    addAndMakeVisible (btn.get());
                }
                else
                {
                    btn->setVisible (false); // Blank slots
                    addChildComponent (btn.get());
                }
                
                categoryButtons.add (std::move (btn));
            }
            
            // Bottom row: 8 slots for module buttons
            for (int i = 0; i < 8; ++i)
            {
                auto btn = std::make_unique<juce::TextButton>();
                btn->setClickingTogglesState (true);
                btn->setRadioGroupId (2002);
                
                int slotIndex = i;
                btn->onClick = [this, slotIndex] {
                    if (currentModuleIndices[slotIndex] >= 0)
                    {
                        // Manually untoggle all other module buttons first
                        for (int j = 0; j < 8; ++j)
                        {
                            if (j != slotIndex)
                                moduleButtons[j]->setToggleState (false, juce::dontSendNotification);
                        }
                        moduleButtons[slotIndex]->setToggleState (true, juce::dontSendNotification);
                        
                        updateButtonColors();
                        
                        if (onModuleChanged) 
                            onModuleChanged (currentModuleIndices[slotIndex]);
                    }
                };
                addChildComponent (btn.get());
                moduleButtons.add (std::move (btn));
            }
            
            // Setup category names
            categoryButtons[0]->setButtonText ("OSC");
            categoryButtons[1]->setButtonText ("FILTER");
            categoryButtons[2]->setButtonText ("AMP");
            categoryButtons[3]->setButtonText ("M/FX");
            categoryButtons[4]->setButtonText ("MAIN");
            
            // Setup category mappings: category index -> array of module indices
            // OSC -> OSC1(0), OSC2(1), P-ENV(4)
            categoryModules[0] = {0, 1, 4};
            // FILTER -> FILTER(2), F-ENV(5)
            categoryModules[1] = {2, 5};
            // AMP -> AMP(3), A-ENV(7)
            categoryModules[2] = {3, 7};
            // M/FX -> MOD(12), FX(13)
            categoryModules[3] = {12, 13};
            // MAIN -> LIB(15), CTRL(14)
            categoryModules[4] = {15, 14};
            
            // Default to MAIN category
            setActiveCategory (4);
            categoryButtons[4]->setToggleState (true, juce::dontSendNotification);
            updateButtonColors();
        }

        void setActiveCategory (int categoryIndex)
        {
            if (categoryIndex < 0 || categoryIndex >= 5) return;
            
            currentCategory = categoryIndex;
            const auto& moduleList = categoryModules[categoryIndex];
            
            // Clear all module indices
            for (int i = 0; i < 8; ++i)
                currentModuleIndices[i] = -1;
            
            // Setup module buttons for this category
            for (int i = 0; i < 8; ++i)
            {
                if (i < moduleList.size())
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
                // Explicitly untoggle all buttons first
                moduleButtons[i]->setToggleState (false, juce::dontSendNotification);
            }
            
            // Auto-select first module in category
            if (!moduleList.empty())
            {
                moduleButtons[0]->setToggleState (true, juce::dontSendNotification);
                updateButtonColors();
            }
            
            resized();
        }

        void selectModuleByIndex (int globalModuleIndex, bool sendNotification)
        {
            // Find which category this module belongs to
            int targetCategory = -1;
            int targetButton = -1;
            
            for (int cat = 0; cat < 5; ++cat)
            {
                const auto& moduleList = categoryModules[cat];
                for (int i = 0; i < moduleList.size(); ++i)
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
                // Switch category if needed
                if (currentCategory != targetCategory)
                {
                    categoryButtons[targetCategory]->setToggleState (true, juce::dontSendNotification);
                    setActiveCategory (targetCategory);
                }
                
                // Explicitly untoggle all module buttons first
                for (int i = 0; i < 8; ++i)
                    moduleButtons[i]->setToggleState (false, juce::dontSendNotification);
                
                // Select target module button
                moduleButtons[targetButton]->setToggleState (true, sendNotification ? juce::sendNotification : juce::dontSendNotification);
                updateButtonColors();
            }
        }

        std::function<void (int)> onModuleChanged;  // Callback with global module index
        std::function<void (int)> onCategoryChanged; // Callback with category index

        void resized() override
        {
            auto bounds = getLocalBounds();
            float unitH = (float)bounds.getHeight() / 10.0f;
            int slotWidth = bounds.getWidth() / 8;
            
            // Top row: Category buttons at y=0 (first 1/10)
            for (int i = 0; i < 8; ++i)
            {
                categoryButtons[i]->setBounds (i * slotWidth, 0, slotWidth, (int)unitH);
            }
            
            // Bottom row: Module buttons at y=9/10 (last 1/10)
            int bottomY = (int)(unitH * 9);
            for (int i = 0; i < 8; ++i)
            {
                moduleButtons[i]->setBounds (i * slotWidth, bottomY, slotWidth, (int)unitH);
            }
        }
        
        void updateButtonColors()
        {
            auto magenta = juce::Colour (0xFFFF00FF); // Bright magenta
            auto grey = juce::Colour (0xFF808080); // Grey for unselected
            
            // Update category button colors
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
            
            // Update module button colors
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
        
        int currentCategory = 4; // Default to MAIN
        std::map<int, std::vector<int>> categoryModules; // category index -> module indices
        int currentModuleIndices[8] = {-1, -1, -1, -1, -1, -1, -1, -1};
        
        juce::String getModuleName (int index)
        {
            const char* names[] = {
                "OSC 1", "OSC 2", "FILTER", "AMP", "P-ENV", "F-ENV", "M-ENV", "A-ENV",
                "LFO 1", "LFO 2", "LFO 3", "ARP", "MOD", "FX", "CTRL", "LIB"
            };
            if (index >= 0 && index < 16) return names[index];
            return "???";
        }
    };
}
