#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace neon
{
    /**
     * Primary LookAndFeel for all Neon Synth Factory components.
     * Customizes standard JUCE widgets to fit the cyber-retro aesthetic.
     */
    class LookAndFeel : public juce::LookAndFeel_V4
    {
    public:
        LookAndFeel();
        ~LookAndFeel() override = default;

        /** Returns the specialized 7-segment font used for parameter values. */
        juce::Font getDisplayFont() const;

        // Slider Overrides
        void drawLinearSlider (juce::Graphics&, int x, int y, int width, int height,
                               float sliderPos, float minSliderPos, float maxSliderPos,
                               const juce::Slider::SliderStyle, juce::Slider&) override;

        // Button Overrides
        void drawButtonBackground (juce::Graphics&, juce::Button&, const juce::Colour& backgroundColour,
                                   bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;
        
        void drawComboBox (juce::Graphics& g, int width, int height, bool isButtonDown,
                           int buttonX, int buttonY, int buttonWidth, int buttonHeight, juce::ComboBox& box) override;

        juce::Font getTextButtonFont (juce::TextButton&, int buttonHeight) override;

    private:
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LookAndFeel)
    };
}
