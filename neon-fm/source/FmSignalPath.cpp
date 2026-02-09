#include "FmSignalPath.h"
#include "ModulationTargets.h"
#include <cmath>
#include <algorithm>

namespace neon
{
    // Fast tanh approximation
    inline float fastTanh (float x)
    {
        if (x < -3.0f) return -1.0f;
        if (x > 3.0f) return 1.0f;
        float x2 = x * x;
        return x * (27.0f + x2) / (27.0f + 9.0f * x2);
    }

    FmSignalPath::FmSignalPath() : registry (ParameterRegistry::getInstance())
    {
    }

    void FmSignalPath::prepareToPlay (int samplesPerBlockExpected, double sr)
    {
        sampleRate = sr;
        samplesPerBlock = samplesPerBlockExpected;
        tempBuffer.setSize (2, samplesPerBlockExpected);

        // Prepare 2x oversampling
        oversampling.initProcessing ((size_t) samplesPerBlockExpected);
        oversampledRate = sr * (1 << oversamplingOrder);
        int osBlockSize = samplesPerBlockExpected * (1 << oversamplingOrder);
        osBuffer.setSize (2, osBlockSize);

        juce::dsp::ProcessSpec spec;
        spec.sampleRate = sr;
        spec.maximumBlockSize = (juce::uint32) samplesPerBlockExpected;
        spec.numChannels = 2;

        chorus.prepare (spec);
        phaser.prepare (spec);
        reverb.prepare (spec);
        delay.prepare (spec);

        for (auto& v : voices)
        {
            for (auto& op : v.ops)
                op.prepare (oversampledRate);  // operators run at oversampled rate

            // Filters also run at oversampled rate
            juce::dsp::ProcessSpec osSpec;
            osSpec.sampleRate = oversampledRate;
            osSpec.maximumBlockSize = (juce::uint32) osBlockSize;
            osSpec.numChannels = 2;

            v.filter1.prepare (osSpec);
            v.filter2.prepare (osSpec);
            v.ampEnv.setSampleRate (oversampledRate);
            v.filterEnv.setSampleRate (oversampledRate);
        }
    }

    void FmSignalPath::releaseResources() {}

    void FmSignalPath::noteOn (int midiNote, float velocity)
    {
        if (isMonoMode)
        {
            monoHeldNotes.push_back (midiNote);
            for (auto& v : voices)
            {
                if (v.isActive)
                {
                    v.ampEnv.noteOff();
                    for (auto& op : v.ops) op.noteOff();
                }
            }
        }

        // Find a free voice or steal the oldest
        Voice* voiceToUse = nullptr;
        double oldestNoteTime = std::numeric_limits<double>::max();
        Voice* oldestVoice = nullptr;

        for (auto& v : voices)
        {
            if (!v.isActive.load() || (v.midiNote == midiNote && v.ampEnv.isActive()))
            {
                voiceToUse = &v;
                break;
            }
            if (v.noteOnTime < oldestNoteTime)
            {
                oldestNoteTime = v.noteOnTime;
                oldestVoice = &v;
            }
        }

        if (voiceToUse == nullptr)
            voiceToUse = oldestVoice;

        if (voiceToUse != nullptr)
        {
            int voiceIdx = (int)(voiceToUse - &voices[0]);

            voiceToUse->reset();
            voiceToUse->midiNote = midiNote;
            voiceToUse->velocity = velocity;
            voiceToUse->isActive.store (true);
            voiceToUse->noteOnTime = juce::Time::getMillisecondCounterHiRes();

            float freq = (float) juce::MidiMessage::getMidiNoteInHertz (midiNote);
            voiceToUse->targetFrequency = freq;

            if (portaOn && lastMonoNote >= 0 && lastMonoNote != midiNote)
                voiceToUse->currentGlideFreq = currentPortaFreq;
            else
                voiceToUse->currentGlideFreq = freq;

            currentPortaFreq = freq;
            lastMonoNote = midiNote;

            // Set up operators
            for (int i = 0; i < 4; ++i)
            {
                auto& op = voiceToUse->ops[i];
                auto& gs = globalOps[i];

                op.waveform = (FmOperator::Waveform) gs.waveform;
                op.ratio = gs.ratio;
                op.detuneHz = gs.detune;
                op.level = gs.level;
                op.feedback = gs.feedback;
                op.keySync = gs.keySync;
                op.velocitySens = gs.velocitySens;

                // Apply velocity to level
                float velScale = 1.0f - gs.velocitySens + gs.velocitySens * velocity;
                op.level = gs.level * velScale;

                op.setEnvelopeParams (gs.envParams);
                op.noteOn();
            }

            // Master amp envelope
            voiceToUse->ampEnv.setParameters (ampParams);
            voiceToUse->ampEnv.noteOn();

            // Filter envelope
            voiceToUse->filterEnv.setParameters (filterEnvParams);
            voiceToUse->filterEnv.noteOn();

            // Reset LFO states for this voice
            for (int i = 0; i < 2; ++i)
            {
                if (globalLfos[i].keySync)
                    voiceLfoStates[voiceIdx].lfos[i].reset (globalLfos[i].phaseStart);
            }
        }
    }

