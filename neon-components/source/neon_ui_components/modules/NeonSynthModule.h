#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "../widgets/NeonParameterCard.h"

namespace neon
{
    /**
     * NeonSynthModule
     * Base class for all synth modules (Osc, Filter, Env, etc.)
     * Layout: Top 2/3 Visualization, Bottom 1/3 Parameter Grid (2x4)
     */
    class SynthModule : public juce::Component
    {
    public:
        SynthModule (const juce::String& name, const juce::Colour& accentColor);
        ~SynthModule() override = default;

        void paint (juce::Graphics&) override;
        void resized() override;

        virtual void updateVisualization() = 0;

    protected:
        juce::OwnedArray<ParameterCard> parameters;
        juce::Component* visualizationComponent = nullptr;
        
        juce::Colour categoryColor;
        
    private:
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SynthModule)
    };
}
