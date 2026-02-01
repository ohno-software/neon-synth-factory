#include "NeonBar.h"

namespace neon
{
    NeonBar::NeonBar (bool isBinary) : isBinaryMode (isBinary)
    {
        setSliderStyle (juce::Slider::LinearHorizontal);
        setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    }

    void NeonBar::mouseDown (const juce::MouseEvent& e)
    {
        if (isBinaryMode)
        {
            setValue (getValue() > 0.5 ? 0.0 : 1.0);
            return;
        }
        
        juce::Slider::mouseDown (e);
    }

    void NeonBar::mouseDrag (const juce::MouseEvent& e)
    {
        if (isBinaryMode) return;
        
        juce::Slider::mouseDrag (e);
    }
}
