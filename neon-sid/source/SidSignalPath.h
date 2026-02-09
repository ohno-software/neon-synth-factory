#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_dsp/juce_dsp.h>
#include <neon_ui_components/neon_ui_components.h>
#include "SidOscillator.h"
#include <atomic>
#include <vector>
#include <array>

namespace neon
{
    /**
     * SidSignalPath
     * Polyphonic SID engine with 3 oscillators per voice.
     */
    class SidSignalPath : public juce::AudioSource
    {
    public:
        SidSignalPath();
        ~SidSignalPath() override = default;

        void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override;
        void getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill) override;
        void releaseResources() override;

        void noteOn (int midiNote, float velocity);
        void noteOff (int midiNote);
        void setPitchWheel (float value) { pitchWheel = value; }
        void setModWheel (float value) { modWheel = value; }

        struct OscParams
        {
            int waveform = 0;
            float volume = 0.5f;
            float transpose = 0.0f;
            float fine = 0.0f;
            float pulseWidth = 0.5f;
            bool sync = false;
            bool ringMod = false;
        };

        struct Voice
        {
            int midiNote = -1;
            float velocity = 0.0f;
            std::atomic<bool> isActive { false };
            double noteOnTime = 0.0;

            SidOscillator osc1, osc2, osc3;
            juce::ADSR ampEnv;
            juce::dsp::StateVariableTPTFilter<float> filter;

            void reset()
            {
                osc1.reset(); osc2.reset(); osc3.reset();
                ampEnv.reset();
                filter.reset();
                isActive.store (false);
                midiNote = -1;
            }
        };

    private:
        void updateParams();

        ParameterRegistry& registry;
        std::array<Voice, 8> voices;
        double sampleRate = 44100.0;
        float pitchWheel = 0.0f;
        float modWheel = 0.0f;

        OscParams osc1Params, osc2Params, osc3Params;
        
        float filterCutoff = 1000.0f;
        float filterRes = 0.1f;
        int filterType = 0;

        juce::ADSR::Parameters ampParams;
    };
}