    void FmSignalPath::setPolyAftertouch (int midiNote, float value)
    {
        for (auto& v : voices)
            if (v.isActive && v.midiNote == midiNote)
                v.aftertouch = value;
    }

    void FmSignalPath::setChannelAftertouch (float value)
    {
        for (auto& v : voices)
            if (v.isActive)
                v.aftertouch = value;
    }

    void FmSignalPath::noteOff (int midiNote)
    {
        for (auto& v : voices)
        {
            if (v.isActive && v.midiNote == midiNote)
            {
                v.ampEnv.noteOff();
                v.filterEnv.noteOff();
                for (auto& op : v.ops) op.noteOff();
            }
        }

        if (isMonoMode)
        {
            monoHeldNotes.erase (std::remove (monoHeldNotes.begin(), monoHeldNotes.end(), midiNote),
                                 monoHeldNotes.end());
            if (!monoHeldNotes.empty())
                noteOn (monoHeldNotes.back(), 0.8f);
        }
    }

    // ============================================================
    // Parameter polling
    // ============================================================
    void FmSignalPath::updateParams()
    {
        auto getVal = [this] (const juce::String& path, float fallback = 0.0f) {
            if (auto* p = registry.getParameter (path))
                return p->getValue();
            return fallback;
        };

        // Algorithm
        currentAlgorithm = (FmAlgorithmType)(int) getVal ("Algorithm/Algorithm", 0.0f);

        // Operators 1-4
        for (int i = 0; i < 4; ++i)
        {
            auto name = "Op " + juce::String (i + 1);
            auto& gs = globalOps[i];

            gs.waveform     = (int) getVal (name + "/Waveform", 0.0f);
            gs.ratio        = getVal (name + "/Ratio", 1.0f);
            gs.detune       = getVal (name + "/Detune", 0.0f);
            gs.level        = getVal (name + "/Level", i == 3 ? 1.0f : 0.8f);
            gs.feedback     = getVal (name + "/Feedback", 0.0f);
            gs.velocitySens = getVal (name + "/Velocity", 0.5f);
            gs.keySync      = getVal (name + "/KeySync", 1.0f) > 0.5f;

            float sustain = getVal (name + "/Sustain", 0.7f);
            if (sustain < 0.001f) sustain = 0.001f;

            gs.envParams = juce::ADSR::Parameters ({
                getVal (name + "/Attack", 10.0f) / 1000.0f,
                getVal (name + "/Decay", 500.0f) / 1000.0f,
                sustain,
                getVal (name + "/Release", 500.0f) / 1000.0f
            });
        }

        // Filter
        filterType = (int) getVal ("Filter/Type", 0.0f);
        baseFilterCutoff = getVal ("Filter/Cutoff", 20000.0f);
        baseFilterRes = getVal ("Filter/Res", 0.0f);
        filterKeyTrack = getVal ("Filter/KeyTrack", 0.0f);
        filterIs24dB = getVal ("Filter/Slope", 1.0f) > 0.5f;

        // Filter Envelope
        {
            float feSustain = getVal ("Filter Env/Sustain", 0.0f);
            if (feSustain < 0.001f) feSustain = 0.001f;
            filterEnvParams = juce::ADSR::Parameters ({
                getVal ("Filter Env/Attack", 10.0f) / 1000.0f,
                getVal ("Filter Env/Decay", 300.0f) / 1000.0f,
                feSustain,
                getVal ("Filter Env/Release", 300.0f) / 1000.0f
            });
            filterEnvAmount = getVal ("Filter Env/Amount", 0.0f) / 100.0f;  // stored as -100..100, normalize to -1..1
        }

        // Amp
        ampLevel = getVal ("Amp Output/Level", 0.8f);
        ampVelocity = getVal ("Amp Output/Velocity", 0.5f);

        float sustain = getVal ("Amp Env/Sustain", 0.7f);
        if (sustain < 0.001f) sustain = 0.001f;
        ampParams = juce::ADSR::Parameters ({
            getVal ("Amp Env/Attack", 10.0f) / 1000.0f,
            getVal ("Amp Env/Decay", 500.0f) / 1000.0f,
            sustain,
            getVal ("Amp Env/Release", 500.0f) / 1000.0f
        });

        // LFOs
        auto updateLfo = [&] (LfoSettings& settings, const juce::String& name) {
            settings.shape      = (int) std::round (getVal (name + "/Shape", 0.0f));
            settings.syncMode   = getVal (name + "/Sync", 0.0f) > 0.5f;
            settings.rateHz     = getVal (name + "/Rate Hz", 1.0f) * 5.0f;
            settings.rateNoteIdx = (int) std::round (getVal (name + "/Rate Note", 4.0f));
            settings.keySync    = getVal (name + "/KeySync", 1.0f) > 0.5f;
            settings.phaseStart = getVal (name + "/Phase", 0.0f) / 360.0f;

            for (int i = 0; i < 4; ++i)
            {
                settings.slots[i].target = getVal (name + "/Slot " + juce::String (i + 1) + " Target", 0.0f);
                settings.slots[i].amount = getVal (name + "/Slot " + juce::String (i + 1) + " Amount", 0.0f);
            }
        };

        updateLfo (globalLfos[0], "LFO 1");
        updateLfo (globalLfos[1], "LFO 2");

        // Control
        pbRange = getVal ("Control/PB Range", 2.0f);
        isMonoMode = (int) getVal ("Control/Mode", 0.0f) == 1;
        portaOn = getVal ("Control/Porta On", 0.0f) > 0.5f;
        portaTime = getVal ("Control/Porta Time", 100.0f);

        // FX
        fxSettings.modType     = (int) getVal ("FX/Mod Type", 1.0f);
        fxSettings.modRate     = getVal ("FX/Mod Rate", 1.0f);
        fxSettings.modDepth    = getVal ("FX/Mod Depth", 0.5f);
        fxSettings.modFeedback = getVal ("FX/Mod Feedback", 0.0f);
        fxSettings.modMix      = getVal ("FX/Mod Mix", 0.0f);

        fxSettings.dlyTime     = getVal ("FX/Dly Time", 400.0f);
        fxSettings.dlyNoteIdx  = (int) std::round (getVal ("FX/Dly Note", 4.0f));
        fxSettings.dlyFeedback = getVal ("FX/Dly FB", 0.3f);
        fxSettings.dlyMix      = getVal ("FX/Dly Mix", 0.0f);
        fxSettings.dlySync     = getVal ("FX/Dly Sync", 0.0f) > 0.5f;

        fxSettings.rvbSize = getVal ("FX/Rvb Size", 0.5f);
        fxSettings.rvbDamp = getVal ("FX/Rvb Damp", 0.5f);
        fxSettings.rvbMix  = getVal ("FX/Rvb Mix", 0.0f);

        // Update FX DSP
        chorus.setRate (fxSettings.modRate);
        chorus.setDepth (fxSettings.modDepth);
        chorus.setFeedback (fxSettings.modFeedback);
        chorus.setCentreDelay (fxSettings.modType == 3 ? 7.0f : 20.0f);
        chorus.setMix (fxSettings.modType == 2 ? 0.0f : fxSettings.modMix);

        phaser.setRate (fxSettings.modRate);
        phaser.setDepth (fxSettings.modDepth);
        phaser.setFeedback (fxSettings.modFeedback);
        phaser.setMix (fxSettings.modType == 2 ? fxSettings.modMix : 0.0f);

        delay.feedback = fxSettings.dlyFeedback;
        delay.mix = fxSettings.dlyMix;
        float actualDlyTime = fxSettings.dlyTime;
        if (fxSettings.dlySync)
        {
            static constexpr float divs[] = { 0.0625f, 0.125f, 0.25f, 0.5f, 1.0f, 2.0f, 4.0f, 8.0f, 16.0f };
            int idx = juce::jlimit (0, 8, fxSettings.dlyNoteIdx);
            double secPerBeat = 60.0 / bpm;
            actualDlyTime = (float)(secPerBeat * divs[idx] * 1000.0);
        }
        delay.line.setDelay (actualDlyTime * (float) sampleRate / 1000.0f);

        juce::dsp::Reverb::Parameters rvbParams;
        rvbParams.roomSize = fxSettings.rvbSize;
        rvbParams.damping  = fxSettings.rvbDamp;
        rvbParams.width    = 1.0f;
        rvbParams.wetLevel = fxSettings.rvbMix;
        rvbParams.dryLevel = 1.0f - (fxSettings.rvbMix * 0.5f);
        reverb.setParameters (rvbParams);

        // Update envelopes on active voices
        for (auto& v : voices)
        {
            if (v.isActive)
            {
                v.ampEnv.setParameters (ampParams);
                v.filterEnv.setParameters (filterEnvParams);

                for (int i = 0; i < 4; ++i)
                    v.ops[i].setEnvelopeParams (globalOps[i].envParams);

                auto type = juce::dsp::StateVariableTPTFilterType::lowpass;
                if (filterType == 1) type = juce::dsp::StateVariableTPTFilterType::highpass;
                else if (filterType == 2) type = juce::dsp::StateVariableTPTFilterType::bandpass;
                v.filter1.setType (type);
                v.filter2.setType (type);
            }
        }
    }

