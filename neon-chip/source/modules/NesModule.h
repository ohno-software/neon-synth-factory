#pragma once

#include "ChipModuleBase.h"
#include <juce_dsp/juce_dsp.h>

namespace neon
{
    class NesModule : public ChipModuleBase
    {
    public:
        NesModule() 
            : ChipModuleBase("NES", juce::Colour(0xFF55FF55)) // Green color for NES
        {
            // NES Square Wave Parameters
            addParameter("Waveform", 0.0f, 3.0f, 0.0f, false, 1.0f); // 0=Square1, 1=Square2, 2=Triangle, 3=Noise
            addParameter("Volume", 0.0f, 1.0f, 0.7f);
            addParameter("Frequency", 0.0f, 1.0f, 0.5f);
            addParameter("Duty Cycle", 0.0f, 1.0f, 0.25f); // Square wave duty cycle
            
            // NES Filter Parameters
            addParameter("Filter Type", 0.0f, 2.0f, 0.0f, false, 1.0f); // 0=LP, 1=BP, 2=HP
            addParameter("Cutoff", 0.0f, 1.0f, 0.5f);
            
            // NES Envelope Parameters
            addParameter("Envelope", 0.0f, 1.0f, 0.0f); // Enable envelope
            addParameter("Decay", 0.0f, 1.0f, 0.3f);
            addParameter("Sustain", 0.0f, 1.0f, 0.7f);
            
            // NES Sweep Parameters
            addParameter("Sweep", 0.0f, 1.0f, 0.0f); // Enable sweep
            addParameter("Sweep Rate", 0.0f, 1.0f, 0.5f);
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
                
                // Generate NES-style waveform
                output += generateNesWaveform(sample);
                
                // Apply filter if needed
                output = applyNesFilter(output, sample);
                
                // Apply envelope if enabled
                if (parameters[6]->getValue() > 0.5f) // Envelope parameter
                {
                    output *= applyNesEnvelope(sample);
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
            envelopeValue = 0.0f;
            sweepPhase = 0.0f;
        }

        float generateNesWaveform(int sample)
        {
            const float waveformType = parameters[0]->getValue();
            
            // Generate oscillator output based on waveform type
            float output = 0.0f;
            
            switch ((int)waveformType)
            {
                case 0: // Square Wave 1
                case 1: // Square Wave 2
                    output = squareWave(phase1, parameters[3]->getValue()); // Duty cycle parameter
                    break;
                case 2: // Triangle Wave
                    output = triangleWave(phase1);
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

        float applyNesFilter(float input, int sample)
        {
            const float filterType = parameters[4]->getValue();
            const float cutoff = parameters[5]->getValue();
            
            // Simple low-pass filter implementation
            if (filterType < 0.5f) // LPF
            {
                static float filterState = 0.0f;
                const float fc = cutoff * 10000.0f; // Scale to frequency range
                const float alpha = fc / (fc + currentSampleRate);
                filterState = alpha * input + (1.0f - alpha) * filterState;
                return filterState;
            }
            
            return input;
        }

        float applyNesEnvelope(int sample)
        {
            // Simple envelope implementation
            const float decay = parameters[7]->getValue();
            const float sustain = parameters[8]->getValue();
            
            // Basic envelope that decreases over time
            envelopeValue -= 0.001f * decay;
            if (envelopeValue <= sustain)
            {
                envelopeValue = sustain;
            }
            
            return envelopeValue;
        }

        float squareWave(float phase, float dutyCycle)
        {
            // Square wave with adjustable duty cycle
            float t = phase - std::floor(phase);
            return (t < dutyCycle) ? 1.0f : -1.0f;
        }

        float triangleWave(float phase)
        {
            float t = phase - std::floor(phase + 0.5f);
            return 4.0f * std::abs(t) - 1.0f;
        }

        float noiseWave(int sample)
        {
            // Simple noise generator
            static int noiseSeed = 54321;
            noiseSeed = (noiseSeed * 1664525L + 1013904223L) & 0x7FFFFFFF;
            return ((float)(noiseSeed & 0xFFFF) / 32768.0f) - 1.0f;
        }

        float calculatePhaseIncrement(float frequency)
        {
            return frequency / currentSampleRate;
        }

        // NES oscillator state
        float phase1 = 0.0f;
        float envelopeValue = 0.0f;
        float sweepPhase = 0.0f;
    };
}