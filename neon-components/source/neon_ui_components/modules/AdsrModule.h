#pragma once

#include "ModuleBase.h"

namespace neon
{
    /**
     * @class AdsrModule
     * @brief A standard 4-parameter envelope module (Attack, Decay, Sustain, Release)
     */
    class AdsrModule : public ModuleBase
    {
    public:
        AdsrModule (const juce::String& name, const juce::Colour& color = juce::Colours::cyan) 
            : ModuleBase (name, color)
        {
            addParameter ("Attack",  0.0f, 10000.0f, 100.0f);
            addParameter ("Decay",   0.0f, 10000.0f, 500.0f);
            addParameter ("Sustain", 0.0f, 1.0f,     0.7f);
            addParameter ("Release", 0.0f, 10000.0f, 1000.0f);
            
            lastAdjustedIndex = 0;
        }
        
    protected:
        void paintVisualization (juce::Graphics& g, juce::Rectangle<int> area) override
        {
            auto r = area.reduced (120, 100).toFloat();
            g.setColour (accentColor.withAlpha (0.05f));
            g.fillRect (r);
            
            auto getNormTime = [&](int idx) { 
                return juce::jmap (parameters[idx]->getValue(), 0.0f, 10000.0f, 0.01f, 0.3f); 
            };
            
            float a   = getNormTime(0);
            float dec = getNormTime(1);
            float s   = parameters[2]->getValue();
            float rel = getNormTime(3);

            float total = a + dec + 0.2f + rel; 
            float scaleX = r.getWidth() / total;
            
            float x = r.getX();
            float y0 = r.getBottom();
            float y1 = r.getY();

            juce::Path path;
            path.startNewSubPath (x, y0);
            x += a * scaleX;
            path.lineTo (x, y1);
            x += dec * scaleX;
            path.lineTo (x, y0 - s * (y0 - y1));
            x += 0.2f * scaleX;
            path.lineTo (x, y0 - s * (y0 - y1));
            x += rel * scaleX;
            path.lineTo (x, y0);

            g.setColour (accentColor);
            g.strokePath (path, juce::PathStrokeType (4.0f));
            
            // Highlight active segment
            if (lastAdjustedIndex >= 0 && lastAdjustedIndex < (int)parameters.size())
            {
                g.setColour (juce::Colours::yellow);
                // Simple logic: we'd need to isolate the path segment, 
                // but for now we'll just glow the whole thing if adjusting.
                g.setOpacity (0.4f);
                g.strokePath (path, juce::PathStrokeType (8.0f));
            }
        }
    };
}
