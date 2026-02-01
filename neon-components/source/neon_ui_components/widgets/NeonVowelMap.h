#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace neon
{
    /**
     * A 2D XY-pad for morphing between vowel formants.
     * Displays a phonetic map with "phonetic markers" (A, E, I, O, U).
     */
    class VowelMap : public juce::Component
    {
    public:
        VowelMap();
        ~VowelMap() override = default;

        void paint (juce::Graphics& g) override;
        void resized() override;

        void mouseDown (const juce::MouseEvent& event) override;
        void mouseDrag (const juce::MouseEvent& event) override;

        /** Set the current position [0.0, 1.0] */
        void setPosition (juce::Point<float> newPos);
        juce::Point<float> getPosition() const { return currentPos; }

        /** Callback when position changes */
        std::function<void(juce::Point<float>)> onPositionChanged;

    private:
        void updatePosition (juce::Point<float> screenPos);

        juce::Point<float> currentPos { 0.5f, 0.5f };
        
        struct VowelPoint {
            juce::String label;
            juce::Point<float> pos; // Normalized 0-1
        };
        
        juce::Array<VowelPoint> vowelMarkers;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VowelMap)
    };
}
