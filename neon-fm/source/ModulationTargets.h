#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <vector>

namespace neon
{
    /**
     * FmModTarget
     * Defines all possible modulation destinations in Neon FM.
     */
    enum class FmModTarget
    {
        None = 0,

        MasterPitch,

        Op1Level,
        Op2Level,
        Op3Level,
        Op4Level,

        Op1Ratio,
        Op2Ratio,
        Op3Ratio,
        Op4Ratio,

        Op1Feedback,
        Op2Feedback,
        Op3Feedback,
        Op4Feedback,

        FilterCutoff,
        FilterRes,

        Lfo1Amount1, Lfo1Amount2, Lfo1Amount3, Lfo1Amount4,
        Lfo2Amount1, Lfo2Amount2, Lfo2Amount3, Lfo2Amount4,

        Count
    };

    /**
     * CtrlSource (Neon FM)
     */
    enum class FmCtrlSource
    {
        None = 0,
        PitchBend,
        ModWheel,
        Aftertouch,
        Velocity,
        KeyTrack,
        Count
    };

    static inline std::vector<juce::String> getNeonFmModTargetNames()
    {
        return {
            "None",
            "Master Pitch",
            "Op1 Level", "Op2 Level", "Op3 Level", "Op4 Level",
            "Op1 Ratio", "Op2 Ratio", "Op3 Ratio", "Op4 Ratio",
            "Op1 Feedback", "Op2 Feedback", "Op3 Feedback", "Op4 Feedback",
            "Filter Cutoff", "Filter Res",
            "LFO 1 Amt 1", "LFO 1 Amt 2", "LFO 1 Amt 3", "LFO 1 Amt 4",
            "LFO 2 Amt 1", "LFO 2 Amt 2", "LFO 2 Amt 3", "LFO 2 Amt 4"
        };
    }

    static inline std::vector<juce::String> getNeonFmCtrlSourceNames()
    {
        return { "None", "Pitch Bend", "Mod Wheel", "Aftertouch", "Velocity", "Key Track" };
    }

    /**
     * getFmModTargetNames - returns the target names for use with choice parameters
     */
    static inline std::vector<juce::String> getFmModTargetNames()
    {
        return getNeonFmModTargetNames();
    }

} // namespace neon
