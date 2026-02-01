#pragma once

#include "ModuleBase.h"

namespace neon
{
    class FxModule : public ModuleBase
    {
    public:
        FxModule (const juce::String& name, const juce::Colour& color) 
            : ModuleBase (name, color)
        {
            // PAGE 1: Modulation FX
            // Row 1: Mod Type, Mod Rate, Mod Depth, Mod Feedback
            std::vector<juce::String> modTypes = { "OFF", "CHORUS", "PHASER", "FLANGER" };
            addChoiceParameter ("Mod Type", modTypes, 1);
            addParameter ("Mod Rate", 0.1f, 10.0f, 1.0f);
            addParameter ("Mod Depth", 0.0f, 1.0f, 0.5f);
            addParameter ("Mod Feedback", 0.0f, 0.95f, 0.0f); // NEW
            
            // Row 2: [Blank], [Blank], [Blank], Mix
            addSpacer();
            addSpacer();
            addSpacer();
            addParameter ("Mod Mix", 0.0f, 1.0f, 0.0f);

            // PAGE 2: Delay
            // Row 1: Dly Time, Dly Note, Dly Sync, Dly FB
            addParameter ("Dly Time", 1.0f, 2000.0f, 400.0f);
            std::vector<juce::String> divisions = { 
                "1/64", "1/32", "1/16", "1/8", "1/4", "1/2", "1/1", "2/1", "4/1" 
            };
            addChoiceParameter ("Dly Note", divisions, 4); // NEW: Default 1/4
            addParameter ("Dly Sync", 0.0f, 1.0f, 0.0f, true);
            if (auto* p = parameters.back()) p->setBinaryLabels ("MS", "NOTE");
            addParameter ("Dly FB", 0.0f, 0.95f, 0.3f);
            
            // Row 2: [Blank], [Blank], [Blank], Mix
            addSpacer();
            addSpacer();
            addSpacer();
            addParameter ("Dly Mix", 0.0f, 1.0f, 0.0f);

            // PAGE 3: Reverb
            // Row 1: Rvb Time, Rvb Size, Rvb Damp, Rvb Predelay
            addParameter ("Rvb Time", 0.0f, 10.0f, 2.0f); // NEW: Reverb decay time in seconds
            addParameter ("Rvb Size", 0.0f, 1.0f, 0.5f);
            addParameter ("Rvb Damp", 0.0f, 1.0f, 0.5f);
            addParameter ("Rvb Predelay", 0.0f, 200.0f, 0.0f); // NEW: Predelay in ms
            
            // Row 2: [Blank], [Blank], [Blank], Mix
            addSpacer();
            addSpacer();
            addSpacer();
            addParameter ("Rvb Mix", 0.0f, 1.0f, 0.0f);
            
            moduleNameDisplay.setText("MASTER EFFECTS", juce::dontSendNotification);
            lastAdjustedIndex = 0;
        }

        void paintVisualization (juce::Graphics& g, juce::Rectangle<int> area) override
        {
            auto r = area.reduced (100, 80).toFloat();
            g.setColour (accentColor.withAlpha (0.05f));
            g.fillRoundedRectangle (r, 8.0f);
            g.setColour (accentColor.withAlpha (0.15f));
            g.drawRoundedRectangle (r, 8.0f, 1.0f);

            // Stylized signal chain nodes
            float w = r.getWidth();
            float h = r.getHeight();
            float nodeSize = 40.0f;
            float cy = r.getCentreY();
            
            auto drawNode = [&](float x, juce::String label, bool active) {
                juce::Rectangle<float> node (x - nodeSize*0.5f, cy - nodeSize*0.5f, nodeSize, nodeSize);
                g.setColour (accentColor.withAlpha (active ? 0.3f : 0.05f));
                g.fillEllipse (node);
                g.setColour (accentColor.withAlpha (active ? 0.8f : 0.2f));
                g.drawEllipse (node, 2.0f);
                g.setFont (12.0f);
                g.drawText (label, node.translated(0, nodeSize).withHeight(20), juce::Justification::centred);
            };

            float spacing = w / 4.0f;
            // Actual parameter indices (excluding spacers):
            // 0-3: Page 1 (Mod Type, Rate, Depth, Feedback)
            // 4: Mod Mix
            // 5-8: Page 2 (Dly Time, Note, Sync, FB)
            // 9: Dly Mix
            // 10-13: Page 3 (Rvb Time, Size, Damp, Predelay)
            // 14: Rvb Mix
            drawNode (r.getX() + spacing * 1, "MOD", parameters[4]->getValue() > 0.01f);  // Mod Mix
            drawNode (r.getX() + spacing * 2, "DLY", parameters[9]->getValue() > 0.01f);  // Dly Mix
            drawNode (r.getX() + spacing * 3, "RVB", parameters[14]->getValue() > 0.01f); // Rvb Mix

            // Connectors
            g.setColour (accentColor.withAlpha (0.1f));
            g.drawLine (r.getX() + spacing * 1 + nodeSize*0.5f, cy, r.getX() + spacing * 2 - nodeSize*0.5f, cy, 1.0f);
            g.drawLine (r.getX() + spacing * 2 + nodeSize*0.5f, cy, r.getX() + spacing * 3 - nodeSize*0.5f, cy, 1.0f);
        }
    };
}
