#pragma once

#include "ModuleBase.h"

namespace neon
{
    class AmpModule : public ModuleBase
    {
    public:
        AmpModule (const juce::String& name, const juce::Colour& color) 
            : ModuleBase (name, color)
        {
            // Row 1: Level, Velocity, Aftertouch, [Blank]
            addParameter ("Level",  0.0f, 1.0f, 0.8f);
            addParameter ("Velocity", 0.0f, 1.0f, 0.5f);
            addParameter ("Aftertouch", 0.0f, 1.0f, 0.0f); // NEW: Amp aftertouch sensitivity
            addSpacer(); // Blank
            
            // Row 2: [All Blank]
            addSpacer();
            addSpacer();
            addSpacer();
            addSpacer();
            
            lastAdjustedIndex = 0;
        }

    protected:
        void paintVisualization (juce::Graphics& g, juce::Rectangle<int> area) override
        {
            auto r = area.reduced (150, 100).toFloat();
            float level = parameters[0]->getValue();

            // VU Meter Style visualization (simplified without pan)
            g.setColour (accentColor.withAlpha (0.1f));
            g.fillRect (r);
            
            float meterW = 40.0f;
            auto leftMeter = r.withWidth (meterW).withX (r.getCentreX() - meterW - 10);
            auto rightMeter = r.withWidth (meterW).withX (r.getCentreX() + 10);

            // Equal level for both channels
            float L = level;
            float R = level;

            g.setColour (accentColor);
            g.fillRect (leftMeter.withHeight (leftMeter.getHeight() * L).withY (leftMeter.getBottom() - leftMeter.getHeight() * L));
            g.fillRect (rightMeter.withHeight (rightMeter.getHeight() * R).withY (rightMeter.getBottom() - rightMeter.getHeight() * R));
        }
    };
}
