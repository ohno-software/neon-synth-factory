#pragma once

#include <JuceHeader.h>

class NeonQwenSynthSound : public juce::SynthesiserSound
{
public:
    NeonQwenSynthSound();
    bool appliesToNote (int midiNoteNumber) override;
    bool appliesToChannel (int midiChannel) override;
};
