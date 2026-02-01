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
            // Page 1: Mod & Delay
            std::vector<juce::String> modTypes = { "OFF", "CHORUS", "PHASER", "FLANGER" };
            addChoiceParameter ("Mod Type", modTypes, 1);
            addParameter ("Mod Rate", 0.1f, 10.0f, 1.0f);
            addParameter ("Mod Depth", 0.0f, 1.0f, 0.5f);
            addParameter ("Mod Mix", 0.0f, 1.0f, 0.0f);

            addParameter ("Dly Time", 1.0f, 2000.0f, 400.0f);
            addParameter ("Dly FB", 0.0f, 0.95f, 0.3f);
            addParameter ("Dly Mix", 0.0f, 1.0f, 0.0f);
            addParameter ("Dly Sync", 0.0f, 1.0f, 0.0f, true);
            if (auto* p = parameters.back()) p->setBinaryLabels ("MS", "NOTE");

            // Page 2: Reverb
            addParameter ("Rvb Size", 0.0f, 1.0f, 0.5f);
            addParameter ("Rvb Damp", 0.0f, 1.0f, 0.5f);
            addParameter ("Rvb Width", 0.0f, 1.0f, 0.5f);
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
            drawNode (r.getX() + spacing * 1, "MOD", parameters[3]->getValue() > 0.01f);
            drawNode (r.getX() + spacing * 2, "DLY", parameters[6]->getValue() > 0.01f);
            drawNode (r.getX() + spacing * 3, "RVB", parameters[11]->getValue() > 0.01f);

            // Connectors
            g.setColour (accentColor.withAlpha (0.1f));
            g.drawLine (r.getX() + spacing * 1 + nodeSize*0.5f, cy, r.getX() + spacing * 2 - nodeSize*0.5f, cy, 1.0f);
            g.drawLine (r.getX() + spacing * 2 + nodeSize*0.5f, cy, r.getX() + spacing * 3 - nodeSize*0.5f, cy, 1.0f);
        }
    };
}
