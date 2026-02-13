#pragma once

#include <neon_ui_components/neon_ui_components.h>
#include <vector>
#include <map>

namespace neon
{
    // ============================================================
    // BassModule
    // Params: Preset, Pattern On, Pattern, Step Len,
    //         LPF, Volume, Delay, [spacer],
    //         Dly Time, Dly Mix, [spacer], [spacer]
    // Display: Clickable 16-step grid (2×8) for bass sequencer
    // ============================================================
    class BassModule : public ModuleBase
    {
    public:
        BassModule (const juce::String& name, const juce::Colour& color)
            : ModuleBase (name, color)
        {
            // Page 1, Row 1
            std::vector<juce::String> presetNames;
            for (int i = 0; i < 16; ++i)
                presetNames.push_back ("Preset " + juce::String (i + 1));
            addChoiceParameter ("Preset", presetNames, 0);

            addParameter ("Pattern On", 0.0f, 1.0f, 0.0f, true);
            if (auto* p = parameters.back()) p->setBinaryLabels ("OFF", "ON");

            std::vector<juce::String> patterns = { "Off", "8th Drive", "Oct Bounce", "Sync Pulse", "Pump 8ths", "Stac 16ths", "User Seq" };
            addChoiceParameter ("Pattern", patterns, 0);

            std::vector<juce::String> stepLens = { "1/1", "1/2", "1/4", "1/8", "1/16", "1/32" };
            addChoiceParameter ("Step Len", stepLens, 4);

            // Page 1, Row 2
            addParameter ("LPF", 20.0f, 20000.0f, 20000.0f);
            addParameter ("Volume", 0.0f, 1.0f, 0.8f);
            addParameter ("Delay", 0.0f, 1.0f, 0.0f, true);
            if (auto* p = parameters.back()) p->setBinaryLabels ("OFF", "ON");
            addSpacer();

            // Page 2, Row 1
            std::vector<juce::String> dlyTimes = { "1/16", "1/8", "1/4", "1/2", "1/1" };
            addChoiceParameter ("Dly Time", dlyTimes, 1);
            addParameter ("Dly Mix", 0.0f, 1.0f, 0.3f);
            addSpacer();
            addSpacer();

            // Page 2, Row 2
            addSpacer();
            addSpacer();
            addSpacer();
            addSpacer();

            // Register hidden step parameters directly in the registry (not as cards)
            auto& reg = ParameterRegistry::getInstance();
            for (int i = 0; i < 16; ++i)
                stepParams[i] = reg.getOrCreateParameter (name, "Step " + juce::String (i + 1), 0.0f, 1.0f, 1.0f, true);

            moduleNameDisplay.setText ("BASS ENGINE", juce::dontSendNotification);
            lastAdjustedIndex = 0;
        }

    protected:
        ManagedParameter* stepParams[16] = {};

        struct GridLayout
        {
            float gridX, gridY, cellW, cellH;
            int cols = 8, rows = 2;
            static constexpr float gap = 4.0f;
        };

        GridLayout getGridLayout (juce::Rectangle<int> area) const
        {
            GridLayout gl;
            auto r = area.reduced (40, 30).toFloat();
            float totalW = r.getWidth();
            float totalH = r.getHeight();

            gl.cellW = (totalW - (gl.cols - 1) * gl.gap) / gl.cols;
            gl.cellH = (totalH - (gl.rows - 1) * gl.gap) / gl.rows;
            gl.cellH = juce::jmin (gl.cellH, gl.cellW * 0.85f);

            float gridW = gl.cols * gl.cellW + (gl.cols - 1) * gl.gap;
            float gridH = gl.rows * gl.cellH + (gl.rows - 1) * gl.gap;
            gl.gridX = r.getX() + (totalW - gridW) * 0.5f;
            gl.gridY = r.getY() + (totalH - gridH) * 0.5f;
            return gl;
        }

