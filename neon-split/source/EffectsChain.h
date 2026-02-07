#pragma once

#include <JuceHeader.h>
#include "HighPassFilter.h"
#include "SyncDelay.h"
#include "Reverb.h"
#include "Chorus.h"

/**
 * Effects Chain wrapper for convenience
 * Manages the full effects signal path for each engine
 */
class EffectsChain
{
public:
    //==============================================================================
    EffectsChain() = default;
    ~EffectsChain() = default;
    
    //==============================================================================
    void prepare(double sampleRate, int samplesPerBlock);
    void reset();
    
    //==============================================================================
    HighPassFilter& getHPF() { return hpf; }
    Chorus& getChorus() { return chorus; }
    SyncDelay& getDelay() { return delay; }
    NeonReverb& getReverb() { return reverb; }

private:
    //==============================================================================
    HighPassFilter hpf;
    Chorus chorus;
    SyncDelay delay;
    NeonReverb reverb;
    
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EffectsChain)
};
