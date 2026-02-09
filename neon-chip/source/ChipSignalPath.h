#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_dsp/juce_dsp.h>
#include <neon_ui_components/neon_ui_components.h>
#include <atomic>
#include <array>
#include <vector>

#include "ChipOscillator.h"

namespace neon
{
    /**
     * ChipSignalPath
     * The master audio engine for Neon Chip.
     * SID / NES / Atari 2600 chip-sound synthesis with polyphonic voice management.
     * Polls the ParameterRegistry to drive the DSP — following the same architecture
     * as FmSignalPath (neon-fm) and SignalPath (neon-jr).
     */
    class ChipSignalPath : public juce::AudioSource
    {
    public:
        ChipSignalPath();
        ~ChipSignalPath() override = default;

        void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override;
        void getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill) override;
        void releaseResources() override;

        void noteOn (int midiNote, float velocity);
        void noteOff (int midiNote);
        void setPolyAftertouch (int midiNote, float value);
        void setChannelAftertouch (float value);
        void setPitchWheel (float value) { pitchWheel = value; }
        void setModWheel (float value)   { modWheel = value; }
        void setBpm (double newBpm)      { bpm = newBpm; }

        float getPitchWheel() const { return pitchWheel; }
        float getModWheel()  const { return modWheel; }

        int getActiveVoicesCount() const
        {
            int count = 0;
            for (auto& v : voices)
                if (v.isActive.load()) count++;
            return count;
        }

        // ─── Voice ───
        struct Voice
        {
            int midiNote = -1;
            float velocity = 0.0f;
            float aftertouch = 0.0f;
            std::atomic<bool> isActive { false };
            double noteOnTime = 0.0;

            ChipOscillator osc;
            juce::ADSR ampEnv;
            juce::dsp::StateVariableTPTFilter<float> filter;

            void reset()
            {
                osc.reset();
                ampEnv.reset();
                filter.reset();
                isActive.store (false);
                midiNote = -1;
            }
        };

    private:
        void updateParams();

        double sampleRate = 44100.0;
        int samplesPerBlock = 512;
        double bpm = 120.0;

        // Performance controls
        float pitchWheel = 0.0f;
        float modWheel = 0.0f;
        float pbRange = 2.0f;

        // Cached global parameters (polled from ParameterRegistry each block)
        int waveformIndex = 0;
        int bitDepth = 4;
        float oscVolume = 0.8f;

        // Filter cache
        int filterType = 0;
        float filterCutoff = 20000.0f;
        float filterRes = 0.0f;

        // Amp envelope cache
        juce::ADSR::Parameters ampParams { 0.005f, 0.3f, 0.7f, 0.3f };
        float ampLevel = 0.8f;
        float ampVelocity = 0.5f;

        // Polyphony
        static constexpr int numVoices = 8;
        std::array<Voice, numVoices> voices;

        juce::AudioBuffer<float> tempBuffer;

        ParameterRegistry& registry;
    };

} // namespace neon
