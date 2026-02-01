#include "NeonSynthModule.h"

namespace neon
{
    SynthModule::SynthModule (const juce::String& name, const juce::Colour& accentColor)
        : categoryColor (accentColor)
    {
    }

    void SynthModule::paint (juce::Graphics& g)
    {
        // Drawing logic for the module container
    }

    void SynthModule::resized()
    {
        auto b = getLocalBounds();
        
        // Top 2/3: Visualization
        auto vizArea = b.removeFromTop (getHeight() * 0.66f);
        if (visualizationComponent != nullptr)
            visualizationComponent->setBounds (vizArea);
            
        // Bottom 1/3: Parameter Cards Grid (2x4)
        auto gridArea = b.reduced (10);
        int cardWidth = gridArea.getWidth() / 4;
        int cardHeight = gridArea.getHeight() / 2;
        
        for (int i = 0; i < parameters.size(); ++i)
        {
            int row = i / 4;
            int col = i % 4;
            
            if (auto* card = parameters[i])
            {
                card->setBounds (gridArea.getX() + (col * cardWidth),
                                gridArea.getY() + (row * cardHeight),
                                cardWidth, cardHeight);
            }
        }
    }
}
