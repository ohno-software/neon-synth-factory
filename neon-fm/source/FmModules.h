#pragma once

#include <neon_ui_components/neon_ui_components.h>
#include "FmAlgorithm.h"

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
        }

    protected:
        void paintVisualization (juce::Graphics& g, juce::Rectangle<int> area) override
        {
            auto r = area.reduced (60, 40).toFloat();
            int algo = (int) parameters[0]->getValue();

            g.setColour (accentColor.withAlpha (0.06f));
            g.fillRoundedRectangle (r, 8.0f);

            // Draw algorithm diagram
            float boxSize = 40.0f;
            float halfBox = boxSize / 2.0f;

            // Positions for 4 operators in a grid
            // We'll lay them out based on algorithm
            struct OpPos { float x, y; };

            float cx = r.getCentreX();
            float cy = r.getCentreY();
            float spacingX = 70.0f;
            float spacingY = 60.0f;

            // Default layout: horizontal row
            OpPos positions[4];
            
            // Compute positions based on algorithm type
            switch ((FmAlgorithmType)algo)
            {
                case FmAlgorithmType::SerialChain: // 1→2→3→4
                    positions[0] = { cx - spacingX * 1.5f, cy };
                    positions[1] = { cx - spacingX * 0.5f, cy };
                    positions[2] = { cx + spacingX * 0.5f, cy };
                    positions[3] = { cx + spacingX * 1.5f, cy };
                    break;

                case FmAlgorithmType::Branch: // 1→2→4, 3→4
                    positions[0] = { cx - spacingX, cy - spacingY * 0.5f };
                    positions[1] = { cx,            cy - spacingY * 0.5f };
                    positions[2] = { cx,            cy + spacingY * 0.5f };
                    positions[3] = { cx + spacingX, cy };
                    break;

                case FmAlgorithmType::DualStack: // (1→2) + (3→4)
                    positions[0] = { cx - spacingX * 0.5f, cy - spacingY * 0.5f };
                    positions[1] = { cx + spacingX * 0.5f, cy - spacingY * 0.5f };
                    positions[2] = { cx - spacingX * 0.5f, cy + spacingY * 0.5f };
                    positions[3] = { cx + spacingX * 0.5f, cy + spacingY * 0.5f };
                    break;

                case FmAlgorithmType::FullParallel: // 1+2+3+4
                    positions[0] = { cx - spacingX * 1.5f, cy };
                    positions[1] = { cx - spacingX * 0.5f, cy };
                    positions[2] = { cx + spacingX * 0.5f, cy };
                    positions[3] = { cx + spacingX * 1.5f, cy };
                    break;

                default: // Generic: row layout
                    positions[0] = { cx - spacingX * 1.5f, cy };
                    positions[1] = { cx - spacingX * 0.5f, cy };
                    positions[2] = { cx + spacingX * 0.5f, cy };
                    positions[3] = { cx + spacingX * 1.5f, cy };
                    break;
            }

            // Draw connections first (behind boxes)
            g.setColour (accentColor.withAlpha (0.4f));
            auto drawArrow = [&] (OpPos from, OpPos to) {
                g.drawArrow (juce::Line<float> (from.x, from.y, to.x, to.y)
                                .withShortenedStart (halfBox + 2)
                                .withShortenedEnd (halfBox + 2),
                             2.0f, 8.0f, 8.0f);
            };

            // Draw connections based on algorithm
            switch ((FmAlgorithmType)algo)
            {
                case FmAlgorithmType::SerialChain:
                    drawArrow (positions[0], positions[1]);
                    drawArrow (positions[1], positions[2]);
                    drawArrow (positions[2], positions[3]);
                    break;
                case FmAlgorithmType::Branch:
                    drawArrow (positions[0], positions[1]);
                    drawArrow (positions[1], positions[3]);
                    drawArrow (positions[2], positions[3]);
                    break;
                case FmAlgorithmType::DualStack:
                    drawArrow (positions[0], positions[1]);
                    drawArrow (positions[2], positions[3]);
                    break;
                case FmAlgorithmType::TripleCarrier:
                    drawArrow (positions[0], positions[3]);
                    drawArrow (positions[1], positions[3]);
                    drawArrow (positions[2], positions[3]);
                    break;
                case FmAlgorithmType::DualMod:
                    drawArrow (positions[0], positions[2]);
                    drawArrow (positions[1], positions[3]);
                    break;
                case FmAlgorithmType::OneToThree:
                    drawArrow (positions[0], positions[1]);
                    drawArrow (positions[0], positions[2]);
                    drawArrow (positions[0], positions[3]);
                    break;
                case FmAlgorithmType::TripleParallel:
                    drawArrow (positions[0], positions[1]);
                    break;
                case FmAlgorithmType::FullParallel:
                    // No connections
                    break;
                case FmAlgorithmType::YShape:
                    drawArrow (positions[0], positions[2]);
                    drawArrow (positions[1], positions[2]);
                    drawArrow (positions[2], positions[3]);
                    break;
                case FmAlgorithmType::Diamond:
                    drawArrow (positions[0], positions[1]);
                    drawArrow (positions[0], positions[2]);
                    drawArrow (positions[1], positions[3]);
                    drawArrow (positions[2], positions[3]);
                    break;
                case FmAlgorithmType::CascadeBranch:
                    drawArrow (positions[0], positions[1]);
                    drawArrow (positions[1], positions[2]);
                    drawArrow (positions[2], positions[3]);
                    drawArrow (positions[0], positions[3]);
                    break;
                default: break;
            }

            // Determine which ops are carriers (output to audio) vs modulators
            auto isCarrier = [algo](int op) -> bool {
                switch ((FmAlgorithmType) algo) {
                    case FmAlgorithmType::SerialChain:     return op == 3;
                    case FmAlgorithmType::Branch:          return op == 3;
                    case FmAlgorithmType::DualStack:       return op == 1 || op == 3;
                    case FmAlgorithmType::TripleCarrier:   return op == 3;
                    case FmAlgorithmType::DualMod:         return op == 2 || op == 3;
                    case FmAlgorithmType::OneToThree:      return op == 1 || op == 2 || op == 3;
                    case FmAlgorithmType::TripleParallel:  return op == 1 || op == 2 || op == 3;
                    case FmAlgorithmType::FullParallel:    return true;
                    case FmAlgorithmType::YShape:          return op == 3;
                    case FmAlgorithmType::Diamond:         return op == 3;
                    case FmAlgorithmType::CascadeBranch:   return op == 3;
                    default: return op == 3;
                }
            };

            // Draw operator boxes
            for (int i = 0; i < 4; ++i)
            {
                auto opRect = juce::Rectangle<float> (positions[i].x - halfBox,
                                                       positions[i].y - halfBox,
                                                       boxSize, boxSize);

                bool carrier = isCarrier (i);

                // Carriers get filled, modulators get outline
                if (carrier)
                {
                    g.setColour (accentColor.withAlpha (0.6f));
                    g.fillRoundedRectangle (opRect, 6.0f);
                    g.setColour (juce::Colours::black);
                }
                else
                {
                    g.setColour (accentColor.withAlpha (0.15f));
                    g.fillRoundedRectangle (opRect, 6.0f);
                    g.setColour (accentColor.withAlpha (0.6f));
                    g.drawRoundedRectangle (opRect, 6.0f, 2.0f);
                    g.setColour (accentColor);
                }

                g.setFont (juce::FontOptions (18.0f));
                g.drawText (juce::String (i + 1), opRect, juce::Justification::centred);
            }

            // Draw output arrow from carriers
            g.setColour (accentColor.withAlpha (0.6f));
            float outY = r.getBottom() - 20;
            g.setFont (12.0f);
            g.drawText ("→ OUT", r.withHeight (20).withY ((int) outY), juce::Justification::centred);
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
