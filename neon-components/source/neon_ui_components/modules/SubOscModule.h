#pragma once

#include "ModuleBase.h"

namespace neon
{
    class SubOscModule : public ModuleBase
    {
    public:
        SubOscModule (const juce::String& name, const juce::Colour& color) 
            : ModuleBase (name, color)
        {
            // PAGE 1: Row 1: Octave, Volume, Pan, [Blank]
            std::vector<juce::String> octaves = { "-2 Oct", "-1 Oct", "-0.5 Oct" };
            addChoiceParameter ("Octave", octaves, 1); // Default to -1 Oct
            
            addParameter ("Volume", 0.0f, 1.0f, 0.0f);
            addParameter ("Pan", -1.0f, 1.0f, 0.0f);
            addSpacer(); // Blank
            
            lastAdjustedIndex = 0;
        }

        void paintVisualization (juce::Graphics& g, juce::Rectangle<int> area) override
        {
            auto r = area.reduced (120, 100).toFloat();
            g.setColour (accentColor.withAlpha (0.05f));
            g.fillRect (r);
            g.setColour (accentColor.withAlpha (0.15f));
            g.drawRect (r, 1.0f);

            // Draw simple sine wave for sub oscillator
            juce::Path p;
            float w = r.getWidth();
            float h = r.getHeight();
            float cx = r.getX();
            float midY = r.getCentreY();
            
            g.setColour (accentColor);
            
            int numSteps = 200;
            for (int i = 0; i <= numSteps; ++i)
            {
                float t = i / (float)numSteps;
                float x = cx + t * w;
                float y = midY + std::sin (t * juce::MathConstants<float>::twoPi) * h * 0.3f;
                
                if (i == 0)
                    p.startNewSubPath (x, y);
                else
                    p.lineTo (x, y);
            }
            
            g.strokePath (p, juce::PathStrokeType (2.0f));
        }

    private:
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SubOscModule)
    };
}