    // ============================================================
    // LFO processing helper
    // ============================================================
    static float computeLfo (FmSignalPath::LfoState& state, int shape, float phaseInc)
    {
        state.phase += phaseInc;
        if (state.phase >= 1.0f) state.phase -= 1.0f;

        float p = state.phase;
        float out = 0.0f;

        switch (shape)
        {
            case 0: // Triangle
                out = (p < 0.5f) ? (4.0f * p - 1.0f) : (3.0f - 4.0f * p);
                break;
            case 1: // Ramp Up
                out = 2.0f * p - 1.0f;
                break;
            case 2: // Ramp Down
                out = 1.0f - 2.0f * p;
                break;
            case 3: // Square
                out = p < 0.5f ? 1.0f : -1.0f;
                break;
            case 4: // S&H
                if (p < phaseInc) // trigger on wrap
                {
                    state.lastOutput = juce::Random::getSystemRandom().nextFloat() * 2.0f - 1.0f;
                    state.shTriggered = true;
                }
                out = state.lastOutput;
                break;
            default:
                out = std::sin (p * juce::MathConstants<float>::twoPi);
                break;
        }

        state.lastOutput = out;
        return out;
    }

    // ============================================================
    // Audio processing
    // ============================================================
    void FmSignalPath::getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill)
    {
        auto* buffer = bufferToFill.buffer;
        auto numSamples = bufferToFill.numSamples;

        buffer->clear (bufferToFill.startSample, numSamples);
        updateParams();

        // --- Oversampled voice rendering ---
        // Upsample: create an oversampled block from the output buffer
        juce::dsp::AudioBlock<float> inputBlock (*buffer);
        inputBlock = inputBlock.getSubBlock ((size_t) bufferToFill.startSample, (size_t) numSamples);
        auto osBlock = oversampling.processSamplesUp (inputBlock);
        auto osNumSamples = (int) osBlock.getNumSamples();

        osBlock.clear();

        auto* outL = osBlock.getChannelPointer (0);
        auto* outR = osBlock.getChannelPointer (1);

        for (int voiceIdx = 0; voiceIdx < numVoices; ++voiceIdx)
        {
            auto& v = voices[voiceIdx];
            if (!v.isActive.load()) continue;

            for (int s = 0; s < osNumSamples; ++s)
            {
                // Portamento glide (at oversampled rate)
                if (portaOn && v.currentGlideFreq != v.targetFrequency)
                {
                    float glideRate = 1.0f - std::exp (-1.0f / (portaTime * 0.001f * (float) oversampledRate));
                    v.currentGlideFreq += (v.targetFrequency - v.currentGlideFreq) * glideRate;
                }
                else
                {
                    v.currentGlideFreq = v.targetFrequency;
                }

                // Pitch bend
                float pbSemitones = pitchWheel * pbRange;
                float baseFreq = v.currentGlideFreq * std::pow (2.0f, pbSemitones / 12.0f);

                // LFO processing for this voice (at oversampled rate)
                float lfoOutputs[2] = { 0.0f, 0.0f };
                for (int li = 0; li < 2; ++li)
                {
                    auto& ls = globalLfos[li];
                    auto& lState = voiceLfoStates[voiceIdx].lfos[li];

                    float phaseInc = 0.0f;
                    if (ls.syncMode)
                    {
                        static constexpr float divs[] = { 0.0625f, 0.125f, 0.25f, 0.5f, 1.0f, 2.0f, 4.0f, 8.0f, 16.0f };
                        int idx = juce::jlimit (0, 8, ls.rateNoteIdx);
                        float beatsPerSec = (float)(bpm / 60.0);
                        float hz = beatsPerSec / divs[idx];
                        phaseInc = hz / (float) oversampledRate;
                    }
                    else
                    {
                        phaseInc = ls.rateHz / (float) oversampledRate;
                    }

                    lfoOutputs[li] = computeLfo (lState, ls.shape, phaseInc);
                }

                // Apply LFO modulation to operators
                float lfoFreqMod = 0.0f;
                float lfoOpLevelMod[4] = { 0.0f };

                for (int li = 0; li < 2; ++li)
                {
                    for (int si = 0; si < 4; ++si)
                    {
                        int target = (int) globalLfos[li].slots[si].target;
                        float amount = globalLfos[li].slots[si].amount / 100.0f;
                        float lfoVal = lfoOutputs[li] * amount;

                        if (target == (int) FmModTarget::MasterPitch)
                            lfoFreqMod += lfoVal * 2.0f;

                        if (target >= (int) FmModTarget::Op1Level && target <= (int) FmModTarget::Op4Level)
                        {
                            int opIdx = target - (int) FmModTarget::Op1Level;
                            lfoOpLevelMod[opIdx] += lfoVal;
                        }

                        if (target >= (int) FmModTarget::Op1Ratio && target <= (int) FmModTarget::Op4Ratio)
                        {
                            int opIdx = target - (int) FmModTarget::Op1Ratio;
                            v.ops[opIdx].ratio = globalOps[opIdx].ratio + lfoVal * 2.0f;
                        }

                        if (target == (int) FmModTarget::FilterCutoff)
                        {
                            float modCut = baseFilterCutoff * std::pow (2.0f, lfoVal * 4.0f);
                            baseFilterCutoff = juce::jlimit (20.0f, 20000.0f, modCut);
                        }
                    }
                }

                // Apply pitch LFO mod
                float modFreq = baseFreq * std::pow (2.0f, lfoFreqMod / 12.0f);

                // Temporarily adjust operator levels for LFO modulation
                float savedLevels[4];
                for (int i = 0; i < 4; ++i)
                {
                    savedLevels[i] = v.ops[i].level;
                    v.ops[i].level = juce::jlimit (0.0f, 1.0f, v.ops[i].level + lfoOpLevelMod[i]);
                }

                // Process FM algorithm
                float sample = FmAlgorithmRouter::process (currentAlgorithm, v.ops, modFreq);

                // Restore levels
                for (int i = 0; i < 4; ++i)
                    v.ops[i].level = savedLevels[i];

                // Master amp envelope
                float ampEnvVal = v.ampEnv.getNextSample();

                // Velocity scaling on output
                float velScale = 1.0f - ampVelocity + ampVelocity * v.velocity;

                // Filter envelope modulation
                float filterEnvVal = v.filterEnv.getNextSample();

                // Filter (runs at oversampled rate for better response)
                float cutoff = baseFilterCutoff;

                // Apply filter envelope: bipolar amount modulates cutoff in octaves
                if (std::abs (filterEnvAmount) > 0.001f)
                {
                    float envOctaves = filterEnvVal * filterEnvAmount * 8.0f;  // up to +/- 8 octaves
                    cutoff *= std::pow (2.0f, envOctaves);
                }

                if (filterKeyTrack > 0.0f)
                {
                    float noteFreq = (float) juce::MidiMessage::getMidiNoteInHertz (v.midiNote);
                    cutoff *= std::pow (noteFreq / 261.63f, filterKeyTrack);
                }
                cutoff = juce::jlimit (20.0f, 20000.0f, cutoff);

                v.filter1.setCutoffFrequency (cutoff);
                v.filter1.setResonance (juce::jlimit (0.1f, 5.0f, baseFilterRes * 5.0f));
                float filtered = v.filter1.processSample (0, sample);

                if (filterIs24dB)
                {
                    v.filter2.setCutoffFrequency (cutoff);
                    v.filter2.setResonance (juce::jlimit (0.1f, 5.0f, baseFilterRes * 5.0f));
                    filtered = v.filter2.processSample (0, filtered);
                }

                float finalSample = filtered * ampEnvVal * velScale * ampLevel;

                // Soft clip
                finalSample = fastTanh (finalSample);

                outL[s] += finalSample;
                outR[s] += finalSample;

                // Check if voice is done
                if (!v.ampEnv.isActive())
                {
                    bool anyOpActive = false;
                    for (auto& op : v.ops)
                        if (op.isActive()) anyOpActive = true;
                    if (!anyOpActive)
                    {
                        v.isActive.store (false);
                        break;
                    }
                }
            }
        }

        // --- Downsample back to normal rate ---
        oversampling.processSamplesDown (inputBlock);

        // === FX Processing (at normal sample rate) ===
        juce::dsp::ProcessContextReplacing<float> context (inputBlock);

        // Modulation FX (Chorus/Phaser/Flanger)
        if (fxSettings.modType > 0 && fxSettings.modMix > 0.0f)
        {
            if (fxSettings.modType == 2)
                phaser.process (context);
            else
                chorus.process (context);
        }

        // Delay
        if (fxSettings.dlyMix > 0.0f)
            delay.process (context);

        // Reverb
        if (fxSettings.rvbMix > 0.0f)
            reverb.process (context);
    }

    // ============================================================
    // Delay processing
    // ============================================================
    void FmSignalPath::DelayBlock::process (juce::dsp::ProcessContextReplacing<float>& context)
    {
        auto& block = context.getOutputBlock();
        auto numSamples = block.getNumSamples();
        auto* dataL = block.getChannelPointer (0);
        auto* dataR = block.getChannelPointer (std::min ((size_t) 1, block.getNumChannels() - 1));

        for (size_t i = 0; i < numSamples; ++i)
        {
            float dryL = dataL[i], dryR = dataR[i];
            float wetL = line.popSample (0);
            float wetR = (block.getNumChannels() > 1) ? line.popSample (1) : wetL;

            line.pushSample (0, dryL + wetL * feedback);
            if (block.getNumChannels() > 1)
                line.pushSample (1, dryR + wetR * feedback);

            dataL[i] = dryL + wetL * mix;
            dataR[i] = dryR + wetR * mix;
        }
    }

} // namespace neon
