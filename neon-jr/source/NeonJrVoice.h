#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_dsp/juce_dsp.h>
#include <array>

namespace neon
{
    class SignalPath; // Forward declaration

    /**
     * NeonJrVoice
     * JUCE-native synthesiser voice for Neon Jr
     */
    class NeonJrVoice : public juce::SynthesiserVoice
    {
    public:
        NeonJrVoice (SignalPath& sp);
        
        bool canPlaySound (juce::SynthesiserSound*) override { return true; }
        
        void startNote (int midiNoteNumber, float velocity, juce::SynthesiserSound*, int currentPitchWheelPosition) override;
        void stopNote (float velocity, bool allowTailOff) override;
        void pitchWheelMoved (int newPitchWheelValue) override {}
        void controllerMoved (int controllerNumber, int newControllerValue) override {}
        
        void renderNextBlock (juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples) override;
        
        void prepareToPlay (double sampleRate, int samplesPerBlock);
        
        // Modulation cache (updated less frequently for CPU efficiency)
        void updateModulationCache();
        
    private:
        SignalPath& signalPath;
        
        int currentMidiNote = -1;
        float currentVelocity = 0.0f;
        double noteOnTime = 0.0;
        double sampleRate = 44100.0;
        
        // Per-voice state
        struct OscState
        {
            float phases[8] = {0.0f};
            float currentFrequency = 440.0f;
        };
        
        OscState osc1, osc2;
        
        struct LfoState
        {
            float phase = 0.0f;
            float lastOutput = 0.0f;
            bool sampleHoldTriggered = false;
            double noteOnTime = -1.0;
            
            void reset(float startPhase, double onTime) { 
                phase = startPhase; 
                lastOutput = 0.0f; 
                sampleHoldTriggered = false; 
                noteOnTime = onTime;
            }
        };
        
        std::array<LfoState, 3> lfos;
        
        juce::ADSR ampEnv, filterEnv, pitchEnv, modEnv;
        juce::dsp::StateVariableTPTFilter<float> filter1;
        juce::dsp::StateVariableTPTFilter<float> filter2;
        
        // Modulation cache (decimated updates)
        float u1Freqs[8], u1GainsL[8], u1GainsR[8];
        float u2Freqs[8], u2GainsL[8], u2GainsR[8];
        float mOsc1Pitch = 0, mOsc2Pitch = 0;
        float mOsc1Sym = 0, mOsc1Fold = 0, mOsc1Drive = 0, mOsc1Bit = 0, mOsc1Lvl = 0, mOsc1Pan = 0;
        float mOsc2Sym = 0, mOsc2Fold = 0, mOsc2Drive = 0, mOsc2Bit = 0, mOsc2Lvl = 0, mOsc2Pan = 0;
        float mOsc1Det = 0, mOsc2Det = 0;
        float mFiltCut = 0, mFiltRes = 0;
        float mLfoAmts[3][4] = {{0.0f}};
        int decimationCounter = 0;
        
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NeonJrVoice)
    };
    
    /**
     * Simple sound class - NeonJrVoice can play any sound
     */
    class NeonJrSound : public juce::SynthesiserSound
    {
    public:
        bool appliesToNote (int) override { return true; }
        bool appliesToChannel (int) override { return true; }
    };
}