        void paintVisualization (juce::Graphics& g, juce::Rectangle<int> area) override
        {
            auto bg = area.reduced (30, 20).toFloat();
            g.setColour (accentColor.withAlpha (0.03f));
            g.fillRoundedRectangle (bg, 8.0f);

            auto gl = getGridLayout (area);

            for (int i = 0; i < 16; ++i)
            {
                int col = i % gl.cols;
                int row = i / gl.cols;
                float x = gl.gridX + col * (gl.cellW + gl.gap);
                float y = gl.gridY + row * (gl.cellH + gl.gap);

                bool active = stepParams[i] ? stepParams[i]->getValue() > 0.5f : false;

                g.setColour (active ? accentColor.withAlpha (0.2f) : accentColor.withAlpha (0.04f));
                g.fillRect (x, y, gl.cellW, gl.cellH);

                g.setColour (active ? accentColor : accentColor.withAlpha (0.25f));
                g.drawRect (x, y, gl.cellW, gl.cellH, 1.5f);

                if (active)
                {
                    auto inner = juce::Rectangle<float> (x, y, gl.cellW, gl.cellH).reduced (5.0f);
                    g.setColour (accentColor);
                    g.fillRect (inner);
                }

                g.setColour (active ? juce::Colours::black : accentColor.withAlpha (0.35f));
                g.setFont (12.0f);
                g.drawText (juce::String (i + 1), juce::Rectangle<float> (x, y, gl.cellW, gl.cellH),
                            juce::Justification::centred);
            }

            g.setFont (13.0f);
            g.setColour (accentColor.withAlpha (0.3f));
            g.drawText ("BASS STEPS", bg, juce::Justification::centredBottom);
        }

        void handleVisualizationInteraction (const juce::MouseEvent& e, bool isDrag) override
        {
            if (isDrag) return;

            auto bounds = getLocalBounds();
            float unitH = (float) bounds.getHeight() / 10.0f;
            auto displayArea = bounds.removeFromTop ((int) (unitH * 7)).reduced (2);
            auto gl = getGridLayout (displayArea);
            auto pos = e.getPosition().toFloat();

            for (int i = 0; i < 16; ++i)
            {
                int col = i % gl.cols;
                int row = i / gl.cols;
                float x = gl.gridX + col * (gl.cellW + gl.gap);
                float y = gl.gridY + row * (gl.cellH + gl.gap);

                if (juce::Rectangle<float> (x, y, gl.cellW, gl.cellH).contains (pos))
                {
                    if (stepParams[i])
                    {
                        bool current = stepParams[i]->getValue() > 0.5f;
                        stepParams[i]->setValue (current ? 0.0f : 1.0f);
                        repaint();
                    }
                    return;
                }
            }
        }
    };

    // ============================================================
    // PadModule
    // Page 1: Preset, Chorus Type, Chorus Mix, Volume
    //         Delay, Dly Time, Dly Mix, [spacer]
    // Page 2: Reverb, Rvb Time, Rvb Mix, [spacer]
    //         [spacer], [spacer], [spacer], [spacer]
    // ============================================================
    class PadModule : public ModuleBase
    {
    public:
        PadModule (const juce::String& name, const juce::Colour& color)
            : ModuleBase (name, color)
        {
            // Page 1, Row 1
            std::vector<juce::String> presetNames;
            for (int i = 0; i < 16; ++i)
                presetNames.push_back ("Preset " + juce::String (i + 1));
            addChoiceParameter ("Preset", presetNames, 0);

            std::vector<juce::String> chorusTypes = { "Type I", "Type II", "Type III" };
            addChoiceParameter ("Chorus", chorusTypes, 0);
            addParameter ("Chorus Mix", 0.0f, 1.0f, 0.5f);
            addParameter ("Volume", 0.0f, 1.0f, 0.8f);

            // Page 1, Row 2
            addParameter ("Delay", 0.0f, 1.0f, 0.0f, true);
            if (auto* p = parameters.back()) p->setBinaryLabels ("OFF", "ON");

            std::vector<juce::String> dlyTimes = { "1/16", "1/8", "1/4", "1/2", "1/1" };
            addChoiceParameter ("Dly Time", dlyTimes, 2);
            addParameter ("Dly Mix", 0.0f, 1.0f, 0.25f);
            addSpacer();

            // Page 2, Row 1
            addParameter ("Reverb", 0.0f, 1.0f, 1.0f, true);
            if (auto* p = parameters.back()) p->setBinaryLabels ("OFF", "ON");
            addParameter ("Rvb Time", 0.1f, 10.0f, 2.5f);
            addParameter ("Rvb Mix", 0.0f, 1.0f, 0.35f);
            addSpacer();

            // Page 2, Row 2
            addSpacer();
            addSpacer();
            addSpacer();
            addSpacer();

            moduleNameDisplay.setText ("PAD ENGINE", juce::dontSendNotification);
            lastAdjustedIndex = 0;
        }

