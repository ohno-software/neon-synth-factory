#pragma once

#include <juce_graphics/juce_graphics.h>
#include <neon_ui_components/neon_ui_components.h>
#include "FmAlgorithm.h"

// If you link NeonFmBinaryData, juce adds this header globally.
#include <BinaryData.h>

namespace neon
{
    /**
     * FmOperatorModule
     * UI module for a single FM operator.
     * Page 1: Waveform, Ratio, Detune, Level, Feedback, Velocity, KeySync
     * Page 2: Attack, Decay, Sustain, Release (per-operator envelope)
     */
    class FmOperatorModule : public ModuleBase
    {
    public:
        FmOperatorModule (const juce::String& name, const juce::Colour& color, int opIndex)
            : ModuleBase (name, color), operatorIndex (opIndex)
        {
            // Page 1, Row 1: Waveform, Ratio, Detune, Level
            addChoiceParameter ("Waveform", { "Sine", "Triangle", "Saw", "Square" }, 0);

            // Ratio: DX7-style snapping to 0.5 increments (0.5, 1.0, 1.5, ..., 16.0)
            addParameter ("Ratio", 0.5f, 16.0f, getDefaultRatio (opIndex), false, 0.5f, false, true);

            addParameter ("Detune", -50.0f, 50.0f, 0.0f, false, 0.0f, false, true);
            addParameter ("Level", 0.0f, 1.0f, getDefaultLevel (opIndex));

            // Page 1, Row 2: Feedback, Velocity, KeySync, [spacer]
            addParameter ("Feedback", 0.0f, 1.0f, 0.0f);
            addParameter ("Velocity", 0.0f, 1.0f, 0.5f);
            addParameter ("KeySync", 0.0f, 1.0f, 1.0f, true);
            if (auto* p = parameters.back()) p->setBinaryLabels ("FREE", "SYNC");
            addSpacer();

            // Page 2, Row 1: Attack, Decay, Sustain, Release (operator envelope)
            addParameter ("Attack",  0.0f, 10000.0f, 10.0f);
            addParameter ("Decay",   0.0f, 10000.0f, 500.0f);
            addParameter ("Sustain", 0.0f, 1.0f, 0.7f);
            addParameter ("Release", 0.0f, 10000.0f, 500.0f);

            lastAdjustedIndex = 0;
        }

    protected:
        int operatorIndex;

        static float getDefaultRatio (int idx)
        {
            // Classic DX defaults
            static const float defaults[] = { 1.0f, 2.0f, 3.0f, 1.0f };
            return defaults[idx % 4];
        }

        static float getDefaultLevel (int idx)
        {
            // Carrier (op4) louder, modulators quieter
            return (idx == 3) ? 1.0f : 0.7f;
        }

        void paintVisualization (juce::Graphics& g, juce::Rectangle<int> area) override
        {
            auto r = area.reduced (80, 60).toFloat();

            // Draw operator indicator
            g.setColour (accentColor.withAlpha (0.08f));
            g.fillRoundedRectangle (r, 8.0f);

            // Draw waveform preview
            int waveform = (int) parameters[0]->getValue();
            float ratio = parameters[1]->getValue();
            float level = parameters[3]->getValue();
            float feedback = parameters[4]->getValue();

            juce::Path wavePath;
            float waveW = r.getWidth() * 0.8f;
            float waveH = r.getHeight() * 0.5f;
            float waveX = r.getCentreX() - waveW / 2;
            float waveY = r.getCentreY();

            int numPoints = 200;
            for (int i = 0; i <= numPoints; ++i)
            {
                float t = (float) i / numPoints;
                float phase = t * ratio * 2.0f;  // Show multiple cycles based on ratio
                phase -= std::floor (phase);

                float sample = 0.0f;
                switch (waveform)
                {
                    case 0: // Sine
                        sample = std::sin (phase * juce::MathConstants<float>::twoPi);
                        break;
                    case 1: // Triangle
                        if (phase < 0.25f) sample = phase * 4.0f;
                        else if (phase < 0.75f) sample = 2.0f - phase * 4.0f;
                        else sample = phase * 4.0f - 4.0f;
                        break;
                    case 2: // Saw
                        sample = 2.0f * phase - 1.0f;
                        break;
                    case 3: // Square
                        sample = phase < 0.5f ? 1.0f : -1.0f;
                        break;
                }

                // Show feedback effect as mild distortion
                if (feedback > 0.01f)
                    sample = std::sin (sample * (1.0f + feedback * 3.0f));

                float x = waveX + t * waveW;
                float y = waveY - sample * waveH * 0.4f * level;

                if (i == 0) wavePath.startNewSubPath (x, y);
                else wavePath.lineTo (x, y);
            }

            // Draw the wave
            auto drawColor = (lastAdjustedIndex >= 0) ? juce::Colours::yellow : accentColor;
            g.setColour (drawColor);
            g.strokePath (wavePath, juce::PathStrokeType (2.5f, juce::PathStrokeType::curved));

            // Draw operator number badge
            g.setColour (accentColor);
            auto badge = juce::Rectangle<float> (r.getX() + 10, r.getY() + 10, 36, 36);
            g.fillRoundedRectangle (badge, 6.0f);
            g.setColour (juce::Colours::black);
            g.setFont (juce::FontOptions (20.0f));
            g.drawText (juce::String (operatorIndex + 1), badge, juce::Justification::centred);

            // Level bar on the right
            auto levelBar = juce::Rectangle<float> (r.getRight() - 20, r.getY() + 30, 10, r.getHeight() - 60);
            g.setColour (accentColor.withAlpha (0.15f));
            g.fillRoundedRectangle (levelBar, 3.0f);
            float barH = levelBar.getHeight() * level;
            auto filledBar = levelBar.withHeight (barH).withY (levelBar.getBottom() - barH);
            g.setColour (accentColor.withAlpha (0.7f));
            g.fillRoundedRectangle (filledBar, 3.0f);

            // Ratio text
            g.setColour (accentColor.withAlpha (0.5f));
            g.setFont (14.0f);
            g.drawText ("x" + juce::String (ratio, 3), r.getX() + 50, r.getY() + 15, 80, 20,
                         juce::Justification::centredLeft);
        }
    };

