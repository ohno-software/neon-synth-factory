#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_dsp/juce_dsp.h>
#include <neon_ui_components/neon_ui_components.h>
#include <atomic>
#include <vector>

namespace neon
{
    /**
     * SignalPath
     * The master audio engine for Neon Jr.
     * Polls the ParameterRegistry to drive the DSP.
     */
    class SignalPath : public juce::AudioSource
    {
    public:
        SignalPath();
        ~SignalPath() override = default;

        void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override;
        void getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill) override;
        void releaseResources() override;

        void noteOn (int midiNote, float velocity);
        void noteOff (int midiNote);
        void setPitchWheel (float value) { pitchWheel = value; }
        void setModWheel (float value) { modWheel = value; }
        void setAftertouch (float value) { aftertouch = value; }
        void setBpm (double newBpm) { bpm = newBpm; }

        float getPitchWheel() const { return pitchWheel; }
        float getModWheel() const { return modWheel; }
        float getAftertouch() const { return aftertouch; }
        
        int getActiveVoicesCount() const 
        {
            int count = 0;
            for (auto& v : voices) if (v.isActive.load()) count++;
            return count;
        }

        struct OscState
        {
            float phases[8] = {0.0f}; // Support up to 8 unison voices
            float currentFrequency = 440.0f;
            
            // Parameter cache (polled from registry)
            int waveIdx = 0;
            float symmetry = 0.5f;
            float fold = 0.0f;
            float volume = 0.8f;
            float pan = 0.0f;
            float detune = 0.0f;
            float transp = 0.0f;
            float drive = 0.0f;
            float bitRedux = 0.0f;
            int unison = 1;
            float uSpread = 0.1f;
            float phaseStart = 0.0f;
            bool keySync = true;
        };

        struct LfoSettings
        {
            int shape = 0; // Tri, RampUp, RampDn, Sq, S&H
            bool syncMode = false; // false=Hz, true=Note
            float rateHz = 1.0f;
            int rateNoteIdx = 4;
            bool keySync = true;
            float phaseStart = 0.0f;
            float delayMs = 0.0f;
            
            struct ModSlot { float target = 0.0f; float amount = 0.0f; };
            ModSlot slots[4];
        };

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

        struct Voice
        {
            int midiNote = -1;
            float velocity = 0.0f;
            std::atomic<bool> isActive { false };
            double noteOnTime = 0.0; // For voice stealing

            OscState osc1, osc2;
            std::array<LfoState, 3> lfos;
            juce::ADSR ampEnv, filterEnv, pitchEnv, modEnv;
            juce::dsp::StateVariableTPTFilter<float> filter1;
            juce::dsp::StateVariableTPTFilter<float> filter2;

            // Decimation cache for CPU optimization
            float u1Freqs[8], u1GainsL[8], u1GainsR[8];
            float u2Freqs[8], u2GainsL[8], u2GainsR[8];
            float mOsc1Pitch = 0, mOsc2Pitch = 0;
            float mOsc1Sym = 0, mOsc1Fold = 0, mOsc1Drive = 0, mOsc1Bit = 0, mOsc1Lvl = 0, mOsc1Pan = 0;
            float mOsc2Sym = 0, mOsc2Fold = 0, mOsc2Drive = 0, mOsc2Bit = 0, mOsc2Lvl = 0, mOsc2Pan = 0;
            float mOsc1Det = 0, mOsc2Det = 0;
            float mFiltCut = 0, mFiltRes = 0;
            float mLfoAmts[3][4] = {{0.0f}};
            int decimationCounter = 0;

            void reset()
            {
                for (int i = 0; i < 8; ++i)
                {
                    osc1.phases[i] = 0.0f;
                    osc2.phases[i] = 0.0f;
                }
                for (auto& l : lfos) l.reset(0.0f, -1.0);
                
                ampEnv.reset();
                filterEnv.reset();
                pitchEnv.reset();
                modEnv.reset();
                filter1.reset();
                filter2.reset();
                
                for (int i = 0; i < 3; ++i)
                    for (int j = 0; j < 4; ++j)
                        mLfoAmts[i][j] = 0.0f;

                isActive.store (false);
                midiNote = -1;
            }
        };

