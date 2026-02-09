#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <vector>

namespace neon
{
    /**
     * ChipModTarget
     * Defines all possible modulation destinations in Neon Chip.
     */
    enum class ChipModTarget
    {
        None = 0,

        OscPitch,
        OscPulseWidth,
        OscVolume,

        FilterCutoff,
        FilterRes,

        Count
    };

    /**
     * ChipCtrlSource
     * Defines global control sources in Neon Chip.
     */
    enum class ChipCtrlSource
    {
        None = 0,
        PitchBend,
        ModWheel,
        Aftertouch,
        Velocity,
        KeyTrack,
        Count
    };

    static inline std::vector<juce::String> getNeonChipModTargetNames()
    {
        return {
            "None",
            "Osc Pitch", "Osc Pulse Width", "Osc Volume",
            "Filter Cutoff", "Filter Res"
        };
    }

    static inline std::vector<juce::String> getNeonChipCtrlSourceNames()
    {
        return { "None", "Pitch Bend", "Mod Wheel", "Aftertouch", "Velocity", "Key Track" };
    }

} // namespace neon
