#pragma once

#include "ChipModuleBase.h"
#include <juce_dsp/juce_dsp.h>

namespace neon
{
    class SidModule : public ChipModuleBase
    {
    public:
        SidModule() 
            : ChipModuleBase("SID", juce::Colour(0xFFFF5555)) // Red color for SID
        {
            // SID Oscillator Parameters
            addParameter("Waveform", 0.0f, 3.0f, 0.0f, false, 1.0f); // 0=SAW, 1=PULSE, 2=SINE, 3=TRIANGLE
            addParameter("Volume", 0.0f, 1.0f, 0.7f);
            addParameter("Detune", -100.0f, 100.0f, 0.0f);
            addParameter("Pulse Width", 0.0f, 1.0f, 0.5f);
            
            // SID Filter Parameters
            addParameter("Filter Type", 0.0f, 2.0f, 0.0f, false, 1.0f); // 0=LP, 1=BP, 2=HP
            addParameter("Cutoff", 0.0f, 1.0f, 0.5f);
            addParameter("Resonance", 0.0f, 1.0f, 0.0f);
            
            // SID Envelope Parameters
            addParameter("Attack", 0.0f, 1.0f, 0.1f);
            addParameter("Decay", 0.0f, 1.0f, 0.3f);
            addParameter("Sustain", 0.0f, 1.0f, 0.7f);
            addParameter("Release", 0.0f, 1.0f, 0.2f);
            
            // SID Noise Parameters
            addParameter("Noise", 0.0f, 1.0f, 0.0f);
        }

        void setSampleRate(double sampleRate) override
        {
            ChipModuleBase::setSampleRate(sampleRate);
            // Initialize SID oscillator state
            resetOscillators();
        }

        void prepareToPlay(int samplesPerBlock) override
        {
            ChipModuleBase::prepareToPlay(samplesPerBlock);
            resetOscillators();
        }

        void processBlock(juce::AudioBuffer<float>& buffer, int numChannels) override
        {
            const int numSamples = buffer.getNumSamples();
            
            // Process each sample
            for (int sample = 0; sample < numSamples; ++sample)
            {
                float output = 0.0f;
                
                // Generate SID-style waveform
                output += generateSidWaveform(sample);
                
                // Apply filter if needed
                output = applySidFilter(output, sample);
                
                // Apply envelope
                output *= applySidEnvelope(sample);
                
                // Apply noise if enabled
                if (parameters[8]->getValue() > 0.5f) // Noise parameter
                {
                    output += generateNoise(sample) * parameters[8]->getValue();
                }
                
                // Apply volume
                output *= parameters[1]->getValue(); // Volume parameter
                
                // Write to all channels
                for (int channel = 0; channel < numChannels; ++channel)
                {
                    buffer.addSample(channel, sample, output);
                }
            }
        }

    private:
        void resetOscillators()
        {
            phase1 = 0.0f;
            phase2 = 0.0f;
            phase3 = 0.0f;
            
            envelopeState = 0; // 0=attack, 1=decay, 2=sustain, 3=release
            envelopeValue = 0.0f;
        }

        float generateSidWaveform(int sample)
        {
            const float waveformType = parameters[0]->getValue();
            const float pulseWidth = parameters[3]->getValue();
            
            // Generate oscillator output based on waveform type
            float output = 0.0f;
            
            switch ((int)waveformType)
            {
                case 0: // SAWTOOTH
                    output = sawtoothWave(phase1);
                    break;
                case 1: // PULSE
                    output = pulseWave(phase1, pulseWidth);
                    break;
                case 2: // SINE
                    output = sineWave(phase1);
                    break;
                case 3: // TRIANGLE
                    output = triangleWave(phase1);
                    break;
            }
            
            // Update phase for next sample
            phase1 += calculatePhaseIncrement(440.0f); // Default to 440Hz
            
            return output;
        }

        float applySidFilter(float input, int sample)
        {
            const float filterType = parameters[4]->getValue();
            const float cutoff = parameters[5]->getValue();
            const float resonance = parameters[6]->getValue();
            
            // Simple low-pass filter implementation
            if (filterType < 0.5f) // LPF
            {
                // Very basic filter - in a real implementation this would be more sophisticated
                static float filterState = 0.0f;
                const float fc = cutoff * 20000.0f; // Scale to frequency range
                const float alpha = fc / (fc + currentSampleRate);
                filterState = alpha * input + (1.0f - alpha) * filterState;
                return filterState;
            }
            
            return input;
        }

        float applySidEnvelope(int sample)
        {
            // Simple envelope implementation
            const float attack = parameters[7]->getValue();
            const float decay = parameters[8]->getValue(); 
            const float sustain = parameters[9]->getValue();
            const float release = parameters[10]->getValue();
            
            switch (envelopeState)
            {
                case 0: // Attack
                    envelopeValue += 0.001f * attack;
                    if (envelopeValue >= 1.0f)
                    {
                        envelopeValue = 1.0f;
                        envelopeState = 1; // Decay
                    }
                    break;
                case 1: // Decay
                    envelopeValue -= 0.001f * decay;
                    if (envelopeValue <= sustain)
                    {
                        envelopeValue = sustain;
                        envelopeState = 2; // Sustain
                    }
                    break;
                case 2: // Sustain
                    // Hold at sustain level
                    break;
                case 3: // Release
                    envelopeValue -= 0.001f * release;
                    if (envelopeValue <= 0.0f)
                    {
                        envelopeValue = 0.0f;
                        envelopeState = 4; // Done
                    }
                    break;
            }
            
            return envelopeValue;
        }

        float generateNoise(int sample)
        {
            // Simple noise generator
            static int noiseSeed = 12345;
            noiseSeed = (noiseSeed * 1664525L + 1013904223L) & 0x7FFFFFFF;
            return ((float)(noiseSeed & 0xFFFF) / 32768.0f) - 1.0f;
        }

        // Waveform generators
        float sawtoothWave(float phase)
        {
            return 2.0f * (phase - std::floor(phase + 0.5f));
        }

        float pulseWave(float phase, float width)
        {
            return (phase < width) ? 1.0f : -1.0f;
        }

        float sineWave(float phase)
        {
            return std::sin(2.0f * juce::MathConstants<float>::pi * phase);
        }

        float triangleWave(float phase)
        {
            float t = phase - std::floor(phase + 0.5f);
            return 4.0f * std::abs(t) - 1.0f;
        }

        float calculatePhaseIncrement(float frequency)
        {
            return frequency / currentSampleRate;
        }

        // SID oscillator state
        float phase1 = 0.0f;
        float phase2 = 0.0f;
        float phase3 = 0.0f;
        
        // Envelope state
        int envelopeState = 0;
        float envelopeValue = 0.0f;
    };
}