#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace neon
{
    /**
     * NeonSelectionPanel
     * The navigational block diagram.
     */
    class SelectionPanel : public juce::Component
    {
    public:
        SelectionPanel();
        ~SelectionPanel() override = default;

        void paint (juce::Graphics&) override;
        void resized() override;
        
        std::function<void (int)> onModuleSelected;

    private:
        struct ModuleButton : public juce::Button
        {
            ModuleButton (const juce::String& name);
            void paintButton (juce::Graphics&, bool mouseOver, bool isDown) override;
        };
        
        juce::OwnedArray<ModuleButton> buttons;
        
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SelectionPanel)
    };
}
