#pragma once

#include "ModuleBase.h"

namespace neon
{
    /**
     * @class DahdsrModule
     * @brief A 6-parameter envelope module (Delay, Attack, Hold, Decay, Sustain, Release)
     */
    class DahdsrModule : public ModuleBase
    {
    public:
        DahdsrModule (const juce::String& name, const juce::Colour& color, bool includeAmount = false) 
            : ModuleBase (name, color)
        {
            // Initialize with DAHDSR parameters
            addParameter ("Delay",   0.0f, 10000.0f, 0.0f);
            addParameter ("Attack",  0.0f, 10000.0f, 100.0f);
            addParameter ("Hold",    0.0f, 10000.0f, 0.0f);
            addParameter ("Decay",   0.0f, 10000.0f, 500.0f);
            addParameter ("Sustain", 0.0f, 1.0f,     0.7f);
            addParameter ("Release", 0.0f, 10000.0f, 1000.0f);
            
            if (includeAmount)
            {
                addParameter ("Amount", -100.0f, 100.0f, 0.0f);
            }

            addChoiceParameter ("Env Mode", {"Normal", "Legato", "Retrigger"}, 0);

            // Set initial display
            lastAdjustedIndex = 1; // Default to Attack
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

            // Normalized values (0-1) for visualization
            // Time parameters (0-10000ms)
            auto getNormTime = [&](int idx) { 
                return juce::jmap (parameters[idx]->getValue(), 0.0f, 10000.0f, 0.01f, 0.25f); 
            };
            
            float d   = getNormTime(0);
            float a   = getNormTime(1);
            float h   = getNormTime(2);
            float dec = getNormTime(3);
            float s   = parameters[4]->getValue(); // Sustain level (0-1)
            float rel = getNormTime(5);

            float total = d + a + h + dec + 0.2f + rel; // 0.2 for sustain visual gap
            float scaleX = r.getWidth() / total;
            
            float x = r.getX();
            float y0 = r.getBottom();
            float y1 = r.getY();

            visualPts.clear();
            
            visualPts.push_back({x, y0, 0});
            x += d * scaleX;
            visualPts.push_back({x, y0, 0}); // Delay end
            x += a * scaleX;
            visualPts.push_back({x, y1, 1}); // Attack end
            x += h * scaleX;
            visualPts.push_back({x, y1, 2}); // Hold end
            x += dec * scaleX;
            visualPts.push_back({x, y0 - s * (y0 - y1), 3}); // Decay end (sustain level)
            x += 0.2f * scaleX;
            visualPts.push_back({x, y0 - s * (y0 - y1), 4}); // Sustain width (param 4 is level but we use it visually)
            x += rel * scaleX;
            visualPts.push_back({x, y0, 5}); // Release end

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

            // Envelope "Amount" visualization if present
            if (parameters.size() > 6 && (parameters[6]->getName() == "Amount" || parameters[6]->getName() == "Atk Env Amount"))
            {
                float amount = parameters[6]->getValue(); // -100 to 100
                float center = r.getCentreY();
                float barH = (r.getHeight() * 0.45f) * (amount / 100.0f);
                
                auto drawColor = (lastAdjustedIndex == 6 ? juce::Colours::yellow : accentColor);
                
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

            if (!isDrag)
            {
                draggingIdx = -1;
                draggingAmount = false;

                // Check for Amount bar hit
                if (parameters.size() > 6 && (parameters[6]->getName() == "Amount" || parameters[6]->getName() == "Atk Env Amount"))
                {
                    juce::Rectangle<float> amountBar (r.getRight() + 30.0f, r.getY(), 40.0f, r.getHeight());
                    if (amountBar.contains (e.position))
                    {
                        draggingAmount = true;
                        lastAdjustedIndex = 6;
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
                    setClamped (6, startVals[6] + dy * amountSens);
                }
                else if (draggingIdx != -1)
                {
                    float timeSens = 20.0f; // 1px = 20ms
                    float levelSens = 0.005f; // 1px = 0.5%

                    if (draggingIdx == 1) setClamped (0, startVals[0] + dx * timeSens); // Delay
                    if (draggingIdx == 2) setClamped (1, startVals[1] + dx * timeSens); // Attack
                    if (draggingIdx == 3) setClamped (2, startVals[2] + dx * timeSens); // Hold
                    if (draggingIdx == 4) { // Decay and Sustain Level
                        setClamped (3, startVals[3] + dx * timeSens);
                        setClamped (4, startVals[4] - dy * levelSens);
                    }
                    if (draggingIdx == 5) { // Sustain point
                        setClamped (4, startVals[4] - dy * levelSens);
                    }
                    if (draggingIdx == 6) setClamped (5, startVals[5] + dx * timeSens); // Release
                }

                repaint();
            }
        }
    };
}
