#pragma once

#include <JuceHeader.h>
#include "VoiceBase.h"

/**
 * Arp Preset/Waveform definitions
 * The arpeggiator uses its own waveform selection (not presets)
 * Available: FM, Analog Pulse, Analog Saw, Analog Sine
 */
class ArpPresets
{
public:
    static juce::StringArray getWaveformNames();
    static juce::StringArray getPatternNames();
};
