#pragma once

#include "ModuleBase.h"
#include <juce_audio_formats/juce_audio_formats.h>

namespace neon
{
    class OscillatorModule : public ModuleBase
    {
    public:
        OscillatorModule (const juce::String& name, const juce::Colour& color) 
            : ModuleBase (name, color)
        {
            // PAGE 1: Row 1: Waveform, Volume, Transp, Detune
            addParameter ("Waveform",  0.0f, 200.0f, 0.0f, false, 1.0f);
            addParameter ("Volume",    0.0f, 1.0f, 0.8f);
            addParameter ("Transp",    -24.0f, 24.0f, 0.0f, false, 1.0f);
            addParameter ("Detune",    -100.0f, 100.0f, 0.0f);
            
            // PAGE 1: Row 2: Phase, KeySync, Unison, USpread
            addParameter ("Phase",     0.0f, 360.0f, 0.0f, false, 0.0f, false, true);
            addParameter ("KeySync",   0.0f, 1.0f, 1.0f, true);
            if (auto* p = parameters.back()) p->setBinaryLabels("OFF", "ON");
            addParameter ("Unison",    1.0f, 8.0f, 1.0f, false, 1.0f);
            addParameter ("USpread",   0.0f, 1.0f, 0.2f);
            
            // PAGE 2: Row 1: Drive, BitRedux, Symmetry, Fold
            addParameter ("Drive",     0.0f, 1.0f, 0.0f);
            addParameter ("BitRedux",  0.0f, 1.0f, 0.0f);
            addParameter ("Symmetry",  0.0f, 1.0f, 0.5f, false, 0.0f, false, true);
            addParameter ("Fold",      0.0f, 1.0f, 0.0f);
            
            // PAGE 2: Row 2: Pan, [Blank], [Blank], [Blank]
            addParameter ("Pan",       -1.0f, 1.0f, 0.0f);
            addSpacer(); // Blank
            addSpacer(); // Blank
            addSpacer(); // Blank

            loadWaveforms();

            waveformSelector.setTextWhenNoChoicesAvailable ("No Waves Found");
            waveformSelector.onChange = [this] { 
                int idx = waveformSelector.getSelectedItemIndex();
                parameters[0]->setValue ((float)idx);
                loadPreviewData (idx);
                lastAdjustedIndex = 0;
            };
            
            // Hide the default slider card for the first parameter
            if (!cards.empty())
                cards[0]->setVisible (false);

            addAndMakeVisible (waveformSelector);
            
            if (waveformSelector.getNumItems() > 0)
                waveformSelector.setSelectedItemIndex (0);
        }

        void resized() override
        {
            ModuleBase::resized();
            
            // Position the dropdown over where the first card's slider would be
            if (!cards.empty() && currentPage == 0)
            {
                auto cardBounds = cards[0]->getBounds();
                
                // Adjust to fit comfortably within the 70px row height
                auto selectorBounds = cardBounds.reduced(10, 0); 
                selectorBounds = selectorBounds.withHeight(35).withY(cardBounds.getY() + 20);
                
                waveformSelector.setBounds (selectorBounds); 
                waveformSelector.setVisible (true);
            }
            else
            {
                waveformSelector.setVisible (false);
            }
        }

        void paintVisualization (juce::Graphics& g, juce::Rectangle<int> area) override
        {
            auto r = area.reduced (120, 100).toFloat();
            
            // Draw background for wave
            g.setColour (accentColor.withAlpha (0.05f));
            g.fillRect (r);
            g.setColour (accentColor.withAlpha (0.15f));
            g.drawRect (r, 1.0f);

            auto* buffer = NeonRegistry::getWaveformBuffer((int)std::round(parameters[0]->getValue()));
            if (buffer != nullptr && buffer->getNumSamples() > 0)
            {
                auto data = buffer->getReadPointer (0);
                juce::Path path;
                float startX = r.getX();
                float midY = r.getCentreY();
                float width = r.getWidth();
                float height = r.getHeight();

                // Parameters affecting visualization (updated indices after reorganization)
                // New order: Waveform(0), Volume(1), Transp(2), Detune(3), Phase(4), KeySync(5), 
                //            Unison(6), USpread(7), Drive(8), BitRedux(9), Symmetry(10), Fold(11), Pan(12)
                float volume   = parameters[1]->getValue();   // 0 to 1
                float phase    = parameters[4]->getValue();   // 0 to 360
                float drive    = parameters[8]->getValue();   // 0 to 1
                float bitRedux = parameters[9]->getValue();  // 0 to 1
                float symmetry = parameters[10]->getValue(); // 0 to 1
                float fold     = parameters[11]->getValue(); // 0 to 1

                path.startNewSubPath (startX, midY - data[0] * height * 0.4f * volume);
                
                int numSamples = buffer->getNumSamples();
                int numSteps = 300; // Resolution for drawing
                
                float phaseOffsetNormalized = phase / 360.0f;

                for (int i = 1; i <= numSteps; ++i)
                {
                    float t = i / (float)numSteps;
                    
                    // Simple symmetry warp (non-linear time) + Phase offset
                    float shiftedT = std::fmod(t + phaseOffsetNormalized, 1.0f);
                    
                    float warpedT = (shiftedT < symmetry) ? 
                        juce::jmap(shiftedT, 0.0f, symmetry, 0.0f, 0.5f) : 
                        juce::jmap(shiftedT, symmetry, 1.0f, 0.5f, 1.0f);
                        
                    int sampleIdx = juce::jlimit(0, numSamples - 1, (int)(warpedT * numSamples));
                    float val = data[sampleIdx] * volume;
                    
                    // Apply Fold
                    if (fold > 0.01f)
                    {
                        val *= (1.0f + fold * 3.0f);
                        for (int f = 0; f < 3; ++f) // Up to 3 folds for visual
                        {
                            if (val > 1.0f) val = 1.0f - (val - 1.0f);
                            if (val < -1.0f) val = -1.0f - (val + 1.0f);
                        }
                    }

                    // Apply Drive (hard clipping)
                    if (drive > 0.01f)
                        val = juce::jlimit(-1.0f, 1.0f, val * (1.0f + drive * 4.0f));

                    // Apply BitRedux (quantization)
                    if (bitRedux > 0.05f)
                    {
                        float steps = juce::jmap(1.0f - bitRedux, 2.0f, 32.0f);
                        val = std::round(val * steps) / steps;
                    }

                    float x = startX + t * width;
                    float y = midY - val * height * 0.45f;
                    
                    path.lineTo (x, y);
                }

                auto drawColor = (lastAdjustedIndex != -1 ? juce::Colours::yellow : accentColor);
                g.setColour (drawColor);
                g.strokePath (path, juce::PathStrokeType (5.0f));
                
                // Glow effect
                g.setColour (drawColor.withAlpha(0.3f));
                g.strokePath (path, juce::PathStrokeType (10.0f));
            }
        }

    private:
        void loadWaveforms()
        {
            auto names = NeonRegistry::getWaveformNames();
            waveformSelector.clear();
            for (int i = 0; i < names.size(); ++i)
                waveformSelector.addItem (names[i], i + 1);
        }

        void loadPreviewData (int index)
        {
            // No longer needed to load files, just repaint
            repaint();
        }

        juce::ComboBox waveformSelector;
    };
}
