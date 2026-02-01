#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <map>
#include "NeonManagedParameter.h"

namespace neon
{
    /**
     * ParameterRegistry
     * A central singleton-style registry for all parameters in the synth.
     * Allows debug views and the (future) audio engine to access any parameter by name.
     */
    class ParameterRegistry
    {
    public:
        static ParameterRegistry& getInstance();

        // Registry becomes the owner of the parameters to ensure they outlive the UI
        ManagedParameter* getOrCreateParameter (const juce::String& modulePath, const juce::String& name, float min, float max, float def, bool isBool = false, float interval = 0.0f, bool isMomentary = false, bool isLinear = false)
        {
            auto fullPath = modulePath + "/" + name;
            if (parameters.count (fullPath))
                return parameters[fullPath].get();
            
            auto param = std::make_unique<ManagedParameter> (name, min, max, def, isBool, isLinear);
            param->setInterval (interval);
            param->setIsMomentary (isMomentary);
            auto* ptr = param.get();
            parameters[fullPath] = std::move (param);
            return ptr;
        }

        ManagedParameter* getOrCreateChoiceParameter (const juce::String& modulePath, const juce::String& name, const std::vector<juce::String>& choices, int defaultIndex)
        {
            auto fullPath = modulePath + "/" + name;
            
            // If it exists, update it if the name contains "Target" to ensure target lists stay sync'd
            if (parameters.count (fullPath))
            {
                auto* existing = parameters[fullPath].get();
                if (name.contains ("Target"))
                    existing->setChoices (choices);
                return existing;
            }
            
            auto param = std::make_unique<ManagedParameter> (name, 0.0f, (float)(choices.size() - 1), (float)defaultIndex, false);
            param->setChoices (choices);
            auto* ptr = param.get();
            parameters[fullPath] = std::move (param);
            return ptr;
        }

        ManagedParameter* getParameter (const juce::String& fullPath)
        {
            if (parameters.count (fullPath))
                return parameters[fullPath].get();
            return nullptr;
        }

        const std::map<juce::String, std::unique_ptr<ManagedParameter>>& getParameters() const
        {
            return parameters;
        }

    private:
        ParameterRegistry() = default;
        std::map<juce::String, std::unique_ptr<ManagedParameter>> parameters;
        
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ParameterRegistry)
    };
}