    /**
     * FmAlgorithmModule
     * UI module that displays and selects the FM algorithm routing.
     * Shows a visual diagram of the operator connections.
     */
    class FmAlgorithmModule : public ModuleBase
    {
    public:
        FmAlgorithmModule (const juce::String& name, const juce::Colour& color)
            : ModuleBase (name, color)
        {
            addChoiceParameter ("Algorithm", getAlgorithmNames(), 0);
            addSpacer();
            addSpacer();
            addSpacer();

            addSpacer();
            addSpacer();
            addSpacer();
            addSpacer();

            lastAdjustedIndex = 0;

            auto loadSvg = [](const char* data, int size) {
                return juce::Drawable::createFromImageData(data, size);
            };

            algoDrawables[0] = loadSvg(BinaryData::algo_1_serial_svg, BinaryData::algo_1_serial_svgSize);
            algoDrawables[1] = loadSvg(BinaryData::algo_2_branch_svg, BinaryData::algo_2_branch_svgSize);
            algoDrawables[2] = loadSvg(BinaryData::algo_3_dualstack_svg, BinaryData::algo_3_dualstack_svgSize);
            algoDrawables[3] = loadSvg(BinaryData::algo_4_triplemod_svg, BinaryData::algo_4_triplemod_svgSize);
            algoDrawables[4] = loadSvg(BinaryData::algo_5_onetomany_svg, BinaryData::algo_5_onetomany_svgSize);
            algoDrawables[5] = loadSvg(BinaryData::algo_6_paralleldual_svg, BinaryData::algo_6_paralleldual_svgSize);
            algoDrawables[6] = loadSvg(BinaryData::algo_7_complexfork_svg, BinaryData::algo_7_complexfork_svgSize);
            algoDrawables[7] = loadSvg(BinaryData::algo_8_fullparallel_svg, BinaryData::algo_8_fullparallel_svgSize);
        }

    protected:
        std::unique_ptr<juce::Drawable> algoDrawables[8];

        void paintVisualization (juce::Graphics& g, juce::Rectangle<int> area) override
        {
            auto r = area.reduced (10, 10).toFloat();
            int algo = (int) parameters[0]->getValue();

            g.setColour (accentColor.withAlpha (0.06f));
            g.fillRoundedRectangle (r, 8.0f);

            // Draw the SVG diagram making full use of the remaining area
            if (algo >= 0 && algo < 8 && algoDrawables[algo] != nullptr)
            {
                // Optionally replace colors to suit our theme
                algoDrawables[algo]->replaceColour(juce::Colour::fromString("#4a90e2"), accentColor.withAlpha(0.6f));
                algoDrawables[algo]->replaceColour(juce::Colour::fromString("#4a9fff"), accentColor);
                algoDrawables[algo]->replaceColour(juce::Colour::fromString("#50c878"), juce::Colours::hotpink); // Carrier
                algoDrawables[algo]->replaceColour(juce::Colour::fromString("#888888"), accentColor.withAlpha(0.5f));
                algoDrawables[algo]->replaceColour(juce::Colour::fromString("#666666"), accentColor.withAlpha(0.5f));
                
                auto svgBounds = r.reduced (10.0f);
                algoDrawables[algo]->drawWithin (g, svgBounds, juce::RectanglePlacement::centred | juce::RectanglePlacement::onlyReduceInSize, 1.0f);
            }
        }
    };

