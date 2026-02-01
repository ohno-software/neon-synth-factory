#pragma once

#include "ModuleBase.h"

namespace neon
{
    /**
     * ModMatrixModule
     * Visualizes real-time performance controls (PitchBend, ModWheel, Aftertouch)
     * and provides the mapping matrix for these controllers.
     */
    class ModMatrixModule : public ModuleBase
    {
    public:
        ModMatrixModule (const juce::String& name, const juce::Colour& color) 
            : ModuleBase (name, color)
        {
            // The mapping parameters are added in PluginEditor or here
            moduleNameDisplay.setText("MODULATION MATRIX", juce::dontSendNotification);
        }

        void paintVisualization (juce::Graphics& g, juce::Rectangle<int> area) override
        {
            // Just a placeholder or simple graphic since meters moved to CTRL
            g.setColour (accentColor.withAlpha (0.1f));
            g.drawRoundedRectangle (area.reduced(20).toFloat(), 10.0f, 2.0f);
            
            g.setColour (accentColor.withAlpha (0.3f));
            g.setFont (juce::FontOptions().withHeight(24.0f).withStyle("Italic"));
            g.drawText ("ROUTING MATRIX", area, juce::Justification::centred);
        }

    private:
    };
}
