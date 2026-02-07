#pragma once

#include <JuceHeader.h>
#include "Reverb.h"

/**
 * DrumEngine - Synthesized analog drum machine
 */
class DrumEngine
{
public:
    DrumEngine();
    ~DrumEngine() = default;

    void prepare(double sampleRate, int samplesPerBlock);
    void reset();

    void triggerKick(float velocity);
    void triggerSnare(float velocity);
    void triggerHiHat(float velocity);

    void processBlock(juce::AudioBuffer<float>& buffer);

    void setEnabled(bool enabled) { isEnabled = enabled; }
    bool getEnabled() const { return isEnabled; }

    void setHiHatTone(float cutoffHz);
    void setSnareReverbMix(float mix);

private:
    double sampleRate = 44100.0;
    bool isEnabled = false;

    // Kick state
    struct KickState {
        float phase = 0.0f;
        float ampEnv = 0.0f;
        float pitchEnv = 0.0f;
        bool active = false;
    } kick;

    // Snare state
    struct SnareState {
        float ampEnv = 0.0f;
        bool active = false;
        juce::Random random;
    } snare;

    // Hi-hat state
    struct HiHatState {
        float ampEnv = 0.0f;
        bool active = false;
        juce::Random random;
    } hihat;

    // Hi-hat Filter (24dB/oct HPF)
    juce::dsp::ProcessorChain<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Filter<float>> hihatFilter;
    float hihatCutoff = 5000.0f;

    // Snare Reverb
    NeonReverb snareReverb;
    float snareReverbMix = 0.3f;

    void updateEnvelopes();
    float renderKick();
    float renderSnare();
    float renderHiHat();
};
