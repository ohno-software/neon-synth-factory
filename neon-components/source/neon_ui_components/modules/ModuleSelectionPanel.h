#pragma once

#include "../core/NeonLookAndFeel.h"

namespace neon
{
    /**
     * @class ModuleSelectionPanel
     * @brief A top-bar selection area with 8 buttons for module switching.
     */
    class ModuleSelectionPanel : public juce::Component
    {
    public:
        ModuleSelectionPanel()
        {
            // Create 12 placeholder buttons (enough for current modules + some expansion)
            for (int i = 0; i < 12; ++i)
            {
                auto btn = std::make_unique<juce::TextButton> ("Module " + juce::String (i + 1));
                btn->setClickingTogglesState (true);
                btn->setRadioGroupId (1001);
                
                btn->onClick = [this, i] { if (onModuleChanged) onModuleChanged (i); };
                
                addChildComponent (btn.get());
                buttons.add (std::move (btn));
            }
            
            // Initial appearance (will be overridden by setModuleNames)
            buttons[0]->setButtonText ("OSC 1");
            buttons[0]->setToggleState (true, juce::dontSendNotification);
        }

        void setModuleNames (const juce::StringArray& names)
        {
            numVisibleButtons = names.size();
            for (int i = 0; i < buttons.size(); ++i)
            {
                if (i < names.size())
                {
                    buttons[i]->setButtonText (names[i]);
                    buttons[i]->setVisible (true);
                }
                else
                {
                    buttons[i]->setVisible (false);
                }
            }
            resized();
        }

        void selectIndex (int index, bool sendNotification)
        {
            for (int i = 0; i < buttons.size(); ++i)
            {
                if (i == index)
                    buttons[i]->setToggleState (true, sendNotification ? juce::sendNotification : juce::dontSendNotification);
                else
                    buttons[i]->setToggleState (false, juce::dontSendNotification);
            }
        }

        std::function<void (int)> onModuleChanged;

        void resized() override
        {
            auto bounds = getLocalBounds();
            if (numVisibleButtons > 0)
            {
                int btnWidth = bounds.getWidth() / numVisibleButtons;
                for (int i = 0; i < numVisibleButtons; ++i)
                {
                    buttons[i]->setBounds (i * btnWidth, 0, btnWidth, bounds.getHeight());
                }
            }
        }

    private:
        juce::OwnedArray<juce::TextButton> buttons;
        int numVisibleButtons = 0;
    };
}
