#include "ArpPresets.h"

//==============================================================================
juce::StringArray ArpPresets::getWaveformNames()
{
    return {"FM", "Analog Pulse", "Analog Saw", "Analog Sine"};
}

juce::StringArray ArpPresets::getPatternNames()
{
    return {"Up", "Down", "Up/Down", "Synthwave Gate", "Random"};
}
