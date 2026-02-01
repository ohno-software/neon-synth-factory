#pragma once

#include "ModuleBase.h"

namespace neon
{
    /**
     * ControlModule
     * Houses global performance settings like Pitch Bend Range.
     */
    class ControlModule : public ModuleBase
    {
    public:
        ControlModule (const juce::String& name, const juce::Colour& color) 
            : ModuleBase (name, color)
        {
            // Row 1: PB Range, Mode, Tempo, Tempo Sync
            addParameter ("PB Range", 0.0f, 24.0f, 2.0f, false, 1.0f);
            addChoiceParameter ("Mode", {"POLY", "MONO"}, 0);
            addParameter ("Tempo", 20.0f, 300.0f, 120.0f, false, 0.0f, false, true);
            addParameter ("Tempo Sync", 0.0f, 1.0f, 1.0f, true);
            if (auto* p = parameters.back()) p->setBinaryLabels ("SELF", "HOST");

            // Row 2: Porta On, Porta Time, Porta Time/Rate, [Blank]
            addParameter ("Porta On", 0.0f, 1.0f, 0.0f, true); // NEW: Portamento on/off
            if (auto* p = parameters.back()) p->setBinaryLabels ("OFF", "ON");
            addParameter ("Porta Time", 0.0f, 5000.0f, 100.0f); // NEW: Portamento time in ms
            addParameter ("Porta Mode", 0.0f, 1.0f, 0.0f, true); // NEW: Time vs Rate mode
            if (auto* p = parameters.back()) p->setBinaryLabels ("TIME", "RATE");
            addSpacer(); // Blank

            moduleNameDisplay.setText("GLOBAL CONTROLS", juce::dontSendNotification);
        }

        void updateMeters (float pb, float mw, float at)
        {
            pitchBend = pb;
            modWheel = mw;
            aftertouch = at;
        }

        void paintVisualization (juce::Graphics& g, juce::Rectangle<int> area) override
        {
            auto displayArea = area.reduced(40);
            
            // Draw stylized performance meters (Moved from Mod Matrix to here)
            int pad = 20;
            auto meterW = (displayArea.getWidth() - (pad * 2)) / 3.0f;
            
            auto drawMeter = [&](juce::Rectangle<float> r, float val, juce::String label, bool bidirectional) {
                g.setColour (accentColor.withAlpha (0.1f));
                g.fillRoundedRectangle (r, 4.0f);
                
                auto inner = r.reduced(4);
                float h = inner.getHeight();
                
                if (bidirectional) // Pitch Bend (-1 to 1)
                {
                    float center = inner.getCentreY();
                    float barH = (std::abs(val) * (h * 0.5f));
                    auto bar = (val >= 0) ? 
                        juce::Rectangle<float>(inner.getX(), center - barH, inner.getWidth(), barH) :
                        juce::Rectangle<float>(inner.getX(), center, inner.getWidth(), barH);
                    
                    g.setColour (accentColor);
                    g.fillRect (bar);
                }
                else // Mod/AT (0 to 1)
                {
                    float barH = val * h;
                    auto bar = juce::Rectangle<float>(inner.getX(), inner.getBottom() - barH, inner.getWidth(), barH);
                    
                    g.setColour (accentColor);
                    g.fillRect (bar);
                }
                
                g.setColour (accentColor.withAlpha (0.6f));
                g.setFont (14.0f);
                g.drawText (label, r.withY(r.getBottom() + 4).withHeight(20), juce::Justification::centredTop);
            };

            drawMeter (displayArea.removeFromLeft ((int)meterW).withSizeKeepingCentre (30, 100).toFloat(), pitchBend, "PITCH", true);
            displayArea.removeFromLeft (pad);
            drawMeter (displayArea.removeFromLeft ((int)meterW).withSizeKeepingCentre (30, 100).toFloat(), modWheel, "MOD", false);
            displayArea.removeFromLeft (pad);
            drawMeter (displayArea.removeFromLeft ((int)meterW).withSizeKeepingCentre (30, 100).toFloat(), aftertouch, "AT", false);
        }

    private:
        float pitchBend = 0.0f;
        float modWheel = 0.0f;
        float aftertouch = 0.0f;
    };
}