    protected:
        void paintVisualization (juce::Graphics& g, juce::Rectangle<int> area) override
        {
            auto r = area.reduced (100, 80).toFloat();
            g.setColour (accentColor.withAlpha (0.05f));
            g.fillRoundedRectangle (r, 8.0f);
            g.setColour (accentColor.withAlpha (0.15f));
            g.drawRoundedRectangle (r, 8.0f, 1.0f);

            // Draw layered pad waves
            float w = r.getWidth();
            float h = r.getHeight();
            float cy = r.getCentreY();

            for (int layer = 0; layer < 3; ++layer)
            {
                float alpha = 0.3f - layer * 0.08f;
                float freq = 1.0f + layer * 0.7f;
                float offset = layer * 0.3f;

                juce::Path wave;
                wave.startNewSubPath (r.getX(), cy);

                for (float x = 0; x < w; x += 2.0f)
                {
                    float phase = (x / w) + offset;
                    float sample = std::sin (phase * juce::MathConstants<float>::twoPi * freq) * 0.3f;
                    wave.lineTo (r.getX() + x, cy - sample * h * 0.4f);
                }

                g.setColour (accentColor.withAlpha (alpha));
                g.strokePath (wave, juce::PathStrokeType (1.5f));
            }

            g.setFont (16.0f);
            g.setColour (accentColor.withAlpha (0.4f));
            g.drawText ("PAD", r, juce::Justification::centredBottom);
        }
    };

    // ============================================================
    // ArpSplitModule
    // Page 1: Arp On, Waveform, Pattern, Volume
    //         Filter, Resonance, [spacer], [spacer]
    // Page 2: Delay, Dly Time, Dly Mix, [spacer]
    //         Reverb, Rvb Time, Rvb Mix, [spacer]
    // ============================================================
    class ArpSplitModule : public ModuleBase
    {
    public:
        ArpSplitModule (const juce::String& name, const juce::Colour& color)
            : ModuleBase (name, color)
        {
            // Page 1, Row 1
            addParameter ("Arp On", 0.0f, 1.0f, 0.0f, true);
            if (auto* p = parameters.back()) p->setBinaryLabels ("OFF", "ON");

            std::vector<juce::String> waveforms = { "FM", "Pulse", "Saw", "Sine" };
            addChoiceParameter ("Waveform", waveforms, 2);

            std::vector<juce::String> patterns = { "Up", "Down", "Up/Down", "Synth Gate", "Random" };
            addChoiceParameter ("Pattern", patterns, 0);

            addParameter ("Volume", 0.0f, 1.0f, 0.7f);

            // Page 1, Row 2
            addParameter ("Filter", 20.0f, 20000.0f, 20000.0f);
            addParameter ("Resonance", 0.0f, 1.0f, 0.0f, true);
            if (auto* p = parameters.back()) p->setBinaryLabels ("OFF", "ON");
            addSpacer();
            addSpacer();

            // Page 2, Row 1
            addParameter ("Delay", 0.0f, 1.0f, 0.0f, true);
            if (auto* p = parameters.back()) p->setBinaryLabels ("OFF", "ON");

            std::vector<juce::String> dlyTimes = { "1/16", "1/8", "1/4", "1/2", "1/1" };
            addChoiceParameter ("Dly Time", dlyTimes, 1);
            addParameter ("Dly Mix", 0.0f, 1.0f, 0.3f);
            addSpacer();

            // Page 2, Row 2
            addParameter ("Reverb", 0.0f, 1.0f, 0.0f, true);
            if (auto* p = parameters.back()) p->setBinaryLabels ("OFF", "ON");
            addParameter ("Rvb Time", 0.1f, 10.0f, 1.5f);
            addParameter ("Rvb Mix", 0.0f, 1.0f, 0.25f);
            addSpacer();

            moduleNameDisplay.setText ("ARPEGGIATOR", juce::dontSendNotification);
            lastAdjustedIndex = 0;
        }

