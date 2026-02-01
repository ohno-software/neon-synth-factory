#include "ModuleBase.h"
#include "../core/NeonParameterRegistry.h"

namespace neon
{
    ModuleBase::ModuleBase (const juce::String& name, const juce::Colour& color)
        : moduleName (name), accentColor (color), lastAdjustedIndex (-1)
    {
        // Display Area
        unitDisplay.setJustificationType (juce::Justification::centred);
        unitDisplay.setFont (juce::FontOptions ().withHeight (120.0f).withStyle ("Bold"));
        unitDisplay.setColour (juce::Label::backgroundColourId, juce::Colours::transparentBlack);
        unitDisplay.setColour (juce::Label::textColourId, accentColor);
        unitDisplay.setInterceptsMouseClicks (false, false);
        addAndMakeVisible (unitDisplay);

        activeParamNameDisplay.setJustificationType (juce::Justification::bottomLeft);
        activeParamNameDisplay.setFont (juce::FontOptions ().withHeight (32.0f).withStyle ("Bold"));
        activeParamNameDisplay.setColour (juce::Label::textColourId, accentColor.withAlpha (0.7f));
        addAndMakeVisible (activeParamNameDisplay);

        activeParamValueDisplay.setJustificationType (juce::Justification::bottomRight);
        activeParamValueDisplay.setFont (juce::FontOptions ().withHeight (32.0f).withStyle ("Bold"));
        activeParamValueDisplay.setColour (juce::Label::textColourId, accentColor.withAlpha (0.7f));
        addAndMakeVisible (activeParamValueDisplay);

        moduleNameDisplay.setText (moduleName.toUpperCase(), juce::dontSendNotification);
        moduleNameDisplay.setJustificationType (juce::Justification::topLeft);
        moduleNameDisplay.setFont (juce::FontOptions ().withHeight (24.0f).withStyle ("Bold"));
        moduleNameDisplay.setColour (juce::Label::textColourId, accentColor.withAlpha (0.5f));
        addAndMakeVisible (moduleNameDisplay);

        midiIndicator.setText ("MIDI", juce::dontSendNotification);
        midiIndicator.setJustificationType (juce::Justification::topRight);
        midiIndicator.setFont (juce::FontOptions ().withHeight (24.0f).withStyle ("Bold"));
        midiIndicator.setColour (juce::Label::textColourId, juce::Colours::transparentBlack); // Hidden by default
        addAndMakeVisible (midiIndicator);

        patchNameDisplay.setText (NeonRegistry::getCurrentPatchName(), juce::dontSendNotification);
        patchNameDisplay.setJustificationType (juce::Justification::centredTop);
        patchNameDisplay.setFont (juce::FontOptions ().withHeight (24.0f).withStyle ("Bold"));
        patchNameDisplay.setColour (juce::Label::textColourId, accentColor.withAlpha (0.9f));
        addAndMakeVisible (patchNameDisplay);

        // Paging
        prevButton.setButtonText ("<<");
        nextButton.setButtonText (">>");
        prevButton.onClick = [this] { setPage (currentPage - 1); };
        nextButton.onClick = [this] { setPage (currentPage + 1); };
        addAndMakeVisible (prevButton);
        addAndMakeVisible (nextButton);

        startTimerHz (60);
    }

    ModuleBase::~ModuleBase()
    {
        stopTimer();
    }

    void ModuleBase::addParameter (const juce::String& name, float min, float max, float def, bool isBool, float interval, bool isMomentary, bool isLinear)
    {
        int index = (int)parameters.size();
        auto* p = ParameterRegistry::getInstance().getOrCreateParameter (moduleName, name, min, max, def, isBool, interval, isMomentary, isLinear);
        parameters.push_back (p);

        auto card = std::make_unique<ParameterCard> (*p, accentColor);
        card->onValueChanged = [this, index] (float) { lastAdjustedIndex = index; };
        
        addChildComponent (card.get());
        cards.push_back (std::move (card));
        
        numPages = (int)std::ceil (cards.size() / 8.0f);
        updatePageVisibility();
    }

    void ModuleBase::addChoiceParameter (const juce::String& name, const std::vector<juce::String>& choices, int defaultIndex)
    {
        int index = (int)parameters.size();
        auto* p = ParameterRegistry::getInstance().getOrCreateChoiceParameter (moduleName, name, choices, defaultIndex);
        parameters.push_back (p);

        auto card = std::make_unique<ParameterCard> (*p, accentColor);
        card->onValueChanged = [this, index] (float) { lastAdjustedIndex = index; };
        
        addChildComponent (card.get());
        cards.push_back (std::move (card));
        
        numPages = (int)std::ceil (cards.size() / 8.0f);
        updatePageVisibility();
    }

