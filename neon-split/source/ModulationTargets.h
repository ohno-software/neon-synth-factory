#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <vector>

namespace neon
{
    /**
     * NeonSplit Modulation Targets
     * Defines all modulation destinations for the Split synth.
     */
    enum class SplitModTarget
    {
        None = 0,
        BassLPF,
        BassVolume,
        PadChorusMix,
        PadVolume,
        ArpFilterCutoff,
        ArpVolume,
        MasterVolume,
        Count
    };

    enum class SplitCtrlSource
    {
        None = 0,
        PitchBend,
        ModWheel,
        Aftertouch,
        Velocity,
        Count
    };

    static inline std::vector<juce::String> getNeonSplitModTargetNames()
    {
        return {
            "None",
            "Bass LPF",
            "Bass Volume",
            "Pad Chorus Mix",
            "Pad Volume",
            "Arp Filter Cutoff",
            "Arp Volume",
            "Master Volume"
        };
    }

    static inline std::vector<juce::String> getNeonSplitCtrlSourceNames()
    {
        return { "None", "Pitch Bend", "Mod Wheel", "Aftertouch", "Velocity" };
    }

} // namespace neon
