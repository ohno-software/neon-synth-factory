#include "NeonVowelMap.h"
#include "../core/NeonColors.h"

namespace neon
{
    VowelMap::VowelMap()
    {
        // Default vowel positions (Triangle/Circle approximation)
        vowelMarkers.add ({ "u", { 0.1f, 0.1f } });
        vowelMarkers.add ({ "o", { 0.9f, 0.2f } });
        vowelMarkers.add ({ "a", { 0.5f, 0.9f } });
        vowelMarkers.add ({ "e", { 0.1f, 0.8f } });
        vowelMarkers.add ({ "i", { 0.05f, 0.5f } });
    }

    void VowelMap::paint (juce::Graphics& g)
    {
        auto bounds = getLocalBounds().toFloat();
        
        // Background Grid
        g.setColour (Colors::cardBackground);
        g.fillRoundedRectangle (bounds, 4.0f);
        
        g.setColour (Colors::barBackground);
        for (float i = 0.2f; i < 1.0f; i += 0.2f)
        {
            g.drawHorizontalLine (int(bounds.getHeight() * i), 0.0f, bounds.getWidth());
            g.drawVerticalLine (int(bounds.getWidth() * i), 0.0f, bounds.getHeight());
        }

        // Vowel Markers
        g.setFont (16.0f);
        for (auto& v : vowelMarkers)
        {
            auto x = v.pos.x * bounds.getWidth();
            auto y = v.pos.y * bounds.getHeight();
            
            g.setColour (Colors::textDim);
            g.drawText (v.label, int(x - 10), int(y - 10), 20, 20, juce::Justification::centered);
        }

        // Current Position Indicator (The "Phonmetic Puck")
        auto puckX = currentPos.x * bounds.getWidth();
        auto puckY = currentPos.y * bounds.getHeight();
        
        g.setColour (Colors::indicator);
        g.fillEllipse (puckX - 6.0f, puckY - 6.0f, 12.0f, 12.0f);
        
        // Outer Glow
        g.setColour (Colors::indicator.withAlpha (0.3f));
        g.drawEllipse (puckX - 10.0f, puckY - 10.0f, 20.0f, 20.0f, 2.0f);
    }

    void VowelMap::resized() {}

    void VowelMap::mouseDown (const juce::MouseEvent& event)
    {
        updatePosition (event.position);
    }

    void VowelMap::mouseDrag (const juce::MouseEvent& event)
    {
        updatePosition (event.position);
    }

    void VowelMap::updatePosition (juce::Point<float> screenPos)
    {
        auto bounds = getLocalBounds().toFloat();
        currentPos.x = juce::jlimit (0.0f, 1.0f, screenPos.x / bounds.getWidth());
        currentPos.y = juce::jlimit (0.0f, 1.0f, screenPos.y / bounds.getHeight());
        
        if (onPositionChanged)
            onPositionChanged (currentPos);
            
        repaint();
    }

    void VowelMap::setPosition (juce::Point<float> newPos)
    {
        currentPos.x = juce::jlimit (0.0f, 1.0f, newPos.x);
        currentPos.y = juce::jlimit (0.0f, 1.0f, newPos.y);
        repaint();
    }
}
