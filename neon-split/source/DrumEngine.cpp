#include "DrumEngine.h"

DrumEngine::DrumEngine()
{
}

void DrumEngine::prepare(double sr, int samplesPerBlock)
{
    sampleRate = sr;
    
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sr;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = 2;

    // Prepare hihat filter for mono (1 channel)
    juce::dsp::ProcessSpec monoSpec = spec;
    monoSpec.numChannels = 1;
    hihatFilter.prepare(monoSpec);
    
    setHiHatTone(hihatCutoff);

    snareReverb.prepare(sr, samplesPerBlock);
}

void DrumEngine::reset()
{
    kick.active = false;
    snare.active = false;
    hihat.active = false;
    hihatFilter.reset();
    snareReverb.reset();
}

void DrumEngine::triggerKick(float velocity)
{
    kick.active = true;
    kick.ampEnv = velocity;
    kick.pitchEnv = 1.0f;
    kick.phase = 0.0f;
}

void DrumEngine::triggerSnare(float velocity)
{
    snare.active = true;
    snare.ampEnv = velocity;
}

void DrumEngine::triggerHiHat(float velocity)
{
    hihat.active = true;
    hihat.ampEnv = velocity;
}

void DrumEngine::setHiHatTone(float cutoffHz)
{
    hihatCutoff = juce::jlimit(500.0f, 15000.0f, cutoffHz);
    // Q = 0.7071f for a flat response (no resonance bump)
    auto coefficients = juce::dsp::IIR::Coefficients<float>::makeHighPass(sampleRate, hihatCutoff, 0.7071f);
    
    hihatFilter.get<0>().coefficients = coefficients;
    hihatFilter.get<1>().coefficients = coefficients;
}

void DrumEngine::setSnareReverbMix(float mix)
{
    snareReverbMix = mix;
}

void DrumEngine::processBlock(juce::AudioBuffer<float>& buffer)
{
    if (!isEnabled) return;

    auto* left = buffer.getWritePointer(0);
    auto* right = buffer.getNumChannels() > 1 ? buffer.getWritePointer(1) : nullptr;

    juce::AudioBuffer<float> snareBuffer(buffer.getNumChannels(), buffer.getNumSamples());
    snareBuffer.clear();
    auto* snareLeft = snareBuffer.getWritePointer(0);
    auto* snareRight = snareBuffer.getNumChannels() > 1 ? snareBuffer.getWritePointer(1) : nullptr;

    // Hi-hat buffer is now mono
    juce::AudioBuffer<float> hihatBuffer(1, buffer.getNumSamples());
    hihatBuffer.clear();
    auto* hihatMono = hihatBuffer.getWritePointer(0);

    for (int s = 0; s < buffer.getNumSamples(); ++s)
    {
        // Kick
        float k = renderKick();
        left[s] += k;
        if (right) right[s] += k;

        // Snare (to separate buffer for reverb)
        float sn = renderSnare();
        snareLeft[s] = sn;
        if (snareRight) snareRight[s] = sn;

        // Hi-hat (to separate buffer for filtering)
        hihatMono[s] = renderHiHat();

        updateEnvelopes();
    }

    // Process Hi-hat Filter (Mono)
    juce::dsp::AudioBlock<float> hhBlock(hihatBuffer);
    juce::dsp::ProcessContextReplacing<float> hhContext(hhBlock);
    hihatFilter.process(hhContext);

    // Process Snare Reverb
    snareReverb.setMix(snareReverbMix);
    snareReverb.processBlock(snareBuffer);

    // Mix back
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
    {
        buffer.addFrom(ch, 0, snareBuffer, ch, 0, buffer.getNumSamples());
        // Mix mono hi-hat into both channels
        buffer.addFrom(ch, 0, hihatBuffer, 0, 0, buffer.getNumSamples());
    }
}

void DrumEngine::updateEnvelopes()
{
    float invSr = 1.0f / static_cast<float>(sampleRate);

    if (kick.active)
    {
        kick.ampEnv -= invSr * 5.0f; // ~200ms decay
        kick.pitchEnv -= invSr * 40.0f; // Very fast pitch drop
        if (kick.ampEnv <= 0.0f) { kick.ampEnv = 0.0f; kick.active = false; }
        if (kick.pitchEnv <= 0.0f) kick.pitchEnv = 0.0f;
    }

    if (snare.active)
    {
        snare.ampEnv -= invSr * 7.0f; // ~140ms decay
        if (snare.ampEnv <= 0.0f) { snare.ampEnv = 0.0f; snare.active = false; }
    }

    if (hihat.active)
    {
        hihat.ampEnv -= invSr * 20.0f; // ~50ms decay
        if (hihat.ampEnv <= 0.0f) { hihat.ampEnv = 0.0f; hihat.active = false; }
    }
}

float DrumEngine::renderKick()
{
    if (!kick.active) return 0.0f;

    // Pitch sweep: 150Hz down to 50Hz
    float freq = 50.0f + (kick.pitchEnv * 100.0f);
    float phaseInc = freq / static_cast<float>(sampleRate);
    
    float out = std::sin(kick.phase * juce::MathConstants<float>::twoPi);
    kick.phase = std::fmod(kick.phase + phaseInc, 1.0f);

    return out * kick.ampEnv;
}

float DrumEngine::renderSnare()
{
    if (!snare.active) return 0.0f;

    // Noise + Sine pop
    float noise = snare.random.nextFloat() * 2.0f - 1.0f;
    
    // Simple sine pop at 180Hz (static phase for simplicity)
    static float snarePhase = 0.0f;
    float freq = 180.0f;
    float phaseInc = freq / static_cast<float>(sampleRate);
    float pop = std::sin(snarePhase * juce::MathConstants<float>::twoPi);
    snarePhase = std::fmod(snarePhase + phaseInc, 1.0f);

    return (noise * 0.7f + pop * 0.3f) * snare.ampEnv;
}

float DrumEngine::renderHiHat()
{
    if (!hihat.active) return 0.0f;

    // Just white noise, filter is applied in processBlock
    return (hihat.random.nextFloat() * 2.0f - 1.0f) * hihat.ampEnv;
}