    /**
     * FmFilterEnvModule
     * Filter envelope module for FM synth.
     * ADSR envelope that modulates filter cutoff, with bipolar Amount control.
     */
    class FmFilterEnvModule : public ModuleBase
    {
    public:
        FmFilterEnvModule (const juce::String& name, const juce::Colour& color)
            : ModuleBase (name, color)
        {
            // Row 1: Attack, Decay, Sustain, Release
            addParameter ("Attack",  0.0f, 10000.0f, 10.0f);
            addParameter ("Decay",   0.0f, 10000.0f, 300.0f);
            addParameter ("Sustain", 0.0f, 1.0f,     0.0f);
            addParameter ("Release", 0.0f, 10000.0f, 300.0f);

            // Row 2: Amount (bipolar), spacers
            addParameter ("Amount", -100.0f, 100.0f, 0.0f, false, 0.0f, false, true);
            addSpacer();
            addSpacer();
            addSpacer();

            lastAdjustedIndex = 0;
        }

    protected:
        void paintVisualization (juce::Graphics& g, juce::Rectangle<int> area) override
        {
            auto r = area.reduced (100, 80).toFloat();

            g.setColour (accentColor.withAlpha (0.05f));
            g.fillRoundedRectangle (r, 6.0f);

            // Get envelope values (in ms / normalized)
            float attack  = parameters[0]->getValue();   // ms
            float decay   = parameters[1]->getValue();    // ms
            float sustain = parameters[2]->getValue();    // 0-1
            float release = parameters[3]->getValue();    // ms
            float amount  = parameters[4]->getValue();    // -100 to 100

            // Normalize to visual widths
            float totalMs = attack + decay + release + 200.0f; // add sustain hold region
            float aW = (attack / totalMs) * r.getWidth();
            float dW = (decay / totalMs) * r.getWidth();
            float sW = (200.0f / totalMs) * r.getWidth();       // sustain hold region
            float rW = (release / totalMs) * r.getWidth();

            float top = r.getY() + 10;
            float bottom = r.getBottom() - 10;
            float range = bottom - top;

            // Flip visual if amount is negative
            float peakY = (amount >= 0.0f) ? top : bottom;
            float sustainY = bottom - sustain * range * (amount >= 0.0f ? 1.0f : -1.0f);
            float zeroY = (amount >= 0.0f) ? bottom : top;

            if (amount < 0.0f)
            {
                peakY = bottom;
                sustainY = top + sustain * range;
                zeroY = top;
            }

            juce::Path envPath;
            float x = r.getX();

            envPath.startNewSubPath (x, zeroY);

            // Attack: zero to peak
            x += aW;
            envPath.lineTo (x, peakY);

            // Decay: peak to sustain
            x += dW;
            envPath.lineTo (x, sustainY);

            // Sustain hold
            x += sW;
            envPath.lineTo (x, sustainY);

            // Release: sustain to zero
            x += rW;
            envPath.lineTo (x, zeroY);

            // Draw fill
            juce::Path fillPath = envPath;
            fillPath.lineTo (x, zeroY);
            fillPath.lineTo (r.getX(), zeroY);
            fillPath.closeSubPath();

            auto fillColor = accentColor.withAlpha (0.12f);
            g.setColour (fillColor);
            g.fillPath (fillPath);

            // Draw stroke
            auto drawColor = (amount > 0.01f || amount < -0.01f) ? accentColor : accentColor.withAlpha (0.4f);
            g.setColour (drawColor);
            g.strokePath (envPath, juce::PathStrokeType (2.5f, juce::PathStrokeType::curved));

            // Draw zero line
            g.setColour (accentColor.withAlpha (0.15f));
            float zeroLineY = (amount >= 0.0f) ? bottom : top;
            g.drawHorizontalLine ((int) zeroLineY, r.getX(), r.getRight());

            // Amount label
            g.setColour (accentColor.withAlpha (0.5f));
            g.setFont (14.0f);
            juce::String amtText = (amount >= 0.0f ? "+" : "") + juce::String ((int) amount) + "%";
            g.drawText (amtText, r.withHeight (20).withY ((int) r.getY()), juce::Justification::centredRight);
        }
    };

