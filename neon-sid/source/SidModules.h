#pragma once

#include <neon_ui_components/neon_ui_components.h>

namespace neon
{
    /**
     * SidOscModule
     * Enhanced SID oscillator module matching the template functionality but with SID flair.
     */
    class SidOscModule : public ModuleBase
    {
    public:
        SidOscModule (const juce::String& name, const juce::Colour& color)
            : ModuleBase (name, color)
        {
            // Row 1: Primary Controls
            addChoiceParameter ("Waveform", { "Triangle", "Sawtooth", "Pulse", "Noise" }, 2);
            addParameter ("Volume", 0.0f, 1.0f, 0.8f);
            addParameter ("Transp", -24.0f, 24.0f, 0.0f, false, 1.0f);
            addParameter ("Fine", -100.0f, 100.0f, 0.0f);

            // Row 2: SID Special Features
            addParameter ("Pulse Width", 0.01f, 0.99f, 0.5f);
            addParameter ("Sync", 0.0f, 1.0f, 0.0f, true);
            if (auto* p = parameters.back()) p->setBinaryLabels("OFF", "ON");
            
            addParameter ("Ring Mod", 0.0f, 1.0f, 0.0f, true);
            if (auto* p = parameters.back()) p->setBinaryLabels("OFF", "ON");
            
            addSpacer();
        }

    protected:
        void paintVisualization (juce::Graphics& g, juce::Rectangle<int> area) override
        {
            auto r = area.reduced (120, 80).toFloat();
            
            // Draw Oscilloscope background
            g.setColour (juce::Colours::black.withAlpha (0.4f));
            g.fillRoundedRectangle (r, 5.0f);
            g.setColour (accentColor.withAlpha (0.3f));
            g.drawRoundedRectangle (r, 5.0f, 1.0f);

            // Draw grid lines
            g.setColour (accentColor.withAlpha (0.1f));
            g.drawHorizontalLine ((int)r.getCentreY(), r.getX(), r.getRight());
            g.drawVerticalLine ((int)r.getCentreX(), r.getY(), r.getBottom());

            // Draw MOS Badge in corner
            auto badgeRect = r.withSize (60, 30).withPosition (r.getRight() - 70, r.getBottom() - 40);
            g.setColour (juce::Colours::darkgrey.withAlpha(0.8f));
            g.fillRoundedRectangle (badgeRect, 2.0f);
            g.setColour (juce::Colours::white.withAlpha (0.5f));
            g.setFont (12.0f);
            g.drawText ("6581", badgeRect, juce::Justification::centred);

            // Draw waveform based on selection
            int waveform = (int) parameters[0]->getValue();
            float pw = parameters[4]->getValue();
            
            juce::Path p;
            float step = r.getWidth() / 100.0f;
            p.startNewSubPath (r.getX(), r.getCentreY());

            for (int i = 0; i < 100; ++i)
            {
                float x = r.getX() + (float)i * step;
                float phase = std::fmod ((float)i / 25.0f, 1.0f);
                float val = 0.0f;

                switch (waveform)
                {
                    case 0: val = (phase < 0.5f) ? (phase * 4.0f - 1.0f) : (3.0f - phase * 4.0f); break;
                    case 1: val = phase * 2.0f - 1.0f; break;
                    case 2: val = (phase < pw) ? 1.0f : -1.0f; break;
                    case 3: val = ((float)std::rand() / (float)RAND_MAX) * 2.0f - 1.0f; break;
                }

                p.lineTo (x, r.getCentreY() - (val * r.getHeight() * 0.4f));
            }

            g.setColour (accentColor);
            g.strokePath (p, juce::PathStrokeType (2.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        }
    };

} // namespace neon
