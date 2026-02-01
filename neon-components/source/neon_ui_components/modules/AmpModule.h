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
            addParameter ("Level",  0.0f, 1.0f, 0.8f);
            addParameter ("Pan",    -1.0f, 1.0f, 0.0f);
            addParameter ("Spread", 0.0f, 1.0f, 0.0f);
            addParameter ("Velocity", 0.0f, 1.0f, 0.5f);
            
            lastAdjustedIndex = 0;
        }

    protected:
        void paintVisualization (juce::Graphics& g, juce::Rectangle<int> area) override
        {
            auto r = area.reduced (150, 100).toFloat();
            float level = parameters[0]->getValue();
            float pan = parameters[1]->getValue();

            // VU Meter Style visualization
            g.setColour (accentColor.withAlpha (0.1f));
            g.fillRect (r);
            
            float meterW = 40.0f;
            auto leftMeter = r.withWidth (meterW).withX (r.getCentreX() - meterW - 10);
            auto rightMeter = r.withWidth (meterW).withX (r.getCentreX() + 10);

            float L = level * juce::jmap (pan, 1.0f, -1.0f, 0.0f, 1.0f);
            float R = level * juce::jmap (pan, -1.0f, 1.0f, 0.0f, 1.0f);

            g.setColour (accentColor);
            g.fillRect (leftMeter.withHeight (leftMeter.getHeight() * L).withY (leftMeter.getBottom() - leftMeter.getHeight() * L));
            g.fillRect (rightMeter.withHeight (rightMeter.getHeight() * R).withY (rightMeter.getBottom() - rightMeter.getHeight() * R));
        }
    };
}
