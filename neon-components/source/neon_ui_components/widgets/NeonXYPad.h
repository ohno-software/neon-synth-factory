#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace neon
{
    /**
     * A 2D XY-pad with optional physics support.
     */
    class XYPad : public juce::Component
    {
    public:
        XYPad();
        ~XYPad() override = default;

        void paint (juce::Graphics& g) override;
        void resized() override;
        
        void mouseDown (const juce::MouseEvent& event) override;
        void mouseDrag (const juce::MouseEvent& event) override;
        void mouseUp (const juce::MouseEvent& event) override;

        /** Set the current position [0.0, 1.0] */
        void setPosition (juce::Point<float> newPos, bool sendNotification = true);
        juce::Point<float> getPosition() const { return currentPos; }

        /** Physics: Spring behavior (snaps to center) */
        void setSpringEnabled (bool enabled) { springEnabled = enabled; }
        
        std::function<void(juce::Point<float>)> onPositionChanged;

    private:
        void updatePosition (juce::Point<float> screenPos);

        juce::Point<float> currentPos { 0.5f, 0.5f };
        bool springEnabled = false;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (XYPad)
    };
}
