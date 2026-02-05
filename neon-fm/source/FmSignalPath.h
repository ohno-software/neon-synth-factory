#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_dsp/juce_dsp.h>
#include <neon_ui_components/neon_ui_components.h>
#include <atomic>
#include <array>

#include "FmOperator.h"
#include "FmAlgorithm.h"

namespace neon
{
    /**
     * FmSignalPath
     * The master audio engine for Neon FM.
     * 4-operator FM synthesis with analog oscillators.
     * Polls the ParameterRegistry to drive the DSP.
     */
    class FmSignalPath : public juce::AudioSource
    {
    public:
        FmSignalPath();
        ~FmSignalPath() override = default;

        void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override;
        void getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill) override;
        void releaseResources() override;

        void noteOn (int midiNote, float velocity);
        void noteOff (int midiNote);
        void setPolyAftertouch (int midiNote, float value);
        void setChannelAftertouch (float value);
        void setPitchWheel (float value) { pitchWheel = value; }
        void setModWheel (float value) { modWheel = value; }
        void setBpm (double newBpm) { bpm = newBpm; }

        float getPitchWheel() const { return pitchWheel; }
        float getModWheel() const { return modWheel; }

        int getActiveVoicesCount() const
        {
            int count = 0;
            for (auto& v : voices) if (v.isActive.load()) count++;
            return count;
        }

        // Per-operator parameter cache
        struct OpSettings
        {
            int waveform = 0;           // 0=Sine, 1=Tri, 2=Saw, 3=Square
            float ratio = 1.0f;         // Frequency ratio (coarse)
            float detune = 0.0f;        // Fine detune in Hz
            float level = 1.0f;         // Operator output level
            float feedback = 0.0f;      // Self-modulation (only meaningful for some algos)
            float velocitySens = 0.5f;  // Velocity sensitivity
            bool keySync = true;        // Phase reset on note-on

            // Per-operator envelope
            juce::ADSR::Parameters envParams { 0.01f, 0.5f, 0.7f, 0.5f };
        };

        // Per-voice state
        struct Voice
        {
            int midiNote = -1;
            float velocity = 0.0f;
            float aftertouch = 0.0f;
            std::atomic<bool> isActive { false };
            double noteOnTime = 0.0;

            // Portamento
            float targetFrequency = 440.0f;
            float currentGlideFreq = 440.0f;

            // 4 FM operators per voice
            std::array<FmOperator, 4> ops;

            // Master filter per voice
            juce::dsp::StateVariableTPTFilter<float> filter1;
            juce::dsp::StateVariableTPTFilter<float> filter2;

            // Master amplitude envelope
            juce::ADSR ampEnv;

            void reset()
            {
                for (auto& op : ops) op.reset();
                ampEnv.reset();
                filter1.reset();
                filter2.reset();
                isActive.store (false);
                midiNote = -1;
            }
        };

        struct FxSettings
        {
            int modType = 0;
            float modRate = 1.0f;
            float modDepth = 0.5f;
            float modFeedback = 0.0f;
            float modMix = 0.0f;

            float dlyTime = 400.0f;
            float dlyFeedback = 0.3f;
            float dlyMix = 0.0f;
            bool dlySync = false;
            int dlyNoteIdx = 4;

            float rvbSize = 0.5f;
            float rvbDamp = 0.5f;
            float rvbMix = 0.0f;
        };

    public:
        // LFO state needs to be accessible for static helper
        struct LfoState
        {
            float phase = 0.0f;
            float lastOutput = 0.0f;
            bool shTriggered = false;
            void reset (float startPhase) { phase = startPhase; lastOutput = 0.0f; shTriggered = false; }
        };

    private:
        void updateParams();

        double sampleRate = 44100.0;
        double oversampledRate = 88200.0;
        int samplesPerBlock = 512;
        double bpm = 120.0;

        // Global operator settings (shared, read by all voices)
        std::array<OpSettings, 4> globalOps;
        FmAlgorithmType currentAlgorithm = FmAlgorithmType::SerialChain;

        // Filter
        int filterType = 0;
        float baseFilterCutoff = 20000.0f;
        float baseFilterRes = 0.0f;
        float filterKeyTrack = 0.0f;
        bool filterIs24dB = true;

        // Master amp
        float ampLevel = 0.8f;
        float ampVelocity = 0.5f;
        juce::ADSR::Parameters ampParams { 0.01f, 0.3f, 0.7f, 0.3f };

        // Performance controls
        float pitchWheel = 0.0f;
        float modWheel = 0.0f;
        float pbRange = 2.0f;

        // Portamento
        bool portaOn = false;
        float portaTime = 100.0f;
        float currentPortaFreq = 440.0f;
        int lastMonoNote = -1;
        bool isMonoMode = false;
        std::vector<int> monoHeldNotes;

        // LFO
        struct LfoSettings
        {
            int shape = 0;
            bool syncMode = false;
            float rateHz = 1.0f;
            int rateNoteIdx = 4;
            bool keySync = true;
            float phaseStart = 0.0f;

            struct ModSlot { float target = 0.0f; float amount = 0.0f; };
            ModSlot slots[4];
        };

        std::array<LfoSettings, 2> globalLfos;

        // Per-voice LFO states are stored separately  
        struct VoiceLfoState
        {
            std::array<LfoState, 2> lfos;
        };
        std::array<VoiceLfoState, 16> voiceLfoStates;

        // FX
        FxSettings fxSettings;
        juce::dsp::Chorus<float> chorus;
        juce::dsp::Phaser<float> phaser;
        juce::dsp::Reverb reverb;

        struct DelayBlock
        {
            juce::dsp::DelayLine<float> line { 192000 };
            float feedback = 0.0f;
            float mix = 0.0f;
            float lastL = 0, lastR = 0;
            void prepare (const juce::dsp::ProcessSpec& spec) { line.prepare (spec); lastL = 0; lastR = 0; }
            void process (juce::dsp::ProcessContextReplacing<float>& ctx);
        } delay;

        juce::AudioBuffer<float> tempBuffer;

        // 2x oversampling to reduce FM aliasing
        static constexpr int oversamplingOrder = 1; // 2^1 = 2x
        juce::dsp::Oversampling<float> oversampling { 2, oversamplingOrder,
            juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR, true };
        juce::AudioBuffer<float> osBuffer;

        // Polyphony
        static constexpr int numVoices = 16;
        std::array<Voice, numVoices> voices;

        ParameterRegistry& registry;
    };

} // namespace neon
