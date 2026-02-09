#pragma once

#include <juce_dsp/juce_dsp.h>
#include <cmath>
#include <cstdint>

namespace neon
{
    /**
     * ChipOscillator
     * Pure DSP class for generating Atari 2600 (TIA) style sound.
     * Uses polynomial counters (LFSR) and dividers.
     */
    class ChipOscillator
    {
    public:
        /**
         * Waveform index maps to various TIA AUDCx modes.
         * 0=Square, 1=Div6, 2=Div31, 3=Poly4, 4=Poly5, 5=Poly9
         */

        void setSampleRate (double sr) { sampleRate = std::max(1.0, sr); }

        void setWaveform (int idx)       { waveformIndex = idx; }
        void setBitDepth (int bits)      { bitDepth = juce::jlimit (1, 16, bits); }

        void noteOn (float freqHz, bool resetPhase)
        {
            frequency = std::max(0.1f, freqHz);
            if (resetPhase)
            {
                phase = 0.0f;
                lfsr4 = 0xF;
                lfsr5 = 0x1F;
                lfsr9 = 0x1FF;
            }
        }

        float process()
        {
            float out = 0.0f;

            switch (waveformIndex)
            {
                case 0: // Square
                    out = (phase < 0.5f) ? 1.0f : -1.0f;
                    break;
                case 1: // Div 6 (Buzz)
                    out = (std::fmod (phase * 6.0f, 1.0f) < 0.5f) ? 1.0f : -1.0f;
                    break;
                case 2: // Div 31 (Low Buzz)
                    out = (std::fmod (phase * 31.0f, 1.0f) < 0.5f) ? 1.0f : -1.0f;
                    break;
                case 3: // Poly 4 (Lead/Noise)
                    out = processPoly4();
                    break;
                case 4: // Poly 5 (Metallic)
                    out = processPoly5();
                    break;
                case 5: // Poly 9 (Noise)
                    out = processPoly9();
                    break;
                default:
                    out = (phase < 0.5f) ? 1.0f : -1.0f;
                    break;
            }

            // Bit-crush to emulate limited DAC resolution (TIA is 4-bit volume)
            if (bitDepth < 16)
            {
                float levels = (float) (1 << bitDepth);
                out = std::round (out * levels) / levels;
            }

            advancePhase();
            return out;
        }

        void reset()
        {
            phase = 0.0f;
            lfsr4 = 0xF;
            lfsr5 = 0x1F;
            lfsr9 = 0x1FF;
        }

    private:
        void advancePhase()
        {
            float inc = (float) (frequency / sampleRate);
            phase += inc;
            if (phase >= 1.0f)
            {
                phase -= 1.0f;
                stepLfsrs();
            }
        }

        void stepLfsrs()
        {
            // Poly 4: x^4 + x + 1
            uint32_t b4 = ((lfsr4 >> 0) ^ (lfsr4 >> 1)) & 1;
            lfsr4 = (lfsr4 >> 1) | (b4 << 3);

            // Poly 5: x^5 + x^2 + 1
            uint32_t b5 = ((lfsr5 >> 0) ^ (lfsr5 >> 2)) & 1;
            lfsr5 = (lfsr5 >> 1) | (b5 << 4);

            // Poly 9: x^9 + x^4 + 1
            uint32_t b9 = ((lfsr9 >> 0) ^ (lfsr9 >> 4)) & 1;
            lfsr9 = (lfsr9 >> 1) | (b9 << 8);
        }

        float processPoly4() { return (lfsr4 & 1) ? 1.0f : -1.0f; }
        float processPoly5() { return (lfsr5 & 1) ? 1.0f : -1.0f; }
        float processPoly9() { return (lfsr9 & 1) ? 1.0f : -1.0f; }

        double sampleRate = 44100.0;
        float phase = 0.0f;
        float frequency = 440.0f;
        int waveformIndex = 0;
        int bitDepth = 4; // Atari is 4-bit

        uint32_t lfsr4 = 0xF;
        uint32_t lfsr5 = 0x1F;
        uint32_t lfsr9 = 0x1FF;
    };

} // namespace neon
