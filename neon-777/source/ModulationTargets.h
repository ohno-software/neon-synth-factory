#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <vector>

namespace neon
{
    /**
     * ModTarget (Neon Jr Specific)
     * Defines all possible modulation destinations in Neon Jr.
     */
    enum class ModTarget
    {
        None = 0,
        
        Osc1Pitch,
        Osc1FM,
        Osc1Symmetry,
        Osc1Fold,
        Osc1Drive,
        Osc1BitRedux,
        Osc1Level,
        Osc1Pan,
        Osc1Detune,

        Osc2Pitch,
        Osc2FM,
        Osc2Symmetry,
        Osc2Fold,
        Osc2Drive,
        Osc2BitRedux,
        Osc2Level,
        Osc2Pan,
        Osc2Detune,
        
        SubLevel,
        SubPitch,
        
        FilterCutoff,
        FilterRes,

        Lfo1Amount1, Lfo1Amount2, Lfo1Amount3, Lfo1Amount4,
        Lfo2Amount1, Lfo2Amount2, Lfo2Amount3, Lfo2Amount4,
        
        Count
    };

    /**
     * CtrlSource (Neon Jr Specific)
     * Defines global control sources in Neon Jr.
     */
    enum class CtrlSource
    {
        None = 0,
        PitchBend,
        ModWheel,
        Aftertouch,
        Velocity,
        KeyTrack,
        Count
    };

    /**
     * getNeonJrModTargetNames
     * Provides the display names for the ModTarget enum.
     */
    static inline std::vector<juce::String> getNeon777ModTargetNames()
    {
        return {
            "None",
            "Osc1 Pitch", "Osc1 FM", "Osc1 Symmetry", "Osc1 Fold", "Osc1 Drive", "Osc1 BitRedux", "Osc1 Level", "Osc1 Pan", "Osc1 Detune",
            "Osc2 Pitch", "Osc2 FM", "Osc2 Symmetry", "Osc2 Fold", "Osc2 Drive", "Osc2 BitRedux", "Osc2 Level", "Osc2 Pan", "Osc2 Detune",
            "Sub Level", "Sub Pitch",
            "Filter Cutoff", "Filter Res",
            "LFO 1 Amt 1", "LFO 1 Amt 2", "LFO 1 Amt 3", "LFO 1 Amt 4",
            "LFO 2 Amt 1", "LFO 2 Amt 2", "LFO 2 Amt 3", "LFO 2 Amt 4"
        };
    }

    /**
     * getNeon777CtrlSourceNames
     * Provides the display names for the CtrlSource enum.
     */
    static inline std::vector<juce::String> getNeon777CtrlSourceNames()
    {
        return { "None", "Pitch Bend", "Mod Wheel", "Aftertouch", "Velocity", "Key Track" };
    }
}