    void ModuleBase::updateChoiceParameter (const juce::String& name, const std::vector<juce::String>& choices)
    {
        for (size_t i = 0; i < parameters.size(); ++i)
        {
            if (parameters[i]->getName() == name)
            {
                parameters[i]->setChoices (choices);
                cards[i]->refreshChoices (choices);
                break;
            }
        }
    }

    void ModuleBase::addSpacer()
    {
        auto card = std::make_unique<ParameterCard> (" ", accentColor);
        card->setVisible (false);
        addChildComponent (card.get());
        cards.push_back (std::move (card));
        
        numPages = (int)std::ceil (cards.size() / 8.0f);
        updatePageVisibility();
    }

    void ModuleBase::loadPatch (const std::map<juce::String, float>& patchData)
    {
        for (auto& param : parameters)
        {
            if (patchData.count (param->getName()))
                param->setValue (patchData.at (param->getName()));
        }
        
        // Refresh cards
        for (int i = 0; i < parameters.size(); ++i)
            cards[i]->setValue (parameters[i]->getValue());
    }

    void ModuleBase::setPage (int newPage)
    {
        currentPage = juce::jlimit (0, juce::jmax (0, numPages - 1), newPage);
        updatePageVisibility();
        resized();
        repaint();
    }

    void ModuleBase::updatePageVisibility()
    {
        int startIndex = currentPage * 8;
        for (int i = 0; i < (int)cards.size(); ++i)
            cards[i]->setVisible (i >= startIndex && i < startIndex + 8);
            
        prevButton.setEnabled (currentPage > 0);
        nextButton.setEnabled (currentPage < numPages - 1);
    }

    void ModuleBase::timerCallback()
    {
        // Update MIDI indicator
        midiIndicator.setColour (juce::Label::textColourId, midiActive ? accentColor : juce::Colours::transparentBlack);

        // Update Patch Name
        patchNameDisplay.setText (NeonRegistry::getCurrentPatchName(), juce::dontSendNotification);

        // Always trigger a repaint for visualization
        repaint();

        if (lastAdjustedIndex >= 0 && lastAdjustedIndex < (int)parameters.size())
        {
            auto& p = *parameters[lastAdjustedIndex];
            float val = p.getValue();
            
            juce::String valStr;
            if (p.getIsBoolean())
                valStr = p.getBinaryLabel (val > 0.5f);
            else
                valStr = juce::String (val, 1);
            
            // Large center display is now blank/reserved. 
            // We only show Parameter info in the small detail displays.
            unitDisplay.setText ({}, juce::dontSendNotification);
            activeParamNameDisplay.setText (p.getName().toUpperCase(), juce::dontSendNotification);
            activeParamValueDisplay.setText (valStr, juce::dontSendNotification);
        }
        else
        {
            unitDisplay.setText ({}, juce::dontSendNotification);
            activeParamNameDisplay.setText ({}, juce::dontSendNotification);
            activeParamValueDisplay.setText ({}, juce::dontSendNotification);
        }
    }

    void ModuleBase::paint (juce::Graphics& g)
    {
        g.fillAll (Colors::background);

        auto bounds = getLocalBounds();
        float unitH = (float)bounds.getHeight() / 10.0f;
        auto displayArea = bounds.removeFromTop ((int)(unitH * 7));

        // Call sub-module visualization
        paintVisualization (g, displayArea.reduced(2));

        // Voice Indicator Dots (Underneath MIDI indicator in top right)
        auto topDecor = displayArea.reduced (30, 25);
        auto midiArea = topDecor.removeFromTop (30);
        auto dotArea = topDecor.removeFromTop (10); // Area just below MIDI text
        dotArea = dotArea.removeFromRight (150); // Keep it aligned to the right like the MIDI text
        
        float dotSize = 4.0f;
        float dotSpacing = 8.0f;
        int maxDots = 16;
        
        for (int i = 0; i < maxDots; ++i)
        {
            float x = dotArea.getRight() - (i + 1) * dotSpacing;
            float y = dotArea.getCentreY();
            
            if (i < voiceCountCount)
                g.setColour (accentColor.withAlpha (0.9f));
            else
                g.setColour (accentColor.withAlpha (0.1f));
                
            g.fillEllipse (x - dotSize/2.0f, y - dotSize/2.0f, dotSize, dotSize);
        }

        // Page Indicators
        if (numPages > 1)
        {
            auto footer = getLocalBounds().removeFromBottom ((int)unitH);
            auto center = footer.getCentre().toFloat();
            float circleSize = 15.0f;
            float spacing = 45.0f;

            for (int i = 0; i < numPages; ++i)
            {
                float xOffset = (i - (numPages - 1) / 2.0f) * spacing;
                g.setColour (currentPage == i ? accentColor : juce::Colours::grey);
                g.fillEllipse (center.x + xOffset - (circleSize / 2.0f), center.y - (circleSize / 2.0f), circleSize, circleSize);
            }
        }
    }

