#include "NeonToggle.h"
#include "../core/NeonColors.h"

namespace neon
{
    NeonToggle::NeonToggle()
    {
    }

    void NeonToggle::paint (juce::Graphics& g)
    {
        auto bounds = getLocalBounds().toFloat();
        
        // Bar dimensions centered vertically
        float barHeight = 45.0f;
        auto barRect = bounds.withSizeKeepingCentre (bounds.getWidth(), juce::jmin (barHeight, bounds.getHeight()));
        
        // 1. Draw Background Bar
        g.setColour (Colors::barBackground);
        g.fillRoundedRectangle (barRect, 3.0f);

        float indicatorSize = 12.0f;
        float padding = 6.0f;

        // 2. Draw Fill and Indicator based on state
        if (value > 0.5f)
        {
            // ON State: Filled bar + indicator at right
            g.setColour (accentColor);
            g.fillRoundedRectangle (barRect, 3.0f);
            
            g.setColour (juce::Colours::white);
            g.fillEllipse (barRect.getRight() - indicatorSize - padding, 
                           barRect.getCentreY() - (indicatorSize / 2.0f), 
                           indicatorSize, indicatorSize);
        }
        else
        {
            // OFF State: Empty bar + indicator at left
            g.setColour (juce::Colours::white.withAlpha (0.4f));
            g.fillEllipse (barRect.getX() + padding, 
                           barRect.getCentreY() - (indicatorSize / 2.0f), 
                           indicatorSize, indicatorSize);
        }
    }

    void NeonToggle::mouseDown (const juce::MouseEvent&)
    {
        if (isMomentary)
            setValue (1.0f);
        else
            setValue (value > 0.5f ? 0.0f : 1.0f);
    }

    void NeonToggle::mouseUp (const juce::MouseEvent&)
    {
        if (isMomentary)
            setValue (0.0f);
    }

    void NeonToggle::mouseDoubleClick (const juce::MouseEvent&)
    {
        if (onDoubleClick)
            onDoubleClick();
    }

    void NeonToggle::setValue (float newValue, bool sendNotification)
    {
        float clippedValue = newValue > 0.5f ? 1.0f : 0.0f;
        if (value != clippedValue)
        {
            value = clippedValue;
            if (sendNotification && onValueChanged)
                onValueChanged (value);
            repaint();
        }
    }
}