        struct ArpSettings
        {
            bool enabled = false;
            int rateNoteIdx = 2;
            int mode = 0;
            int octaves = 1;
            float gate = 0.8f;
            bool latch = false;
        };

        struct ArpState
        {
            std::vector<int> heldNotes;
            std::vector<int> sequence;
            int sequenceIndex = 0;
            double timer = 0.0;
            int activeNote = -1;
            bool gateOpen = false;
            int physicalKeysDown = 0;

            void reset() { heldNotes.clear(); sequence.clear(); sequenceIndex = 0; timer = 0.0; activeNote = -1; gateOpen = false; physicalKeysDown = 0; }
        };

        struct FxSettings
        {
            int modType = 0; // 0=Off, 1=Chorus, 2=Phaser, 3=Flanger
            float modRate = 1.0f;
            float modDepth = 0.5f;
            float modMix = 0.0f;

            float dlyTime = 400.0f;
            float dlyFeedback = 0.3f;
            float dlyMix = 0.0f;
            bool dlySync = false;

            float rvbSize = 0.5f;
            float rvbDamp = 0.5f;
            float rvbWidth = 0.5f;
            float rvbMix = 0.0f;
        };

    private:
        void updateParams();
        void updateArpSequence();
        void handleArp (int numSamples);
        void updateWavetables();
        float renderOscSample (float& phase, float actualFreq, const OscState& settings, const juce::AudioBuffer<float>& table);
        
        double sampleRate = 44100.0;
        int samplesPerBlock = 512;
        double bpm = 120.0;
        double internalBpm = 120.0;
        bool useHostBpm = false;

        ArpSettings arpSettings;
        ArpState arpState;
        FxSettings fxSettings;
        bool isMonoMode = false;

        // DSP FX
        juce::dsp::Chorus<float> chorus;
        juce::dsp::Phaser<float> phaser;
        juce::dsp::Reverb reverb;
        
        struct DelayBlock
        {
            juce::dsp::DelayLine<float> line { 192000 }; // 4 seconds at 48k
            float feedback = 0.0f;
            float mix = 0.0f;
            float lastL = 0, lastR = 0;
            void prepare (const juce::dsp::ProcessSpec& spec) { line.prepare(spec); lastL = 0; lastR = 0; }
            void process (juce::dsp::ProcessContextReplacing<float>& context);
        } delay;
        
        juce::AudioFormatManager formatManager;
        juce::Array<juce::File> waveformFiles;
        std::vector<juce::AudioBuffer<float>> wavetables;
        
        // Polyphony
        static constexpr int numVoices = 16;
        std::array<Voice, numVoices> voices;
        
        // Global Parameter Cache (shared across voices)
        OscState globalOsc1, globalOsc2;
        std::array<LfoSettings, 3> globalLfos;
        int   filterType = 0; // 0=LP, 1=HP, 2=BP
        float baseFilterCutoff = 20000.0f;
        float baseFilterRes = 0.0f;
        float baseFilterDrive = 1.0f;
        float filterKeyTrack = 0.5f;
        bool filterIs24dB = true;
        
        float filterEnvAmount = 0.0f;
        float pitchEnvAmount = 0.0f;
        float ampLevel = 0.8f;

        float pitchWheel = 0.0f;  // -1.0 to 1.0
        float modWheel = 0.0f;    // 0.0 to 1.0
        float aftertouch = 0.0f;  // 0.0 to 1.0
        float pbRange = 2.0f;     // Pitch bend range in semitones
        
        float ampEnvMode = 0.0f;
        float filterEnvMode = 0.0f;
        float pitchEnvMode = 0.0f;
        float modEnvMode = 0.0f;
        
        juce::ADSR::Parameters ampParams, filterParams, pitchParams, modParams;

        struct ModSlot
        {
            float target = 0.0f;
            float amount = 0.0f;
        };
        ModSlot modSlots[4];

        struct CtrlSlot
        {
            float source = 0.0f;
            float target = 0.0f;
            float amount = 0.0f;
        };
        std::array<CtrlSlot, 8> ctrlSlots;
        
        ParameterRegistry& registry;
        juce::AudioBuffer<float> tempBuffer;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SignalPath)
    };
}
