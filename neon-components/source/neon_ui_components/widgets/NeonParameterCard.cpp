#include "NeonParameterCard.h"
#include "../core/NeonColors.h"

namespace neon
{
    ParameterCard::ParameterCard (const juce::String& name, const juce::Colour& accentColor, bool isBinary)
        : isBinaryMode (isBinary), isChoiceMode (false), defaultValue (0.0f)
    {
        setupSlider();
        label.setText (name.toUpperCase(), juce::dontSendNotification);
        
        juce::NormalisableRange<double> r (0.0, 1.0);
        if (!isBinaryMode)
            r.setSkewForCentre (0.25);
            
        slider.setNormalisableRange (r);
        
        slider.setColour (juce::Slider::thumbColourId, accentColor);
        slider.setColour (juce::Slider::trackColourId, accentColor.withAlpha (0.5f));
        
        toggle.setAccentColor (accentColor);
        
        bool isBlank = name.isEmpty() || name == " ";
        toggle.setVisible (isBinaryMode && !isBlank);
        slider.setVisible (!isBinaryMode && !isBlank);
        choiceCombo.setVisible (false);
        label.setVisible (!isBlank);
        valueDisplay.setVisible (!isBlank);
    }

    ParameterCard::ParameterCard (const ParameterTemplate& config)
        : isBinaryMode (false), isChoiceMode (false), defaultValue (config.defaultValue)
    {
        setupSlider();
        label.setText (config.name.toUpperCase(), juce::dontSendNotification);
        
        juce::NormalisableRange<double> r ((double)config.minValue, (double)config.maxValue);
        if (config.minValue < config.maxValue)
            r.setSkewForCentre ((double)(config.minValue + (config.maxValue - config.minValue) * 0.25f));
            
        slider.setNormalisableRange (r);
        slider.setValue (config.defaultValue, juce::dontSendNotification);
        
        slider.setColour (juce::Slider::thumbColourId, config.accentColor);
        slider.setColour (juce::Slider::trackColourId, config.accentColor.withAlpha (0.5f));

        toggle.setAccentColor (config.accentColor);

        bool isBlank = config.name.isEmpty() || config.name == " ";
        toggle.setVisible (false);
        slider.setVisible (!isBlank);
        choiceCombo.setVisible (false);
        label.setVisible (!isBlank);
        valueDisplay.setVisible (!isBlank);

        setValueText (juce::String (config.defaultValue, 1));
    }

    ParameterCard::ParameterCard (ManagedParameter& param, const juce::Colour& accentColor)
        : isBinaryMode (param.getIsBoolean()), isChoiceMode (param.hasChoices()), defaultValue (param.getDefaultValue())
    {
        setupSlider();
        label.setText (param.getName().toUpperCase(), juce::dontSendNotification);
        
        slider.setNormalisableRange (param.getRange());
        slider.setValue (param.getValue(), juce::dontSendNotification);
        toggle.setValue (param.getValue(), false);
        toggle.setIsMomentary (param.getIsMomentary());
        toggle.setAccentColor (accentColor);

        bool isBlank = param.getName().isEmpty() || param.getName() == " ";

        if (isChoiceMode)
        {
            int i = 1;
            for (auto& c : param.getChoices())
                choiceCombo.addItem (c, i++);
            
            choiceCombo.setSelectedId ((int)std::round (param.getValue()) + 1, juce::dontSendNotification);
            choiceCombo.setColour (juce::ComboBox::backgroundColourId, juce::Colours::black.withAlpha (0.2f));
            choiceCombo.setColour (juce::ComboBox::outlineColourId, accentColor.withAlpha (0.3f));
            choiceCombo.setColour (juce::ComboBox::textColourId, accentColor);
            choiceCombo.setColour (juce::ComboBox::arrowColourId, accentColor);
        }

        slider.setVisible (!isBinaryMode && !isChoiceMode && !isBlank);
        toggle.setVisible (isBinaryMode && !isBlank);
        choiceCombo.setVisible (isChoiceMode && !isBlank);
        label.setVisible (!isBlank);
        valueDisplay.setVisible (!isBlank);

        slider.setColour (juce::Slider::thumbColourId, accentColor);
        slider.setColour (juce::Slider::trackColourId, accentColor.withAlpha (0.5f));

        auto updateLabels = [this, &param] (float val) {
            if (isBinaryMode)
                setValueText (param.getBinaryLabel (val > 0.5f));
            else if (param.hasChoices())
                setValueText (param.getChoiceLabel ((int)std::round (val)));
            else if (param.getName().equalsIgnoreCase ("Sustain"))
                setValueText (juce::String (val * 100.0f, 0));
            else {
                int decimals = (param.getRange().interval >= 1.0f) ? 0 : 1;
                setValueText (juce::String (val, decimals));
            }
        };

        slider.onValueChange = [this, &param, updateLabels] {
            float val = (float)slider.getValue();
            param.setValue (val);
            toggle.setValue (val, false);
            if (onValueChanged)
                onValueChanged (val);
            updateLabels (val);
        };

        toggle.onValueChanged = [this, &param, updateLabels] (float val) {
            param.setValue (val);
            slider.setValue (val, juce::dontSendNotification);
            if (onValueChanged)
                onValueChanged (val);
            updateLabels (val);
        };

        choiceCombo.onChange = [this, &param, updateLabels] {
            float val = (float)(choiceCombo.getSelectedId() - 1);
            param.setValue (val);
            if (onValueChanged)
                onValueChanged (val);
            updateLabels (val);
        };

        updateLabels (param.getValue());
    }

