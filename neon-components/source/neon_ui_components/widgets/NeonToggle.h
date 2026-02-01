#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace neon
{
    /**
     * NeonToggle
     * A specialized toggle component for binary parameters.
     * Click to toggle between 0.0 and 1.0. No dragging.
     */
    class NeonToggle : public juce::Component
    {
    public:
        NeonToggle();
        ~NeonToggle() override = default;

        void paint (juce::Graphics& g) override;
        void mouseDown (const juce::MouseEvent& e) override;
        void mouseUp (const juce::MouseEvent& e) override;
        void mouseDoubleClick (const juce::MouseEvent& e) override;

        void setValue (float newValue, bool sendNotification = true);
        float getValue() const { return value; }

        void setIsMomentary (bool momentary) { isMomentary = momentary; }

        std::function<void (float)> onValueChanged;
        std::function<void()> onDoubleClick;
        
        void setAccentColor (juce::Colour color) { accentColor = color; repaint(); }

    private:
        float value = 1.0f;
        bool isMomentary = false;
        juce::Colour accentColor = juce::Colours::orange;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NeonToggle)
    };
}
