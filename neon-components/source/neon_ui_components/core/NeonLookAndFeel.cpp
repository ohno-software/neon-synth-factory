#include "NeonLookAndFeel.h"
#include "NeonColors.h"

namespace neon
{
    LookAndFeel::LookAndFeel()
    {
        // Set default colors for the look and feel
        setColour (juce::Slider::backgroundColourId, Colors::barBackground);
        setColour (juce::Slider::thumbColourId, Colors::indicator);
        setColour (juce::Slider::trackColourId, Colors::barBackground);
        setColour (juce::TextButton::buttonColourId, Colors::cardBackground);
        setColour (juce::Label::textColourId, Colors::textBright);
    }

    juce::Font LookAndFeel::getDisplayFont() const
    {
        /* Placeholder for 7-segment font until BinaryData is properly linked/generated */
        return juce::Font (juce::FontOptions ().withHeight (14.0f).withName (juce::Font::getDefaultMonospacedFontName()));
    }

    void LookAndFeel::drawLinearSlider (juce::Graphics& g, int x, int y, int width, int height,
                                       float sliderPos, float minSliderPos, float maxSliderPos,
                                       const juce::Slider::SliderStyle style, juce::Slider& slider)
    {
        if (style != juce::Slider::LinearHorizontal)
        {
            LookAndFeel_V4::drawLinearSlider (g, x, y, width, height, sliderPos, minSliderPos, maxSliderPos, style, slider);
            return;
        }

        auto bounds = juce::Rectangle<int> (x, y, width, height).toFloat();
        
        // Bar dimensions from design spec (centered vertically in bounds)
        // Ensure we don't exceed the provided height
        float barHeight = 45.0f;
        auto barRect = bounds.withSizeKeepingCentre (bounds.getWidth(), juce::jmin (barHeight, bounds.getHeight()));
        
        // 1. Draw Background Bar
        g.setColour (slider.findColour (juce::Slider::backgroundColourId));
        g.fillRoundedRectangle (barRect, 3.0f);

        // 2. Draw Fill (from left to current position)
        float fillWidth = sliderPos - (float)x;
        if (fillWidth > 0)
        {
            auto fillRect = barRect.withWidth (fillWidth);
            g.setColour (slider.findColour (juce::Slider::thumbColourId));
            g.fillRoundedRectangle (fillRect, 3.0f);
        }

        // 3. Draw Value Indicator (White Circle)
        g.setColour (juce::Colours::white);
        float indicatorSize = 10.0f;
        g.fillEllipse (sliderPos - (indicatorSize / 2.0f), 
                       barRect.getCentreY() - (indicatorSize / 2.0f), 
                       indicatorSize, indicatorSize);
    }

    void LookAndFeel::drawButtonBackground (juce::Graphics& g, juce::Button& button, const juce::Colour& backgroundColour,
                                           bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown)
    {
        auto bounds = button.getLocalBounds().toFloat();
        g.setColour (backgroundColour.withMultipliedAlpha (button.isEnabled() ? 1.0f : 0.5f));
        g.fillRoundedRectangle (bounds, 4.0f);
        
        if (shouldDrawButtonAsDown)
            g.fillAll (juce::Colours::black.withAlpha (0.2f));
    }

    void LookAndFeel::drawComboBox (juce::Graphics& g, int width, int height, bool isButtonDown,
                                   int buttonX, int buttonY, int buttonWidth, int buttonHeight, juce::ComboBox& box)
    {
        auto bounds = juce::Rectangle<int> (width, height).toFloat();
        
        // Match the NeonBar/Slider background
        g.setColour (juce::Colours::black);
        g.fillRoundedRectangle (bounds, 4.0f);
        
        g.setColour (juce::Colours::white.withAlpha (0.2f));
        g.drawRoundedRectangle (bounds, 4.0f, 2.0f);

        // Arrow area
        auto arrowZone = juce::Rectangle<int> (buttonX, buttonY, buttonWidth, buttonHeight).toFloat();
        g.setColour (juce::Colours::white.withAlpha (0.5f));
        
        juce::Path path;
        path.startNewSubPath (arrowZone.getCentreX() - 5, arrowZone.getCentreY() - 3);
        path.lineTo (arrowZone.getCentreX(), arrowZone.getCentreY() + 3);
        path.lineTo (arrowZone.getCentreX() + 5, arrowZone.getCentreY() - 3);
        g.strokePath (path, juce::PathStrokeType (2.0f));
    }

    juce::Font LookAndFeel::getTextButtonFont (juce::TextButton&, int buttonHeight)
    {
        return juce::Font (juce::FontOptions ().withHeight (juce::jmin (32.0f, (float)buttonHeight * 0.6f)));
    }
}
