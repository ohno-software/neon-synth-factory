/*
  ==============================================================================

    NeonQwenSynthSound.cpp
    Created: 31 Jan 2026 3:52:57pm
    Author:  Adrian

  ==============================================================================
*/

#include "NeonQwenSynthSound.h"

//==============================================================================
NeonQwenSynthSound::NeonQwenSynthSound()
{
}

bool NeonQwenSynthSound::appliesToNote (int midiNoteNumber)
{
    return true;
}

bool NeonQwenSynthSound::appliesToChannel (int midiChannel)
{
    return true;
}