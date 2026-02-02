#pragma once

#include "ModuleBase.h"

namespace neon
{
    class NoiseModule : public ModuleBase
    {
    public:
        NoiseModule (const juce::String& name, const juce::Colour& color) 
            : ModuleBase (name, color)
        {
            addParameter ("Volume", 0.0f, 1.0f, 0.0f);
            addSpacer(); // Blank
            addSpacer(); // Blank
            addSpacer(); // Blank
        }

        void paintVisualization (juce::Graphics& g, juce::Rectangle<int> area) override
        {
            if (parameters.empty()) return;
            
            float volume = parameters[0]->getValue();
            
            g.setColour (accentColor.withAlpha (0.1f));
            g.fillRoundedRectangle (area.reduced(10).toFloat(), 8.0f);
            
            // Draw random noise pattern
            auto waveArea = area.reduced (20);
            int samples = 200;
            float yCenter = waveArea.getCentreY();
            float amplitude = waveArea.getHeight() * 0.3f * volume;
            
            // Use a fixed seed for consistent visualization
            juce::Random r(12345);
            
            g.setColour (accentColor.withAlpha (0.6f));
            for (int i = 0; i < samples; ++i)
            {
                float x = waveArea.getX() + (float)i / samples * waveArea.getWidth();
                float noise = r.nextFloat() * 2.0f - 1.0f;
                float y = yCenter + noise * amplitude;
                
                g.fillEllipse (x - 1, y - 1, 2, 2);
            }
            
            // Volume indicator
            g.setColour (accentColor);
            g.setFont (juce::FontOptions().withHeight(14.0f));
            g.drawText (juce::String((int)(volume * 100)) + "%", 
                       area.reduced(10), juce::Justification::topRight);
        }

    private:
    };
}