    /**
     * FmFilterModule  
     * Simplified filter module for FM synth (LP/HP/BP post-FM).
     */
    class FmFilterModule : public ModuleBase
    {
    public:
        FmFilterModule (const juce::String& name, const juce::Colour& color)
            : ModuleBase (name, color)
        {
            addChoiceParameter ("Type", { "Low Pass", "High Pass", "Band Pass" }, 0);
            addParameter ("Cutoff", 20.0f, 20000.0f, 20000.0f);
            addParameter ("Res", 0.0f, 1.0f, 0.0f);
            addParameter ("Slope", 0.0f, 1.0f, 1.0f, true);
            if (auto* p = parameters.back()) p->setBinaryLabels ("12dB", "24dB");

            addParameter ("KeyTrack", 0.0f, 1.0f, 0.0f, false, 0.0f, false, true);
            addSpacer();
            addSpacer();
            addSpacer();

            lastAdjustedIndex = 0;
        }

    protected:
        void paintVisualization (juce::Graphics& g, juce::Rectangle<int> area) override
        {
            auto r = area.reduced (100, 80).toFloat();

            g.setColour (accentColor.withAlpha (0.05f));
            g.fillRoundedRectangle (r, 5.0f);

            int filterType = (int) parameters[0]->getValue();
            float cutoff = parameters[1]->getValue();
            float res = parameters[2]->getValue();
            bool is24dB = parameters[3]->getValue() > 0.5f;

            float height = r.getHeight();
            float width = r.getWidth();
            float startX = r.getX();
            float zeroGainY = r.getBottom() - 5.0f;
            float unityGainY = r.getY() + height * 0.55f;
            float gainRange = zeroGainY - unityGainY;

            float passbandGain = 1.0f - (res * 0.45f);
            float peakBoost = res * 1.8f;

            juce::Path path;
            auto freqToX = [&](float f) {
                return startX + (std::log10 (f / 20.0f) / 3.0f) * width;
            };

            const int numPoints = 120;
            for (int i = 0; i <= numPoints; ++i)
            {
                float t = i / (float) numPoints;
                float freq = 20.0f * std::pow (1000.0f, t);
                float ratio = freq / cutoff;

                float order = is24dB ? 4.0f : 2.0f;
                float mag = 1.0f;

                if (filterType == 0) // LP
                {
                    mag = 1.0f / std::sqrt (1.0f + std::pow (ratio, order * 2.0f));
                    float bell = std::exp (-std::pow (ratio - 1.0f, 2.0f) / (0.15f - (res * 0.1f)));
                    mag = (mag * passbandGain) + (bell * peakBoost * mag);
                }
                else if (filterType == 1) // HP
                {
                    mag = 1.0f / std::sqrt (1.0f + std::pow (1.0f / ratio, order * 2.0f));
                    float bell = std::exp (-std::pow (ratio - 1.0f, 2.0f) / (0.15f - (res * 0.1f)));
                    mag = (mag * passbandGain) + (bell * peakBoost * mag);
                }
                else // BP
                {
                    float resonanceFactor = 1.0f / (0.5f - (res * 0.45f));
                    mag = 1.0f / std::sqrt (1.0f + std::pow (resonanceFactor * (ratio - 1.0f / ratio), 2.0f));
                }

                float x = freqToX (freq);
                float y = zeroGainY - (mag * gainRange);
                y = juce::jlimit (r.getY(), r.getBottom(), y);

                if (i == 0) path.startNewSubPath (x, y);
                else path.lineTo (x, y);
            }

            auto drawColor = accentColor;
            juce::Path fillPath = path;
            fillPath.lineTo (r.getRight(), r.getBottom());
            fillPath.lineTo (r.getX(), r.getBottom());
            fillPath.closeSubPath();

            juce::ColourGradient grad (drawColor.withAlpha (0.15f), 0, unityGainY,
                                       drawColor.withAlpha (0.02f), 0, r.getBottom(), false);
            g.setGradientFill (grad);
            g.fillPath (fillPath);

            g.setColour (drawColor);
            g.strokePath (path, juce::PathStrokeType (3.0f, juce::PathStrokeType::curved));

            float cutoffX = freqToX (cutoff);
            g.setColour (drawColor.withAlpha (0.2f));
            g.drawVerticalLine ((int) cutoffX, r.getY(), r.getBottom());
        }
    };

} // namespace neon
