#pragma once

#include <neon_ui_components/neon_ui_components.h>

namespace neon
{
    /**
     * ChipOscModule
     * UI module for the chip oscillator page.
     * Chip type selector, waveform, pulse width, bit depth, volume.
     */
    class ChipOscModule : public ModuleBase
    {
    public:
        ChipOscModule (const juce::String& name, const juce::Colour& color)
            : ModuleBase (name, color)
        {
            // Row 1
            addChoiceParameter ("Waveform", { "Square", "Div 6 (Buzz)", "Div 31 (Low Buzz)", "Poly 4 (Noisy)", "Poly 5 (Metallic)", "Poly 9 (White)" }, 0);
            addParameter ("Volume", 0.0f, 1.0f, 0.8f);
            addSpacer();
            addSpacer();

            // Row 2
            addParameter ("Bit Depth", 1.0f, 16.0f, 4.0f, false, 1.0f, false, true);
            addSpacer();
            addSpacer();
            addSpacer();

            lastAdjustedIndex = 0;
        }

    protected:
        void paintVisualization (juce::Graphics& g, juce::Rectangle<int> area) override
        {
            auto r = area.reduced (80, 60).toFloat();

            g.setColour (accentColor.withAlpha (0.08f));
            g.fillRoundedRectangle (r, 8.0f);

            // Draw chip type badge
            g.setColour (accentColor);
            auto badge = juce::Rectangle<float> (r.getX() + 10, r.getY() + 10, 100, 30);
            g.fillRoundedRectangle (badge, 6.0f);
            g.setColour (juce::Colours::black);
            g.setFont (juce::FontOptions (14.0f).withStyle ("Bold"));
            g.drawText ("ATARI 2600", badge, juce::Justification::centred);

            // Draw waveform preview
            int waveform = (int) parameters[0]->getValue();

            juce::Path wavePath;
            float waveW = r.getWidth() * 0.7f;
            float waveH = r.getHeight() * 0.4f;
            float waveX = r.getCentreX() - waveW / 2;
            float waveY = r.getCentreY();

            int numPoints = 200;
            for (int i = 0; i <= numPoints; ++i)
            {
                float t = (float) i / numPoints;
                float sample = 0.0f;

                switch (waveform)
                {
                    case 0: // Square
                        sample = (t < 0.5f) ? 1.0f : -1.0f;
                        break;
                    case 1: // Div 6
                        sample = (std::fmod (t * 6.0f, 1.0f) < 0.5f) ? 1.0f : -1.0f;
                        break;
                    case 2: // Div 31
                        sample = (std::fmod (t * 31.0f, 1.0f) < 0.5f) ? 1.0f : -1.0f;
                        break;
                    default: // Poly
                        sample = ((float)(std::rand() % 200) / 100.0f) - 1.0f;
                        break;
                }

                float x = waveX + t * waveW;
                float y = waveY - sample * waveH * 0.45f;

                if (i == 0) wavePath.startNewSubPath (x, y);
                else wavePath.lineTo (x, y);
            }

            g.setColour (accentColor);
            g.strokePath (wavePath, juce::PathStrokeType (2.5f, juce::PathStrokeType::curved));

            // Bit depth indicator
            int bits = (int) parameters[1]->getValue();
            g.setColour (accentColor.withAlpha (0.5f));
            g.setFont (12.0f);
            g.drawText (juce::String (bits) + "-bit", r.getRight() - 70, r.getY() + 12, 60, 20,
                        juce::Justification::centredRight);
        }
    };

    /**
     * ChipFilterModule
     * UI module for the chip filter.
     */
    class ChipFilterModule : public ModuleBase
    {
    public:
        ChipFilterModule (const juce::String& name, const juce::Colour& color)
            : ModuleBase (name, color)
        {
            // Row 1
            addChoiceParameter ("Filter Type", { "LP", "HP", "BP" }, 0);
            addParameter ("Cutoff", 20.0f, 20000.0f, 20000.0f);
            addParameter ("Resonance", 0.0f, 1.0f, 0.0f);
            addSpacer();

            // Row 2
            addSpacer();
            addSpacer();
            addSpacer();
            addSpacer();

            lastAdjustedIndex = 1;
        }

    protected:
        void paintVisualization (juce::Graphics& g, juce::Rectangle<int> area) override
        {
            auto r = area.reduced (80, 60).toFloat();
            g.setColour (accentColor.withAlpha (0.08f));
            g.fillRoundedRectangle (r, 8.0f);

            // Draw a simple filter response curve
            float cutoff = parameters[1]->getValue();
            float res = parameters[2]->getValue();
            float normCut = std::log2 (cutoff / 20.0f) / std::log2 (20000.0f / 20.0f);

            juce::Path filterPath;
            float fW = r.getWidth() * 0.8f;
            float fH = r.getHeight() * 0.5f;
            float fX = r.getCentreX() - fW / 2;
            float fY = r.getCentreY() + fH * 0.3f;

            for (int i = 0; i <= 200; ++i)
            {
                float t = (float) i / 200.0f;
                float response = 1.0f;

                float diff = t - normCut;
                if (diff > 0.0f)
                    response = std::exp (-diff * 8.0f);

                // Resonance bump near cutoff
                float bump = std::exp (-std::pow (diff * 10.0f, 2.0f));
                response += bump * res * 0.5f;

                response = std::clamp (response, 0.0f, 1.5f);

                float x = fX + t * fW;
                float y = fY - response * fH * 0.6f;

                if (i == 0) filterPath.startNewSubPath (x, y);
                else filterPath.lineTo (x, y);
            }

            g.setColour (accentColor);
            g.strokePath (filterPath, juce::PathStrokeType (2.5f, juce::PathStrokeType::curved));
        }
    };

} // namespace neon
