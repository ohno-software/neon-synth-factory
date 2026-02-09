#pragma once

#include "ChipModuleBase.h"
#include <juce_dsp/juce_dsp.h>

namespace neon
{
    class AtariModule : public ChipModuleBase
    {
    public:
        AtariModule() 
            : ChipModuleBase("ATARI", juce::Colour(0xFF5555FF)) // Blue color for Atari
        {
            // Atari Sound Parameters
            addParameter("Waveform", 0.0f, 3.0f, 0.0f, false, 1.0f); // 0=Square, 1=Triangle, 2=Sawtooth, 3=Noise
            addParameter("Volume", 0.0f, 1.0f, 0.7f);
            addParameter("Frequency", 0.0f, 1.0f, 0.5f);
            addParameter("Pulse Width", 0.0f, 1.0f, 0.5f); // For square wave
            
            // Atari Filter Parameters
            addParameter("Filter Type", 0.0f, 2.0f, 0.0f, false, 1.0f); // 0=LP, 1=BP, 2=HP
            addParameter("Cutoff", 0.0f, 1.0f, 0.5f);
            
            // Atari Envelope Parameters
            addParameter("Attack", 0.0f, 1.0f, 0.1f);
            addParameter("Decay", 0.0f, 1.0f, 0.3f);
            addParameter("Sustain", 0.0f, 1.0f, 0.7f);
            addParameter("Release", 0.0f, 1.0f, 0.2f);
            
            // Atari Noise Parameters
            addParameter("Noise", 0.0f, 1.0f, 0.0f);
        }

        void setSampleRate(double sampleRate) override
        {
            ChipModuleBase::setSampleRate(sampleRate);
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
                
                // Generate Atari-style waveform
                output += generateAtariWaveform(sample);
                
                // Apply filter if needed
                output = applyAtariFilter(output, sample);
                
                // Apply envelope
                output *= applyAtariEnvelope(sample);
                
                // Apply noise if enabled
                if (parameters[10]->getValue() > 0.5f) // Noise parameter
                {
                    output += generateNoise(sample) * parameters[10]->getValue();
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
            envelopeState = 0; // 0=attack, 1=decay, 2=sustain, 3=release
            envelopeValue = 0.0f;
        }

        float generateAtariWaveform(int sample)
        {
            const float waveformType = parameters[0]->getValue();
            
            // Generate oscillator output based on waveform type
            float output = 0.0f;
            
            switch ((int)waveformType)
            {
                case 0: // Square Wave
                    output = squareWave(phase1, parameters[3]->getValue()); // Pulse width parameter
                    break;
                case 1: // Triangle Wave
                    output = triangleWave(phase1);
                    break;
                case 2: // Sawtooth Wave
                    output = sawtoothWave(phase1);
                    break;
                case 3: // Noise
                    output = noiseWave(sample);
                    break;
            }
            
            // Update phase for next sample
            float frequency = 440.0f * (1.0f + parameters[2]->getValue() * 10.0f); // Frequency parameter
            phase1 += calculatePhaseIncrement(frequency);
            
            return output;
        }

        float applyAtariFilter(float input, int sample)
        {
            const float filterType = parameters[4]->getValue();
            const float cutoff = parameters[5]->getValue();
            
            // Simple low-pass filter implementation
            if (filterType < 0.5f) // LPF
            {
                static float filterState = 0.0f;
                const float fc = cutoff * 15000.0f; // Scale to frequency range
                const float alpha = fc / (fc + currentSampleRate);
                filterState = alpha * input + (1.0f - alpha) * filterState;
                return filterState;
            }
            
            return input;
        }

        float applyAtariEnvelope(int sample)
        {
            // Simple envelope implementation
            const float attack = parameters[6]->getValue();
            const float decay = parameters[7]->getValue(); 
            const float sustain = parameters[8]->getValue();
            const float release = parameters[9]->getValue();
            
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

        float squareWave(float phase, float width)
        {
            // Square wave with adjustable duty cycle
            float t = phase - std::floor(phase);
            return (t < width) ? 1.0f : -1.0f;
        }

        float triangleWave(float phase)
        {
            float t = phase - std::floor(phase + 0.5f);
            return 4.0f * std::abs(t) - 1.0f;
        }

        float sawtoothWave(float phase)
        {
            return 2.0f * (phase - std::floor(phase + 0.5f));
        }

        float noiseWave(int sample)
        {
            // Simple noise generator
            static int noiseSeed = 98765;
            noiseSeed = (noiseSeed * 1664525L + 1013904223L) & 0x7FFFFFFF;
            return ((float)(noiseSeed & 0xFFFF) / 32768.0f) - 1.0f;
        }

        float generateNoise(int sample)
        {
            // Simple noise generator
            static int noiseSeed = 98765;
            noiseSeed = (noiseSeed * 1664525L + 1013904223L) & 0x7FFFFFFF;
            return ((float)(noiseSeed & 0xFFFF) / 32768.0f) - 1.0f;
        }

        float calculatePhaseIncrement(float frequency)
        {
            return frequency / currentSampleRate;
        }

        // Atari oscillator state
        float phase1 = 0.0f;
        
        // Envelope state
        int envelopeState = 0;
        float envelopeValue = 0.0f;
    };
}