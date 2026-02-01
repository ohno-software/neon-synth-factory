#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "../widgets/NeonParameterCard.h"
#include "../core/NeonManagedParameter.h"
#include "../core/NeonColors.h"

namespace neon
{
    /**
     * ModuleBase
     * Base class for all high-level synth modules with the "Unit Display" layout.
     * 5/8 Display, 2/8 Parameters (2x4), 1/8 Paging
     */
    class ModuleBase : public juce::Component, public juce::Timer
    {
    public:
        ModuleBase (const juce::String& moduleName, const juce::Colour& accentColor);
        ~ModuleBase() override;

        void paint (juce::Graphics& g) override;
        void resized() override;
        void mouseDown (const juce::MouseEvent& e) override;
        void mouseDrag (const juce::MouseEvent& e) override;
        void mouseUp (const juce::MouseEvent& e) override;
        void timerCallback() override;

        void setPage (int newPage);
        void addParameter (const juce::String& name, float min, float max, float def, bool isBool = false, float interval = 0.0f, bool isMomentary = false, bool isLinear = false);
        void addChoiceParameter (const juce::String& name, const std::vector<juce::String>& choices, int defaultIndex);
        void updateChoiceParameter (const juce::String& name, const std::vector<juce::String>& choices);
        void addSpacer();
        
        void setMidiActive (bool active) { if (midiActive != active) { midiActive = active; repaint(); } }
        void setVoiceCount (int count) { if (voiceCountCount != count) { voiceCountCount = count; repaint(); } }

        // Load settings from a "Patch"
        void loadPatch (const std::map<juce::String, float>& patchData);

    protected:
        virtual void paintVisualization (juce::Graphics& g, juce::Rectangle<int> area) {}
        virtual void handleVisualizationInteraction (const juce::MouseEvent& e, bool isDrag) {}

        juce::String moduleName;
        juce::Colour accentColor;
        bool midiActive = false;
        int voiceCountCount = 0;
        
        std::vector<ManagedParameter*> parameters;
        std::vector<std::unique_ptr<ParameterCard>> cards;
        
        juce::Label unitDisplay;
        juce::Label midiIndicator;
        juce::Label patchNameDisplay;
        juce::Label activeParamNameDisplay;
        juce::Label activeParamValueDisplay;
        juce::Label moduleNameDisplay;
        
        juce::TextButton prevButton, nextButton;
        int currentPage = 0;
        int lastAdjustedIndex = 0;
        int numPages = 1;

    private:
        void updatePageVisibility();
        
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ModuleBase)
    };
}
