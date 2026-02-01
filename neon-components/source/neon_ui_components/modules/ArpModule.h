#pragma once

#include "ModuleBase.h"

namespace neon
{
    class ArpModule : public ModuleBase
    {
    public:
        ArpModule (const juce::String& name, const juce::Colour& color) 
            : ModuleBase (name, color)
        {
            addParameter ("Arp On", 0.0f, 1.0f, 0.0f, true);

            std::vector<juce::String> divisions = { 
                "1/64", "1/32", "1/16", "1/8", "1/4", "1/2", "1/1", "2/1", "4/1" 
            };
            addChoiceParameter ("Rate Note", divisions, 2); // Default 1/16

            std::vector<juce::String> modes = { "UP", "DOWN", "UP/DOWN", "RANDOM" };
            addChoiceParameter ("Mode", modes, 0);

            addParameter ("Octave", 1.0f, 4.0f, 1.0f, false, 1.0f, false, true);
            addParameter ("Gate", 1.0f, 100.0f, 80.0f, false, 1.0f, false, true);
            addParameter ("Latch", 0.0f, 1.0f, 0.0f, true);
            
            moduleNameDisplay.setText("ARPEGGIATOR", juce::dontSendNotification);
            lastAdjustedIndex = 2; // Start with Mode highlighted
        }

        void paintVisualization (juce::Graphics& g, juce::Rectangle<int> area) override
        {
            auto r = area.reduced (100, 80).toFloat();
            g.setColour (accentColor.withAlpha (0.05f));
            g.fillRoundedRectangle (r, 8.0f);
            g.setColour (accentColor.withAlpha (0.15f));
            g.drawRoundedRectangle (r, 8.0f, 1.0f);

            // Draw some stylized "steps" or "notes"
            float w = r.getWidth();
            float h = r.getHeight();
            float stepW = w / 8.0f;
            int mode = (int)std::round (parameters[2]->getValue());

            g.setColour (accentColor);

            for (int i = 0; i < 8; ++i)
            {
                float stepX = r.getX() + (i * stepW) + 2;
                float barW = stepW - 4;
                float val = 0.5f;

                if (mode == 0)      val = 0.2f + (i % 4) * 0.2f; // Up
                else if (mode == 1) val = 0.8f - (i % 4) * 0.2f; // Down
                else if (mode == 2) val = (i < 4) ? (0.2f + i * 0.2f) : (0.8f - (i-4) * 0.2f); // Up/Down
                else                val = juce::Random::getSystemRandom().nextFloat() * 0.6f + 0.2f;

                float barH = val * h;
                g.fillRect (stepX, r.getBottom() - barH, barW, barH);
            }
        }
    };
}