    protected:
        void paintVisualization (juce::Graphics& g, juce::Rectangle<int> area) override
        {
            auto r = area.reduced (100, 80).toFloat();
            g.setColour (accentColor.withAlpha (0.05f));
            g.fillRoundedRectangle (r, 8.0f);
            g.setColour (accentColor.withAlpha (0.15f));
            g.drawRoundedRectangle (r, 8.0f, 1.0f);

            // Draw arp step pattern
            float w = r.getWidth();
            float h = r.getHeight();
            float stepW = w / 8.0f;

            int patternIdx = (parameters.size() > 2) ? static_cast<int> (std::round (parameters[2]->getValue())) : 0;

            g.setColour (accentColor);
            for (int i = 0; i < 8; ++i)
            {
                float stepX = r.getX() + (i * stepW) + 2;
                float barW = stepW - 4;
                float val = 0.5f;

                if (patternIdx == 0)      val = 0.2f + (i % 4) * 0.2f;         // Up
                else if (patternIdx == 1) val = 0.8f - (i % 4) * 0.2f;         // Down
                else if (patternIdx == 2) val = (i < 4) ? (0.2f + i * 0.2f)    // Up/Down
                                                        : (0.8f - (i-4) * 0.2f);
                else if (patternIdx == 3) val = (i % 2 == 0) ? 0.7f : 0.0f;    // Synth Gate
                else val = juce::Random::getSystemRandom().nextFloat() * 0.6f + 0.2f;

                float barH = val * h * 0.7f;
                g.fillRect (stepX, r.getBottom() - barH, barW, barH);
            }

            g.setFont (16.0f);
            g.setColour (accentColor.withAlpha (0.4f));
            g.drawText ("ARP", r, juce::Justification::centredBottom);
        }
    };

    // ============================================================
    // DrumModule
    // Params: Drum On, HH Tone, Snare Rev
    // Display: Clickable 3×16 grid (K/S/H lanes) for drum sequencer
    // ============================================================
    class DrumModule : public ModuleBase
    {
    public:
        DrumModule (const juce::String& name, const juce::Colour& color)
            : ModuleBase (name, color)
        {
            // Page 1, Row 1
            addParameter ("Drum On", 0.0f, 1.0f, 0.0f, true);
            if (auto* p = parameters.back()) p->setBinaryLabels ("OFF", "ON");

            addParameter ("HH Tone", 500.0f, 15000.0f, 5000.0f);
            addParameter ("Snare Rev", 0.0f, 1.0f, 0.3f);
            addSpacer();

            // Page 1, Row 2
            addSpacer();
            addSpacer();
            addSpacer();
            addSpacer();

            // Register hidden step parameters directly in the registry (not as cards)
            auto& reg = ParameterRegistry::getInstance();
            for (int i = 0; i < 16; ++i)
            {
                kickSteps[i]  = reg.getOrCreateParameter (name, "K " + juce::String (i + 1), 0.0f, 1.0f, 0.0f, true);
                snareSteps[i] = reg.getOrCreateParameter (name, "S " + juce::String (i + 1), 0.0f, 1.0f, 0.0f, true);
                hihatSteps[i] = reg.getOrCreateParameter (name, "H " + juce::String (i + 1), 0.0f, 1.0f, 0.0f, true);
            }

            moduleNameDisplay.setText ("DRUM MACHINE", juce::dontSendNotification);
            lastAdjustedIndex = 0;
        }

    protected:
        ManagedParameter* kickSteps[16]  = {};
        ManagedParameter* snareSteps[16] = {};
        ManagedParameter* hihatSteps[16] = {};

        static constexpr int kLanes = 3;
        static constexpr int kSteps = 16;

        struct DrumGridLayout
        {
            float gridX, gridY, cellW, cellH, labelW;
            static constexpr float gapX = 3.0f;
            static constexpr float gapY = 4.0f;
        };

