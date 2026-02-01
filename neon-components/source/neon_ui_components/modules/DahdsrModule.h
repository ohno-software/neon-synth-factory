#pragma once

#include "ModuleBase.h"

namespace neon
{
    /**
     * @class DahdsrModule
     * @brief A 6-parameter envelope module (Delay, Attack, Hold, Decay, Sustain, Release)
     * 
     * @param includeAmount - Legacy parameter for simple Amount control
     * @param includeModulation - If true, adds Target, V.Amount, AT.Amount, V.Attack, V.Amount2 parameters
     */
    class DahdsrModule : public ModuleBase
    {
    public:
        DahdsrModule (const juce::String& name, const juce::Colour& color, bool includeAmount = false, bool includeModulation = false) 
            : ModuleBase (name, color)
        {
            if (includeModulation)
            {
                // PAGE 1: Row 1: Target, Delay, Attack, Hold
                std::vector<juce::String> targets = { "Osc 1", "Osc 2", "Osc 1&2" };
                addChoiceParameter ("Target", targets, 2); // Default to both oscillators
            }
            
            // PAGE 1: DAHDSR parameters (continuing row 1 if includeModulation, else starting fresh)
            addParameter ("Delay",   0.0f, 10000.0f, 0.0f);
            addParameter ("Attack",  0.0f, 10000.0f, 100.0f);
            addParameter ("Hold",    0.0f, 10000.0f, 0.0f);
            
            // PAGE 1: Row 2: Decay, Sustain, Release, Mode
            addParameter ("Decay",   0.0f, 10000.0f, 500.0f);
            addParameter ("Sustain", 0.0f, 1.0f,     0.7f);
            addParameter ("Release", 0.0f, 10000.0f, 1000.0f);
            addChoiceParameter ("Env Mode", {"Normal", "Legato", "Retrigger"}, 0);
            
            if (includeAmount || includeModulation)
            {
                // PAGE 2: Row 1: Amount, V.Amount, AT.Amount, [Blank]
                addParameter ("Amount", -100.0f, 100.0f, 0.0f);
                
                if (includeModulation)
                {
                    addParameter ("V.Amount", 0.0f, 1.0f, 0.0f);  // Velocity sensitivity for amount
                    addParameter ("AT.Amount", 0.0f, 1.0f, 0.0f); // Aftertouch sensitivity for amount
                    addSpacer(); // Blank
                    
                    // PAGE 2: Row 2: [Blank], V.Attack, [Blank], [Blank]
                    addSpacer(); // Blank
                    addParameter ("V.Attack", 0.0f, 1.0f, 0.0f);  // Velocity sensitivity for attack time
                    addSpacer(); // Blank
                    addSpacer(); // Blank
                }
            }

            // Set initial display
            lastAdjustedIndex = (includeModulation ? 2 : 1); // Default to Attack
        }
        
        ~DahdsrModule() override = default;

    protected:
        struct VisualPoint { float x, y; int paramId; };
        std::vector<VisualPoint> visualPts;
        int draggingIdx = -1;
        bool draggingAmount = false;
        std::vector<float> startVals; 

        void paintVisualization (juce::Graphics& g, juce::Rectangle<int> area) override
        {
            auto r = area.reduced (100, 100).toFloat();
            
            // Draw a background grid or frame for the visualization
            g.setColour (accentColor.withAlpha (0.05f));
            g.fillRect (r);
            g.setColour (accentColor.withAlpha (0.15f));
            g.drawRect (r, 1.0f);

            // Determine parameter offset (if Target parameter exists, it's at index 0)
            int offset = 0;
            if (parameters.size() > 0 && parameters[0]->getName() == "Target")
                offset = 1;

            // Normalized values (0-1) for visualization
            // Time parameters (0-10000ms)
            auto getNormTime = [&](int idx) { 
                return juce::jmap (parameters[offset + idx]->getValue(), 0.0f, 10000.0f, 0.01f, 0.25f); 
            };
            
            float d   = getNormTime(0);
            float a   = getNormTime(1);
            float h   = getNormTime(2);
            float dec = getNormTime(3);
            float s   = parameters[offset + 4]->getValue(); // Sustain level (0-1)
            float rel = getNormTime(5);

            float total = d + a + h + dec + 0.2f + rel; // 0.2 for sustain visual gap
            float scaleX = r.getWidth() / total;
            
            float x = r.getX();
            float y0 = r.getBottom();
            float y1 = r.getY();

            visualPts.clear();
            
            visualPts.push_back({x, y0, offset + 0});
            x += d * scaleX;
            visualPts.push_back({x, y0, offset + 0}); // Delay end
            x += a * scaleX;
            visualPts.push_back({x, y1, offset + 1}); // Attack end
            x += h * scaleX;
            visualPts.push_back({x, y1, offset + 2}); // Hold end
            x += dec * scaleX;
            visualPts.push_back({x, y0 - s * (y0 - y1), offset + 3}); // Decay end (sustain level)
            x += 0.2f * scaleX;
            visualPts.push_back({x, y0 - s * (y0 - y1), offset + 4}); // Sustain width (param 4 is level but we use it visually)
            x += rel * scaleX;
            visualPts.push_back({x, y0, offset + 5}); // Release end

            g.setOpacity (0.8f);
            for (size_t i = 0; i < visualPts.size() - 1; ++i)
            {
                int paramId = visualPts[i+1].paramId;
                g.setColour (lastAdjustedIndex == paramId ? juce::Colours::yellow : accentColor);
                g.drawLine (visualPts[i].x, visualPts[i].y, visualPts[i+1].x, visualPts[i+1].y, 6.0f);
                
                // Draw connecting dots
                g.fillEllipse (visualPts[i].x - 4, visualPts[i].y - 4, 8, 8);
            }
            g.fillEllipse (visualPts.back().x - 4, visualPts.back().y - 4, 8, 8);

            // Envelope "Amount" visualization if present (check for Amount parameter after Env Mode)
            int amountIdx = offset + 7; // After DAHDSR (6) + Env Mode (1)
            if (parameters.size() > amountIdx && parameters[amountIdx]->getName() == "Amount")
            {
                float amount = parameters[amountIdx]->getValue(); // -100 to 100
                float center = r.getCentreY();
                float barH = (r.getHeight() * 0.45f) * (amount / 100.0f);
                
                auto drawColor = (lastAdjustedIndex == amountIdx ? juce::Colours::yellow : accentColor);
                
                g.setColour (drawColor.withAlpha(0.1f));
                g.fillRect (r.getRight() + 40.0f, r.getY() + r.getHeight() * 0.05f, 20.0f, r.getHeight() * 0.9f);
                
                g.setColour (drawColor.withAlpha(0.4f));
                g.fillRect (r.getRight() + 40.0f, center, 20.0f, -barH);
                
                g.setColour (drawColor);
                g.drawRect (r.getRight() + 40.0f, center, 20.0f, -barH, 1.5f);
                g.drawHorizontalLine ((int)center, r.getRight() + 40.0f, r.getRight() + 60.0f);
            }
        }

