#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "../core/NeonParameterTemplate.h"
#include "NeonToggle.h"

namespace neon
{
    /**
     * NeonParameterCard
     * The definitive interaction molecule. Combines a label, a horizontal bar, 
     * and a 7-segment style value display.
     */
    class ParameterCard : public juce::Component
    {
    public:
        ParameterCard (const juce::String& name, const juce::Colour& accentColor, bool isBinary = false);
        ParameterCard (const ParameterTemplate& config);
        ParameterCard (ManagedParameter& param, const juce::Colour& accentColor);
        ~ParameterCard() override = default;

        void paint (juce::Graphics&) override;
        void resized() override;
        void mouseDoubleClick (const juce::MouseEvent& e) override;
        
        void setName (const juce::String& newName) override;
        void setValue (float normalizedValue);
        void setValueText (const juce::String& newText);
        void refreshChoices (const std::vector<juce::String>& choices);
        void resetToDefault();

        std::function<void (float)> onValueChanged;

    private:
        void setupSlider();

        juce::Label label;
        juce::Slider slider;
        NeonToggle toggle;
        juce::ComboBox choiceCombo;
        juce::Label valueDisplay;
        
        bool isBinaryMode;
        bool isChoiceMode;
        float defaultValue = 0.0f;
        
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ParameterCard)
    };
}