        ManagedParameter* getStepParam (int lane, int step) const
        {
            switch (lane)
            {
                case 0: return kickSteps[step];
                case 1: return snareSteps[step];
                case 2: return hihatSteps[step];
                default: return nullptr;
            }
        }

        DrumGridLayout getDrumGridLayout (juce::Rectangle<int> area) const
        {
            DrumGridLayout gl;
            auto r = area.reduced (30, 20).toFloat();

            gl.labelW = 24.0f;
            float availW = r.getWidth() - gl.labelW;
            float availH = r.getHeight();

            gl.cellW = (availW - (kSteps - 1) * gl.gapX) / kSteps;
            gl.cellH = (availH - (kLanes - 1) * gl.gapY) / kLanes;
            gl.cellH = juce::jmin (gl.cellH, gl.cellW * 0.7f);

            float gridW = kSteps * gl.cellW + (kSteps - 1) * gl.gapX;
            float gridH = kLanes * gl.cellH + (kLanes - 1) * gl.gapY;

            gl.gridX = r.getX() + gl.labelW + (availW - gridW) * 0.5f;
            gl.gridY = r.getY() + (availH - gridH) * 0.5f;
            return gl;
        }

        void paintVisualization (juce::Graphics& g, juce::Rectangle<int> area) override
        {
            auto bg = area.reduced (20, 10).toFloat();
            g.setColour (accentColor.withAlpha (0.03f));
            g.fillRoundedRectangle (bg, 8.0f);

            auto gl = getDrumGridLayout (area);
            const char* laneLabels[] = { "K", "S", "H" };

            for (int lane = 0; lane < kLanes; ++lane)
            {
                float laneY = gl.gridY + lane * (gl.cellH + gl.gapY);

                g.setColour (accentColor.withAlpha (0.6f));
                g.setFont (16.0f);
                g.drawText (laneLabels[lane],
                            juce::Rectangle<float> (gl.gridX - gl.labelW - 4, laneY, gl.labelW, gl.cellH),
                            juce::Justification::centredRight);

                for (int step = 0; step < kSteps; ++step)
                {
                    float x = gl.gridX + step * (gl.cellW + gl.gapX);
                    auto* sp = getStepParam (lane, step);
                    bool active = sp ? sp->getValue() > 0.5f : false;

                    g.setColour (active ? accentColor.withAlpha (0.2f) : accentColor.withAlpha (0.04f));
                    g.fillRect (x, laneY, gl.cellW, gl.cellH);

                    g.setColour (active ? accentColor : accentColor.withAlpha (0.2f));
                    g.drawRect (x, laneY, gl.cellW, gl.cellH, 1.5f);

                    if (active)
                    {
                        auto inner = juce::Rectangle<float> (x, laneY, gl.cellW, gl.cellH).reduced (4.0f);
                        g.setColour (accentColor);
                        g.fillRect (inner);
                    }

                    if (step % 4 == 0 && lane == 0)
                    {
                        g.setColour (accentColor.withAlpha (0.2f));
                        g.setFont (9.0f);
                        g.drawText (juce::String (step / 4 + 1),
                                    juce::Rectangle<float> (x, gl.gridY - 14, gl.cellW, 12),
                                    juce::Justification::centred);
                    }
                }
            }

            g.setFont (13.0f);
            g.setColour (accentColor.withAlpha (0.3f));
            g.drawText ("DRUM PATTERN", bg, juce::Justification::centredBottom);
        }

        void handleVisualizationInteraction (const juce::MouseEvent& e, bool isDrag) override
        {
            if (isDrag) return;

            auto bounds = getLocalBounds();
            float unitH = (float) bounds.getHeight() / 10.0f;
            auto displayArea = bounds.removeFromTop ((int) (unitH * 7)).reduced (2);

            auto gl = getDrumGridLayout (displayArea);
            auto pos = e.getPosition().toFloat();

            for (int lane = 0; lane < kLanes; ++lane)
            {
                float laneY = gl.gridY + lane * (gl.cellH + gl.gapY);

                for (int step = 0; step < kSteps; ++step)
                {
                    float x = gl.gridX + step * (gl.cellW + gl.gapX);

                    if (juce::Rectangle<float> (x, laneY, gl.cellW, gl.cellH).contains (pos))
                    {
                        auto* sp = getStepParam (lane, step);
                        if (sp)
                        {
                            bool current = sp->getValue() > 0.5f;
                            sp->setValue (current ? 0.0f : 1.0f);
                            repaint();
                        }
                        return;
                    }
                }
            }
        }
    };

