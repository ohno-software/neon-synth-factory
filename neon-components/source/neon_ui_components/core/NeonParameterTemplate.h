#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "NeonColors.h"

namespace neon
{
    /**
     * ParameterTemplate
     * A simple structure to define a parameter's properties.
     */
    struct ParameterTemplate
    {
        juce::String name;
        float minValue;
        float maxValue;
        float defaultValue;
        juce::Colour accentColor;
        
        static ParameterTemplate getDefault()
        {
            return { "Default", 0.0f, 128.0f, 64.0f, Colors::oscillator };
        }
    };
}
