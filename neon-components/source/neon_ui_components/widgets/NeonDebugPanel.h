#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "../core/NeonParameterRegistry.h"

namespace neon
{
    /**
     * NeonDebugPanel
     * A sidebar that lists all registered synth parameters and their current values.
     * High-visibility debug tool.
     */
    class NeonDebugPanel : public juce::Component, public juce::Timer
    {
    public:
        NeonDebugPanel()
        {
            startTimerHz (30);
        }

        void timerCallback() override
        {
            repaint();
        }

        void paint (juce::Graphics& g) override
        {
            auto bounds = getLocalBounds();
            
            // Background
            g.setColour (juce::Colours::black.withAlpha (0.8f));
            g.fillRect (bounds);
            
            g.setColour (juce::Colours::white.withAlpha (0.1f));
            g.drawRect (bounds, 1.0f);

            // Header
            auto headerArea = bounds.removeFromTop (40);
            g.setColour (juce::Colours::yellow);
            g.setFont (juce::FontOptions (18.0f).withStyle ("Bold"));
            g.drawText ("DEBUG PARAMETERS", headerArea.reduced (10, 0), juce::Justification::centredLeft);
            
            g.setColour (juce::Colours::white.withAlpha (0.3f));
            g.drawHorizontalLine (headerArea.getBottom(), bounds.getX(), bounds.getRight());

            // List
            g.setFont (juce::FontOptions (12.0f)); // Slightly smaller font to fit more
            int startY = headerArea.getBottom() + 10;
            int x = bounds.getX() + 10;
            int y = startY;
            int rowH = 18;
            int numCols = 3;
            int colW = (bounds.getWidth() - (10 * (numCols + 1))) / numCols;

            const auto& params = ParameterRegistry::getInstance().getParameters();
            
            for (auto const& [path, param] : params)
            {
                if (y + rowH > getHeight() - 10) 
                {
                    // Move to next column
                    x += colW + 10;
                    y = startY;
                }
                
                if (x + colW > bounds.getRight()) break;

                auto row = juce::Rectangle<int> (x, y, colW, rowH);
                
                // Path/Name (strip prefix if possible to save space, or just draw narrow)
                g.setColour (juce::Colours::grey);
                juce::String displayName = path;
                g.drawText (displayName, row, juce::Justification::centredLeft);
                
                // Value
                g.setColour (juce::Colours::cyan);
                juce::String valStr;
                if (param->hasChoices())
                {
                    valStr = param->getChoiceLabel ((int)param->getValue());
                }
                else if (param->getIsBoolean())
                {
                    valStr = param->getBinaryLabel (param->getValue() > 0.5f);
                }
                else
                {
                    valStr = juce::String (param->getValue(), 2);
                }
                    
                g.drawText (valStr, row, juce::Justification::centredRight);
                
                y += rowH;
            }
        }

    private:
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NeonDebugPanel)
    };
}
