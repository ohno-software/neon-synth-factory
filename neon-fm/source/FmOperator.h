#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_dsp/juce_dsp.h>
#include <cmath>

namespace neon
{
    /**
     * FmOperator
     * A single FM operator with an analog-style oscillator.
     * Can produce sine, saw, square, and triangle waveforms.
     * Supports phase modulation input for FM synthesis.
     */
    class FmOperator
    {
    public:
        enum class Waveform { Sine = 0, Triangle, Saw, Square, Count };

        FmOperator() = default;

        void prepare (double sr)
        {
            sampleRate = sr;
            phase = 0.0;
            envelope.setSampleRate (sr);
        }

        void reset()
        {
            phase = 0.0;
            envelope.reset();
        }

        void noteOn()
        {
            if (keySync)
                phase = 0.0;
            envelope.noteOn();
        }

        void noteOff()
        {
            envelope.noteOff();
        }

        void setEnvelopeParams (const juce::ADSR::Parameters& params)
        {
            envelope.setParameters (params);
        }

        /**
         * Process a single sample.
         * @param baseFreq     The fundamental frequency (from MIDI note + pitch bend etc.)
         * @param phaseMod     Phase modulation input from modulating operators (in radians)
         * @return             The operator output sample (with level and envelope applied)
         */
        float processSample (float baseFreq, float phaseMod = 0.0f)
        {
            // Apply ratio and detune to get operator frequency
            float freq = baseFreq * ratio + detuneHz;

            // Increment phase  
            double phaseInc = freq / sampleRate;
            phase += phaseInc;
            if (phase >= 1.0) phase -= 1.0;
            if (phase < 0.0) phase += 1.0;

            // Compute output with phase modulation
            float modPhase = (float)phase + phaseMod / juce::MathConstants<float>::twoPi;
            
            // Wrap modulated phase to [0, 1)
            modPhase -= std::floor (modPhase);

            float sample = generateWaveform (modPhase);

            // Apply feedback (self-modulation)
            if (feedback > 0.0f)
            {
                float fbSample = generateWaveform (modPhase + lastOutput * feedback / juce::MathConstants<float>::twoPi);
                sample = fbSample;
            }

            lastOutput = sample;

            // Apply envelope and level
            float envValue = envelope.getNextSample();
            return sample * level * envValue;
        }

        bool isActive() const { return envelope.isActive(); }

        // Parameters
        Waveform waveform = Waveform::Sine;
        float ratio = 1.0f;        // Frequency ratio (coarse tuning)
        float detuneHz = 0.0f;     // Fine detune in Hz
        float level = 1.0f;        // Output level (0-1)
        float feedback = 0.0f;     // Self-modulation amount (0-1)
        bool keySync = true;       // Reset phase on note-on
        float velocitySens = 0.5f; // Velocity sensitivity (0-1)

    private:
        float generateWaveform (float p) const
        {
            switch (waveform)
            {
                case Waveform::Sine:
                    return std::sin (p * juce::MathConstants<float>::twoPi);

                case Waveform::Triangle:
                {
                    if (p < 0.25f) return p * 4.0f;
                    if (p < 0.75f) return 2.0f - p * 4.0f;
                    return p * 4.0f - 4.0f;
                }

                case Waveform::Saw:
                    return 2.0f * p - 1.0f;

                case Waveform::Square:
                    return p < 0.5f ? 1.0f : -1.0f;

                default:
                    return std::sin (p * juce::MathConstants<float>::twoPi);
            }
        }

        double phase = 0.0;
        double sampleRate = 44100.0;
        float lastOutput = 0.0f;

        juce::ADSR envelope;
    };

} // namespace neon
