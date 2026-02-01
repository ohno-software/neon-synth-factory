#pragma once

#include "ModuleBase.h"

namespace neon
{
    class LadderFilterModule : public ModuleBase
    {
    public:
        LadderFilterModule (const juce::String& name, const juce::Colour& color) 
            : ModuleBase (name, color)
        {
            // Row 1: Type, Cutoff, Res, Slope
            addChoiceParameter ("Type", NeonRegistry::getFilterTypes(), 0);
            addParameter ("Cutoff",   20.0f, 20000.0f, 20000.0f);
            addParameter ("Res",      0.0f, 1.0f, 0.0f);
            addParameter ("Slope",    0.0f, 1.0f, 1.0f, true);
            if (auto* p = parameters.back()) p->setBinaryLabels ("12dB", "24dB");
            
            // Row 2: KeyTrack, Drive, Velocity, Aftertouch
            addParameter ("KeyTrack", 0.0f, 1.0f, 0.5f, false, 0.0f, false, true);
            addParameter ("Drive",    1.0f, 10.0f, 1.0f);
            addParameter ("Velocity", 0.0f, 1.0f, 0.0f); // NEW: Filter velocity sensitivity
            addParameter ("Aftertouch", 0.0f, 1.0f, 0.0f); // NEW: Filter aftertouch sensitivity
            
            lastAdjustedIndex = 0;
        }

    protected:
        float startCutoff = 1000.0f;
        float startRes = 0.0f;

        void paintVisualization (juce::Graphics& g, juce::Rectangle<int> area) override
        {
            auto r = area.reduced (100, 80).toFloat();
            
            // Draw background
            g.setColour (accentColor.withAlpha (0.05f));
            g.fillRoundedRectangle (r, 5.0f);
            
            int filterType = (int)parameters[0]->getValue(); // 0=LP, 1=HP, 2=BP
            float cutoff = parameters[1]->getValue();
            float res = parameters[2]->getValue();
            bool is24dB = parameters[3]->getValue() > 0.5f; // Slope
            float drive = parameters[5]->getValue(); // Drive (1.0 to 10.0)

            // Visualization constants
            float height = r.getHeight();
            float width = r.getWidth();
            float startX = r.getX();
            float zeroGainY = r.getBottom() - 5.0f;
            float unityGainY = r.getY() + height * (filterType == 2 ? 0.35f : 0.65f); // BP needs more headroom
            float gainRange = zeroGainY - unityGainY;
            
            float passbandGain = 1.0f - (res * 0.45f); 
            float peakBoost = res * 1.8f;
            float driveFactor = 1.0f + ((drive - 1.0f) * 0.08f);

            juce::Path path;
            auto freqToX = [&](float f) {
                return startX + (std::log10(f / 20.0f) / 3.0f) * width;
            };

            const int numPoints = 120;
            for (int i = 0; i <= numPoints; ++i)
            {
                float t = i / (float)numPoints;
                float freq = 20.0f * std::pow (1000.0f, t); 
                float ratio = freq / cutoff;
                
                float order = is24dB ? 4.0f : 2.0f;
                float mag = 1.0f;

                if (filterType == 0) // LP
                {
                    mag = 1.0f / std::sqrt (1.0f + std::pow (ratio, order * 2.0f));
                    float bell = std::exp (-std::pow (ratio - 1.0f, 2.0f) / (0.15f - (res * 0.1f)));
                    mag = ((mag * passbandGain) + (bell * peakBoost * mag)) * driveFactor;
                }
                else if (filterType == 1) // HP
                {
                    mag = 1.0f / std::sqrt (1.0f + std::pow (1.0f / ratio, order * 2.0f));
                    float bell = std::exp (-std::pow (ratio - 1.0f, 2.0f) / (0.15f - (res * 0.1f)));
                    mag = ((mag * passbandGain) + (bell * peakBoost * mag)) * driveFactor;
                }
                else // BP
                {
                    // Approximation for BP visual
                    float resonanceFactor = 1.0f / (0.5f - (res * 0.45f));
                    mag = 1.0f / std::sqrt (1.0f + std::pow (resonanceFactor * (ratio - 1.0f/ratio), 2.0f));
                    mag = mag * driveFactor;
                }
                
                float x = freqToX (freq);
                float y = zeroGainY - (mag * gainRange);

                y = juce::jlimit (r.getY(), r.getBottom(), y);

                if (i == 0) path.startNewSubPath (x, y);
                else path.lineTo (x, y);
            }

            // Styling
            auto drawColor = (lastAdjustedIndex >= 0 && lastAdjustedIndex <= 4) ? juce::Colours::yellow : accentColor;
            
            // Fill area under the curve
            juce::Path fillPath = path;
            fillPath.lineTo (r.getRight(), r.getBottom());
            fillPath.lineTo (r.getX(), r.getBottom());
            fillPath.closeSubPath();
            
            juce::ColourGradient grad (drawColor.withAlpha (0.15f), 0, unityGainY - gainRange,
                                      drawColor.withAlpha (0.02f), 0, r.getBottom(), false);
            g.setGradientFill (grad);
            g.fillPath (fillPath);

            // Draw the main curve
            g.setColour (drawColor);
            g.strokePath (path, juce::PathStrokeType (3.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

            // Cutoff frequency indicator (subtle glowing line)
            float cutoffX = freqToX (cutoff);
            g.setColour (drawColor.withAlpha (0.2f));
            g.drawVerticalLine ((int)cutoffX, r.getY(), r.getBottom());
            g.fillEllipse (cutoffX - 3, r.getBottom() - 10, 6, 6);
        }

        void handleVisualizationInteraction (const juce::MouseEvent& e, bool isDrag) override
        {
            if (!isDrag)
            {
                startCutoff = parameters[0]->getValue();
                startRes = parameters[1]->getValue();
                lastAdjustedIndex = 0;
            }
            else
            {
                float dx = (float)e.getDistanceFromDragStartX();
                float dy = (float)e.getDistanceFromDragStartY();

                // Cutoff (Logarithmic)
                float logScale = 0.005f;
                float newCutoff = startCutoff * std::pow (10.0f, dx * logScale);
                parameters[0]->setValue (newCutoff);
                cards[0]->setValue (parameters[0]->getValue());

                // Resonance
                float resSens = -0.005f;
                parameters[1]->setValue (startRes + dy * resSens);
                cards[1]->setValue (parameters[1]->getValue());

                repaint();
            }
        }
    };
}