    void ModuleBase::mouseDown (const juce::MouseEvent& e)
    {
        auto bounds = getLocalBounds();
        float unitH = (float)bounds.getHeight() / 10.0f;
        auto displayArea = bounds.removeFromTop ((int)(unitH * 7));

        if (displayArea.contains (e.getPosition()))
        {
            handleVisualizationInteraction (e, false);
            return;
        }

        if (numPages <= 1) return;

        auto footer = getLocalBounds().removeFromBottom ((int)unitH);
        auto center = footer.getCentre().toFloat();
        float hitSize = 30.0f;
        float spacing = 45.0f;

        for (int i = 0; i < numPages; ++i)
        {
            float xOffset = (i - (numPages - 1) / 2.0f) * spacing;
            juce::Rectangle<float> hitArea (center.x + xOffset - (hitSize / 2.0f), center.y - (hitSize / 2.0f), hitSize, hitSize);
            if (hitArea.contains (e.position))
            {
                setPage (i);
                return;
            }
        }
    }

    void ModuleBase::mouseDrag (const juce::MouseEvent& e)
    {
        auto bounds = getLocalBounds();
        float unitH = bounds.getHeight() / 8.0f;
        auto displayArea = bounds.removeFromTop (unitH * 5);

        if (displayArea.contains (e.getMouseDownPosition().toInt()))
        {
            handleVisualizationInteraction (e, true);
        }
    }

    void ModuleBase::mouseUp (const juce::MouseEvent& e)
    {
    }

    void ModuleBase::resized()
    {
        auto bounds = getLocalBounds();
        
        // According to user request:
        // Total area for module content is 10/12 of the total height.
        // Within that 10/12:
        // Display = 7/10 (which is 7/12 total)
        // Parameters = 2/10 (which is 2/12 total)
        // Navigation = 1/10 (which is 1/12 total)
        
        float unitH = (float)bounds.getHeight() / 10.0f;

        // 1. Display Area (7 units)
        auto displayArea = bounds.removeFromTop ((int)(unitH * 7));
        unitDisplay.setBounds (displayArea.reduced (10));
        
        auto detailArea = displayArea.reduced (30, 20);
        activeParamNameDisplay.setBounds (detailArea.removeFromLeft (400).withHeight (40).withY (displayArea.getBottom() - 60));
        activeParamValueDisplay.setBounds (detailArea.removeFromRight (400).withHeight (40).withY (displayArea.getBottom() - 60));
        
        auto topDecor = displayArea.reduced (30, 25);
        auto headerArea = topDecor.removeFromTop (40);
        moduleNameDisplay.setBounds (headerArea.removeFromLeft (headerArea.getWidth() / 3));
        midiIndicator.setBounds (headerArea.removeFromRight (headerArea.getWidth() / 2)); // Half of remaining is 1/3 total
        patchNameDisplay.setBounds (headerArea); // Remaining 1/3 center area

        // 2. Parameter Area (2 units)
        auto paramArea = bounds.removeFromTop ((int)(unitH * 2));
        int cols = 4;
        int rows = 2;
        int w = paramArea.getWidth() / cols;
        int h = paramArea.getHeight() / rows;

        int startIndex = currentPage * 8;
        for (int i = 0; i < 8; ++i)
        {
            int globalIndex = startIndex + i;
            if (globalIndex < (int)cards.size())
            {
                int row = i / cols;
                int col = i % cols;
                cards[globalIndex]->setBounds (paramArea.getX() + col * w, paramArea.getY() + row * h, w, h);
            }
        }

        // 3. Paging Area (1 unit)
        auto footer = bounds.reduced (20);
        float btnWidth = 100.0f;
        float indicatorSpread = 250.0f;
        prevButton.setBounds (footer.getCentreX() - indicatorSpread - btnWidth, footer.getCentreY() - 30, btnWidth, 60);
        nextButton.setBounds (footer.getCentreX() + indicatorSpread, footer.getCentreY() - 30, btnWidth, 60);
    }
}
