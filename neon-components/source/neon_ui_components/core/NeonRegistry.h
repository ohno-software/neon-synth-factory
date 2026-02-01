#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <vector>
#include <map>

#include "NeonColors.h"

namespace neon
{
    /**
     * NeonRegistry
     * A central place for the host application to register its modulation targets, 
     * control sources, filter types, and wavetables.
     */
    class NeonRegistry
    {
    public:
        // Theme
        static void setTheme(const Theme& theme) { get().theme = theme; }
        static Theme getTheme() { return get().theme; }

        // Modulation Targets
        static void setTargetNames(const std::vector<juce::String>& names) { get().targetNames = names; }
        static std::vector<juce::String> getTargetNames() { return get().targetNames; }

        // Control Sources
        static void setSourceNames(const std::vector<juce::String>& names) { get().sourceNames = names; }
        static std::vector<juce::String> getSourceNames() { return get().sourceNames; }

        // Filter Types
        static void setFilterTypes(const std::vector<juce::String>& types) { get().filterTypes = types; }
        static std::vector<juce::String> getFilterTypes() { return get().filterTypes; }

        // Wavetables
        static void setWaveformNames(const std::vector<juce::String>& names) { get().waveNames = names; }
        static std::vector<juce::String> getWaveformNames() { return get().waveNames; }

        static void setWaveformBuffer(int index, juce::AudioBuffer<float>* buffer) { get().waveBuffers[index] = buffer; }
        static juce::AudioBuffer<float>* getWaveformBuffer(int index) { 
            if (get().waveBuffers.count(index)) return get().waveBuffers[index];
            return nullptr;
        }

        static void setCurrentPatchName(const juce::String& name) { get().patchName = name; }
        static juce::String getCurrentPatchName() { return get().patchName; }

    private:
        static NeonRegistry& get() {
            static NeonRegistry instance;
            return instance;
        }

        NeonRegistry() {
            targetNames = { "None" };
            sourceNames = { "None" };
            filterTypes = { "LP", "HP", "BP" };
            waveNames = { "Sine" };

            // Initialize theme from default Colors
            theme.background = Colors::background;
            theme.cardBackground = Colors::cardBackground;
            theme.barBackground = Colors::barBackground;
            theme.textDim = Colors::textDim;
            theme.textBright = Colors::textBright;
            theme.oscillator = Colors::oscillator;
            theme.mutator = Colors::mutator;
            theme.filter = Colors::filter;
            theme.envelope = Colors::envelope;
            theme.amplifier = Colors::amplifier;
            theme.modulation = Colors::modulation;
            theme.effects = Colors::effects;
            theme.indicator = Colors::indicator;
        }

        Theme theme;
        std::vector<juce::String> targetNames;
        std::vector<juce::String> sourceNames;
        std::vector<juce::String> filterTypes;
        std::vector<juce::String> waveNames;
        std::map<int, juce::AudioBuffer<float>*> waveBuffers;
        juce::String patchName = "INIT PATCH";
    };

    /**
     * Helper functions to keep existing UI code working or provide shorter access.
     */
    static inline std::vector<juce::String> getModTargetNames() { return NeonRegistry::getTargetNames(); }
    static inline std::vector<juce::String> getCtrlSourceNames() { return NeonRegistry::getSourceNames(); }
}
