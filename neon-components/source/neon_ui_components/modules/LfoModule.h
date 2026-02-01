#pragma once

#include "ModuleBase.h"

namespace neon
{
    class LfoModule : public ModuleBase
    {
    public:
        LfoModule (const juce::String& name, const juce::Colour& color) 
            : ModuleBase (name, color)
        {
            // Page 1: Main LFO controls
            std::vector<juce::String> shapes = { "TRIANGLE", "RAMP UP", "RAMP DOWN", "SQUARE", "S&H" };
            addChoiceParameter ("Shape", shapes, 0);
            
            addParameter ("Sync", 0.0f, 1.0f, 0.0f, true); // Hz / Note
            if (auto* p = parameters.back()) p->setBinaryLabels ("HZ", "NOTE");

            addParameter ("Rate Hz", 0.2f, 20.0f, 1.0f); // 0.2-20 Hz with default skew
            
            std::vector<juce::String> divisions = { 
                "1/64", "1/32", "1/16", "1/8", "1/4", "1/2", "1/1", "2/1", "4/1" 
            };
            addChoiceParameter ("Rate Note", divisions, 4); // Default 1/4

            addParameter ("KeySync", 0.0f, 1.0f, 1.0f, true);
            if (auto* p = parameters.back()) p->setBinaryLabels ("OFF", "ON");

            addParameter ("Phase", 0.0f, 360.0f, 0.0f, false, 0.0f, false, true);
            addParameter ("Delay", 0.0f, 5000.0f, 0.0f, false, 0.0f, false, true); // Delay in ms
            addParameter ("Fade", 0.0f, 5000.0f, 0.0f, false, 0.0f, false, true);  // NEW: Fade-in time in ms

            // Page 2: Modulation Targets (All 4 on one page)
            // To ensure they are together, we add 0 spacers (8 used already - perfect fit)

            // Targets
            std::vector<juce::String> targets = getModTargetNames();

            for (int i = 1; i <= 4; ++i)
            {
                addChoiceParameter ("Slot " + juce::String(i) + " Target", targets, 0);
                addParameter       ("Slot " + juce::String(i) + " Amount", -100.0f, 100.0f, 0.0f);
            }
            
            lastAdjustedIndex = 0;
        }

        void paintVisualization (juce::Graphics& g, juce::Rectangle<int> area) override
        {
            auto r = area.reduced (120, 100).toFloat();
            g.setColour (accentColor.withAlpha (0.05f));
            g.fillRect (r);
            g.setColour (accentColor.withAlpha (0.15f));
            g.drawRect (r, 1.0f);

            int shape = (int)std::round (parameters[0]->getValue());
            juce::Path p;
            float w = r.getWidth();
            float h = r.getHeight();
            float cx = r.getX();
            float cy = r.getY();
            float midY = r.getCentreY();

            g.setColour (accentColor);

            if (shape == 0) // Triangle
            {
                p.startNewSubPath (cx, midY + h * 0.3f);
                p.lineTo (cx + w * 0.25f, midY - h * 0.3f);
                p.lineTo (cx + w * 0.5f,  midY + h * 0.3f);
                p.lineTo (cx + w * 0.75f, midY - h * 0.3f);
                p.lineTo (cx + w,         midY + h * 0.3f);
            }
            else if (shape == 1) // Ramp Up
            {
                p.startNewSubPath (cx, midY + h * 0.3f);
                p.lineTo (cx + w * 0.5f, midY - h * 0.3f);
                p.lineTo (cx + w * 0.5f, midY + h * 0.3f);
                p.lineTo (cx + w, midY - h * 0.3f);
            }
            else if (shape == 2) // Ramp Down
            {
                p.startNewSubPath (cx, midY - h * 0.3f);
                p.lineTo (cx + w * 0.5f, midY + h * 0.3f);
                p.lineTo (cx + w * 0.5f, midY - h * 0.3f);
                p.lineTo (cx + w, midY + h * 0.3f);
            }
            else if (shape == 3) // Square
            {
                p.startNewSubPath (cx, midY + h * 0.3f);
                p.lineTo (cx + w * 0.25f, midY + h * 0.3f);
                p.lineTo (cx + w * 0.25f, midY - h * 0.3f);
                p.lineTo (cx + w * 0.5f,  midY - h * 0.3f);
                p.lineTo (cx + w * 0.5f,  midY + h * 0.3f);
                p.lineTo (cx + w * 0.75f, midY + h * 0.3f);
                p.lineTo (cx + w * 0.75f, midY - h * 0.3f);
                p.lineTo (cx + w,         midY - h * 0.3f);
            }
            else // S&H
            {
                p.startNewSubPath (cx, midY);
                for (int i = 1; i < 6; ++i)
                {
                    float rx = cx + (w * i / 5.0f);
                    float ry = cy + (h * (0.2f + (rand() % 100) / 150.0f));
                    p.lineTo (rx, midY); // staircase-ish
                    p.lineTo (rx, ry);
                }
            }

            g.strokePath (p, juce::PathStrokeType (2.0f));
        }
    };
}