    // ============================================================
    // SplitControlModule
    // Master controls for the split synth
    // Page 1: Split Point, Sync Mode, Master Vol, [spacer]
    //         [spacer], [spacer], [spacer], [spacer]
    // ============================================================
    class SplitControlModule : public ModuleBase
    {
    public:
        SplitControlModule (const juce::String& name, const juce::Colour& color)
            : ModuleBase (name, color)
        {
            // Page 1, Row 1
            addParameter ("Split Point", 24.0f, 84.0f, 60.0f, false, 1.0f, false, true);
            addChoiceParameter ("Sync Mode", { "Host Sync", "Free Run" }, 0);
            addParameter ("Master Vol", 0.0f, 1.0f, 0.8f);
            addSpacer();

            // Page 1, Row 2
            addSpacer();
            addSpacer();
            addSpacer();
            addSpacer();

            moduleNameDisplay.setText ("GLOBAL CONTROLS", juce::dontSendNotification);
            lastAdjustedIndex = 0;
        }

    protected:
        void paintVisualization (juce::Graphics& g, juce::Rectangle<int> area) override
        {
            auto r = area.reduced (80, 60).toFloat();
            g.setColour (accentColor.withAlpha (0.05f));
            g.fillRoundedRectangle (r, 8.0f);
            g.setColour (accentColor.withAlpha (0.15f));
            g.drawRoundedRectangle (r, 8.0f, 1.0f);

            // Draw keyboard split visualization
            float w = r.getWidth();
            float h = r.getHeight();
            float keyboardY = r.getCentreY() - 20.0f;
            float keyH = 40.0f;

            int splitNote = 60;
            if (!parameters.empty())
                splitNote = static_cast<int> (std::round (parameters[0]->getValue()));

            // Draw simplified keyboard (C1 to C5 = notes 24 to 84)
            int totalNotes = 60;
            float noteW = w / (float) totalNotes;
            int splitIdx = splitNote - 24;

            for (int i = 0; i < totalNotes; ++i)
            {
                float x = r.getX() + i * noteW;
                bool isBlack = false;
                int noteInOctave = i % 12;
                if (noteInOctave == 1 || noteInOctave == 3 || noteInOctave == 6 
                    || noteInOctave == 8 || noteInOctave == 10)
                    isBlack = true;

                if (i < splitIdx)
                    g.setColour (isBlack ? juce::Colour (0xFF003366) : juce::Colour (0xFF0066CC));
                else
                    g.setColour (isBlack ? juce::Colour (0xFF663300) : juce::Colour (0xFFCC6600));

                g.fillRect (x, keyboardY, noteW - 0.5f, isBlack ? keyH * 0.6f : keyH);
            }

            // Split line
            float splitX = r.getX() + splitIdx * noteW;
            g.setColour (juce::Colours::white);
            g.drawLine (splitX, keyboardY - 10, splitX, keyboardY + keyH + 10, 2.0f);

            // Labels
            g.setFont (14.0f);
            g.setColour (juce::Colour (0xFF0088FF));
            g.drawText ("BASS", r.getX(), keyboardY + keyH + 12, splitX - r.getX(), 20, juce::Justification::centred);

            g.setColour (juce::Colour (0xFFFF8800));
            g.drawText ("PAD / ARP", splitX, keyboardY + keyH + 12, r.getRight() - splitX, 20, juce::Justification::centred);

            // Note name
            static const char* noteNames[] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
            int octave = (splitNote / 12) - 1;
            int noteName = splitNote % 12;
            g.setColour (juce::Colours::white);
            g.setFont (20.0f);
            g.drawText (juce::String (noteNames[noteName]) + juce::String (octave),
                        r.withHeight (30), juce::Justification::centredTop);
        }
    };

} // namespace neon