        void handleVisualizationInteraction (const juce::MouseEvent& e, bool isDrag) override
        {
            auto r = getLocalBounds().reduced (100, 100).toFloat();

            // Determine parameter offset
            int offset = 0;
            if (parameters.size() > 0 && parameters[0]->getName() == "Target")
                offset = 1;

            int amountIdx = offset + 7; // After DAHDSR + Env Mode

            if (!isDrag)
            {
                draggingIdx = -1;
                draggingAmount = false;

                // Check for Amount bar hit
                if (parameters.size() > amountIdx && parameters[amountIdx]->getName() == "Amount")
                {
                    juce::Rectangle<float> amountBar (r.getRight() + 30.0f, r.getY(), 40.0f, r.getHeight());
                    if (amountBar.contains (e.position))
                    {
                        draggingAmount = true;
                        lastAdjustedIndex = amountIdx;
                    }
                }

                if (!draggingAmount)
                {
                    // Simple hit detection for points
                    for (int i = 0; i < (int)visualPts.size(); ++i)
                    {
                        if (e.position.getDistanceSquaredFrom (juce::Point<float>(visualPts[i].x, visualPts[i].y)) < 900.0f) // 30px radius
                        {
                            draggingIdx = i;
                            lastAdjustedIndex = visualPts[i].paramId;
                            break;
                        }
                    }
                }

                // Cache start values
                startVals.clear();
                for (auto* p : parameters) startVals.push_back (p->getValue());
            }
            else
            {
                float dx = (float)e.getDistanceFromDragStartX();
                float dy = (float)e.getDistanceFromDragStartY();

                auto setClamped = [&](int idx, float val) {
                    if (idx >= 0 && idx < (int)parameters.size()) {
                        parameters[idx]->setValue (val);
                        if (idx < (int)cards.size())
                            cards[idx]->setValue (parameters[idx]->getValue());
                    }
                };

                if (draggingAmount)
                {
                    float amountSens = -0.5f; // Vertical drag
                    setClamped (amountIdx, startVals[amountIdx] + dy * amountSens);
                }
                else if (draggingIdx != -1)
                {
                    float timeSens = 20.0f; // 1px = 20ms
                    float levelSens = 0.005f; // 1px = 0.5%

                    if (draggingIdx == 1) setClamped (offset + 0, startVals[offset + 0] + dx * timeSens); // Delay
                    if (draggingIdx == 2) setClamped (offset + 1, startVals[offset + 1] + dx * timeSens); // Attack
                    if (draggingIdx == 3) setClamped (offset + 2, startVals[offset + 2] + dx * timeSens); // Hold
                    if (draggingIdx == 4) { // Decay and Sustain Level
                        setClamped (offset + 3, startVals[offset + 3] + dx * timeSens);
                        setClamped (offset + 4, startVals[offset + 4] - dy * levelSens);
                    }
                    if (draggingIdx == 5) { // Sustain point
                        setClamped (offset + 4, startVals[offset + 4] - dy * levelSens);
                    }
                    if (draggingIdx == 6) setClamped (offset + 5, startVals[offset + 5] + dx * timeSens); // Release
                }

                repaint();
            }
        }
    };
}
