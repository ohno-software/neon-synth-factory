#pragma once

#include <juce_dsp/juce_dsp.h>
#include <cmath>
#include <cstdint>

namespace neon
{
    /**
     * SidOscillator
     * Emulates the Commodore 64 SID 6581/8580 oscillator.
     * Features: Tri, Saw, Pulse (PWM), Noise (23-bit LFSR).
     * Includes Sync and Ring Modulation hooks.
     */
    class SidOscillator
    {
    public:
        enum class Waveform { Triangle, Saw, Pulse, Noise };

        void setSampleRate (double sr) { sampleRate = std::max(1.0, sr); }

        void setWaveform (int idx) 
        { 
            waveform = static_cast<Waveform>(juce::jlimit(0, 3, idx)); 
        }

        void setPulseWidth (float pw) { pulseWidth = std::clamp(pw, 0.01f, 0.99f); }
        
        void setFrequency (float freqHz) { frequency = std::max(0.1f, freqHz); }

        void noteOn (float freqHz, bool resetPhase)
        {
            frequency = std::max(0.1f, freqHz);
            if (resetPhase)
            {
                phase = 0.0f;
                lfsr = 0x7FFFFF; 
            }
        }

        /**
         * Process the oscillator sample.
         * @param syncIn The phase of the 'syncing' oscillator (0.0 to 1.0)
         * @param ringIn The output of the 'modulating' oscillator (-1.0 to 1.0)
         */
        float process (float syncInPhase = 0.0f, bool syncEnabled = false, float ringInSample = 0.0f, bool ringEnabled = false)
        {
            // Sync: If the master oscillator wraps around, we reset our phase
            // (Note: This is a simplified software-sync compared to real SID hardware's bit-level sync)
            if (syncEnabled && syncInPhase < lastSyncInPhase)
            {
                phase = 0.0f;
            }
            lastSyncInPhase = syncInPhase;

            float rawWave = 0.0f;

            switch (waveform)
            {
                case Waveform::Triangle:
                {
                    rawWave = (phase < 0.5f) ? (phase * 4.0f - 1.0f) : (3.0f - phase * 4.0f);
                    
                    // Ring Mod: In the SID, Triangle is special. It's XORed with the MSB of the master osc.
                    // Effectively it flips the triangle if the master is in the second half of its cycle.
                    if (ringEnabled && ringInSample < 0.0f)
                        rawWave = -rawWave;
                    break;
                }
                case Waveform::Saw:
                {
                    rawWave = phase * 2.0f - 1.0f;
                    break;
                }
                case Waveform::Pulse:
                {
                    rawWave = (phase < pulseWidth) ? 1.0f : -1.0f;
                    break;
                }
                case Waveform::Noise:
                {
                    rawWave = (lfsr & 0x400000) ? 1.0f : -1.0f;
                    break;
                }
            }

            advancePhase();
            return rawWave;
        }

        float getPhase() const { return phase; }

        void reset()
        {
            phase = 0.0f;
            lfsr = 0x7FFFFF;
            lastSyncInPhase = 0.0f;
        }

    private:
        void advancePhase()
        {
            float inc = (float)(frequency / sampleRate);
            phase += inc;
            
            if (phase >= 1.0f)
            {
                phase -= 1.0f;
                stepNoise();
            }
        }

        void stepNoise()
        {
            // SID noise LFSR step
            uint32_t bit = ((lfsr >> 22) ^ (lfsr >> 17)) & 1;
            lfsr = ((lfsr << 1) | bit) & 0x7FFFFF;
        }

        double sampleRate = 44100.0;
        float frequency = 440.0f;
        float phase = 0.0f;
        float pulseWidth = 0.5f;
        float lastSyncInPhase = 0.0f;
        Waveform waveform = Waveform::Triangle;
        uint32_t lfsr = 0x7FFFFF;
    };
}