    void ParameterCard::setupSlider()
    {
        // Setup Label
        label.setJustificationType (juce::Justification::centred);
        label.setFont (juce::FontOptions (14.0f).withStyle ("Bold"));
        label.setInterceptsMouseClicks (false, false);
        addAndMakeVisible (label);

        // Setup Slider/Bar
        slider.setSliderStyle (juce::Slider::LinearHorizontal);
        slider.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
        slider.setDoubleClickReturnValue (true, defaultValue);

        slider.onValueChange = [this] {
            if (onValueChanged)
                onValueChanged ((float)slider.getValue());
            
            if (!isBinaryMode && !isChoiceMode)
                setValueText (juce::String (slider.getValue(), 1));
        };
        addAndMakeVisible (slider);

        // Setup Toggle
        toggle.onValueChanged = [this] (float val) {
            slider.setValue (val, juce::dontSendNotification);
            if (onValueChanged)
                onValueChanged (val);
        };
        toggle.onDoubleClick = [this] { resetToDefault(); };
        addAndMakeVisible (toggle);

        // Setup Choice (ComboBox)
        choiceCombo.addMouseListener (this, false);
        addAndMakeVisible (choiceCombo);

        // Setup Value Display (7-segment pseudo)
        valueDisplay.setText (isBinaryMode ? "[ OFF ]" : "0.0", juce::dontSendNotification);
        valueDisplay.setJustificationType (juce::Justification::centred);
        // Larger font for "display" look
        valueDisplay.setFont (juce::FontOptions (24.0f).withStyle ("Bold")); 
        valueDisplay.setColour (juce::Label::backgroundColourId, juce::Colours::black.withAlpha (0.2f));
        valueDisplay.setInterceptsMouseClicks (false, false);
        addAndMakeVisible (valueDisplay);
    }

    void ParameterCard::paint (juce::Graphics& g)
    {
        if (label.getText().isEmpty() || label.getText() == " ")
            return;

        auto bounds = getLocalBounds().toFloat();
        
        // Draw Card Background
        g.setColour (Colors::cardBackground);
        g.fillRoundedRectangle (bounds.reduced (2.0f), 5.0f);
    }

    void ParameterCard::resized()
    {
        auto b = getLocalBounds();
        
        // Compact layout: Label at top, Display at bottom, Slider/Toggle in middle
        label.setBounds (b.removeFromTop (16));
        
        auto valueArea = b.removeFromBottom (24);
        valueDisplay.setBounds (valueArea);
        
        // Slider/Toggle/Combo takes all remaining space with minimal vertical gap
        auto widgetArea = b.reduced (2, 0);
        slider.setBounds (widgetArea); 
        toggle.setBounds (widgetArea);
        choiceCombo.setBounds (widgetArea);
    }

    void ParameterCard::setName (const juce::String& newName)
    {
        label.setText (newName.toUpperCase(), juce::dontSendNotification);
    }

    void ParameterCard::setValue (float normalizedValue)
    {
        slider.setValue (slider.getRange().clipValue (normalizedValue), juce::sendNotificationSync);
        toggle.setValue (normalizedValue, false);
        choiceCombo.setSelectedId ((int)std::round (normalizedValue) + 1, juce::dontSendNotification);
    }

    void ParameterCard::setValueText (const juce::String& newText)
    {
        valueDisplay.setText (newText, juce::dontSendNotification);
    }

    void ParameterCard::mouseDoubleClick (const juce::MouseEvent&)
    {
        resetToDefault();
    }

    void ParameterCard::resetToDefault()
    {
        if (isChoiceMode)
            choiceCombo.setSelectedId ((int)std::round (defaultValue) + 1);
        else if (isBinaryMode)
            toggle.setValue (defaultValue > 0.5f, true);
        else
            slider.setValue (defaultValue, juce::sendNotification);
    }

    void ParameterCard::refreshChoices (const std::vector<juce::String>& choices)
    {
        if (!isChoiceMode) return;
        
        int currentId = choiceCombo.getSelectedId();
        choiceCombo.clear (juce::dontSendNotification);
        
        int i = 1;
        for (auto& c : choices)
            choiceCombo.addItem (c, i++);
            
        choiceCombo.setSelectedId (currentId, juce::dontSendNotification);
    }
}
