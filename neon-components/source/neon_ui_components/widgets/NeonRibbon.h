#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace neon
{
    /**
     * A horizontal touch-strip for expressive modulation.
     * Supports Absolute and Relative modes.
     */
    class Ribbon : public juce::Component
    {
    public:
        Ribbon();
        ~Ribbon() override = default;

        void paint (juce::Graphics& g) override;
        void mouseDown (const juce::MouseEvent& event) override;
        void mouseDrag (const juce::MouseEvent& event) override;
        void mouseUp (const juce::MouseEvent& event) override;

        /** Set the current value [0.0, 1.0] */
        void setValue (float newValue, bool sendNotification = true);
        float getValue() const { return currentValue; }

        /** Mode selection: Absolute vs Relative */
        void setRelativeMode (bool isRelative) { relativeMode = isRelative; }
        
        /** Snap back behavior */
        void setResetOnRelease (bool shouldReset) { resetOnRelease = shouldReset; }

        std::function<void(float)> onValueChanged;

    private:
        float currentValue = 0.5f;
        float lastValue = 0.5f;
        bool relativeMode = false;
        bool resetOnRelease = false;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Ribbon)
    };
}
