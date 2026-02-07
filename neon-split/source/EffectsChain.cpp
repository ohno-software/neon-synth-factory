#include "EffectsChain.h"

//==============================================================================
void EffectsChain::prepare(double sampleRate, int samplesPerBlock)
{
    hpf.prepare(sampleRate, samplesPerBlock);
    chorus.prepare(sampleRate, samplesPerBlock);
    delay.prepare(sampleRate, samplesPerBlock);
    reverb.prepare(sampleRate, samplesPerBlock);
}

void EffectsChain::reset()
{
    hpf.reset();
    chorus.reset();
    delay.reset();
    reverb.reset();
}
