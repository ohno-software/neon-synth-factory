#include "SignalPath.h"
#include "ModulationTargets.h"
#include <cmath>
#include <algorithm>
#include <random>

namespace neon
{
    SignalPath::SignalPath() : registry (ParameterRegistry::getInstance())
    {
        formatManager.registerBasicFormats();
        
        // Try to find the waves-1 directory relative to the binary or in known locations
        juce::File exeFile = juce::File::getSpecialLocation(juce::File::currentExecutableFile);
        juce::File waveDir = exeFile.getParentDirectory().getChildFile("waves-1");

        // Fallback for development/standalone on Windows
        if (!waveDir.exists())
        {
            // Use parent of current executable as relative root for development
            waveDir = exeFile.getParentDirectory().getParentDirectory().getChildFile("neon-jr/waves-1");
        }
        
        if (!waveDir.exists())
            waveDir = juce::File::getCurrentWorkingDirectory().getChildFile("../neon-jr/waves-1");
            
        waveformFiles = waveDir.findChildFiles (juce::File::findFiles, false, "*.wav");
        
        // Initial setup of master wavetables
        updateWavetables();
    }

    void SignalPath::updateWavetables()
    {
        wavetables.clear();
        std::vector<juce::String> names;
        
        for (auto& file : waveformFiles)
        {
            std::unique_ptr<juce::AudioFormatReader> reader (formatManager.createReaderFor (file));
            if (reader != nullptr)
            {
                juce::AudioBuffer<float> buffer (1, (int)reader->lengthInSamples);
                reader->read (&buffer, 0, (int)reader->lengthInSamples, 0, true, true);
                wavetables.push_back (std::move (buffer));
                names.push_back (file.getFileNameWithoutExtension());
            }
        }
        
        // Ensure at least one working table exists if none found (fallback to basic Sine/Saw)
        if (wavetables.empty())
        {
            juce::AudioBuffer<float> basicWave (1, 2048);
            for (int i = 0; i < 2048; ++i) {
                float phase = (float)i / 2048.0f;
                // Sawtooth
                basicWave.setSample(0, i, phase * 2.0f - 1.0f);
            }
            wavetables.push_back (std::move (basicWave));
            names.push_back ("Basic Saw");
        }

        // Register with the UI registry
        NeonRegistry::setWaveformNames (names);
        for (int i = 0; i < (int)wavetables.size(); ++i)
            NeonRegistry::setWaveformBuffer (i, &wavetables[i]);
    }

    void SignalPath::prepareToPlay (int samplesPerBlockExpected, double sr)
    {
        sampleRate = sr;
        samplesPerBlock = samplesPerBlockExpected;
        tempBuffer.setSize (2, samplesPerBlockExpected);
        
        juce::dsp::ProcessSpec spec;
        spec.sampleRate = sr;
        spec.maximumBlockSize = (juce::uint32)samplesPerBlockExpected;
        spec.numChannels = 2;
        
        chorus.prepare (spec);
        phaser.prepare (spec);
        reverb.prepare (spec);
        delay.prepare (spec);

        for (auto& v : voices)
        {
            v.filter1.prepare (spec);
            v.filter2.prepare (spec);
            v.ampEnv.setSampleRate (sr);
            v.filterEnv.setSampleRate (sr);
            v.pitchEnv.setSampleRate (sr);
            v.modEnv.setSampleRate (sr);
        }
    }

    void SignalPath::releaseResources() {}

    void SignalPath::noteOn (int midiNote, float velocity)
    {
        if (arpSettings.enabled)
        {
            if (arpSettings.latch)
            {
                if (arpState.physicalKeysDown == 0)
                    arpState.heldNotes.clear();
            }
            arpState.physicalKeysDown++;

            arpState.heldNotes.push_back (midiNote);
            std::sort (arpState.heldNotes.begin(), arpState.heldNotes.end());
            arpState.heldNotes.erase (std::unique (arpState.heldNotes.begin(), arpState.heldNotes.end()), arpState.heldNotes.end());
            updateArpSequence();

            // If Arp is doing something, we don't trigger notes here manually 
            // unless we want some complex "Arp + Lead" mode. Let's stick to pure Arp for now.
            if (arpState.sequence.size() > 0)
                return;
        }

        // Mono Mode: Kill other voices
        if (isMonoMode)
        {
            for (auto& v : voices)
            {
                if (v.isActive)
                {
                    v.ampEnv.noteOff();
                    v.filterEnv.noteOff();
                    v.pitchEnv.noteOff();
                    v.modEnv.noteOff();
                }
            }
        }

        // 1. Find a voice to use (including voice stealing)
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
            voiceToUse->reset();
            voiceToUse->midiNote = midiNote;
            voiceToUse->velocity = velocity;
            voiceToUse->isActive.store (true);
            voiceToUse->noteOnTime = juce::Time::getMillisecondCounterHiRes();
            voiceToUse->decimationCounter = juce::Random::getSystemRandom().nextInt (8);

            float freq = (float)juce::MidiMessage::getMidiNoteInHertz (midiNote);
            voiceToUse->osc1.currentFrequency = freq;
            if (globalOsc1.keySync) {
                for (int i = 0; i < 8; ++i) voiceToUse->osc1.phases[i] = globalOsc1.phaseStart;
            }

            voiceToUse->osc2.currentFrequency = freq;
            if (globalOsc2.keySync) {
                for (int i = 0; i < 8; ++i) voiceToUse->osc2.phases[i] = globalOsc2.phaseStart;
            }

            // LFOs KeySync
            for (int i = 0; i < 3; ++i)
            {
                if (globalLfos[i].keySync)
                    voiceToUse->lfos[i].reset (globalLfos[i].phaseStart, voiceToUse->noteOnTime);
                else
                    voiceToUse->lfos[i].noteOnTime = voiceToUse->noteOnTime;
            }

            // Apply global params initially
            voiceToUse->osc1.waveIdx = globalOsc1.waveIdx;
            voiceToUse->osc2.waveIdx = globalOsc2.waveIdx;

            voiceToUse->ampEnv.setParameters (ampParams);
            voiceToUse->filterEnv.setParameters (filterParams);
            voiceToUse->pitchEnv.setParameters (pitchParams);
            voiceToUse->modEnv.setParameters (modParams);

            voiceToUse->ampEnv.noteOn();
            voiceToUse->filterEnv.noteOn();
            voiceToUse->pitchEnv.noteOn();
            voiceToUse->modEnv.noteOn();
        }
    }

    void SignalPath::noteOff (int midiNote)
    {
        if (arpSettings.enabled)
        {
            arpState.physicalKeysDown = std::max (0, arpState.physicalKeysDown - 1);

            if (!arpSettings.latch)
            {
                arpState.heldNotes.erase (std::remove (arpState.heldNotes.begin(), arpState.heldNotes.end(), midiNote), arpState.heldNotes.end());
                updateArpSequence();
            }

            if (arpState.sequence.size() > 0)
                return;
        }

        for (auto& v : voices)
        {
            if (v.isActive && v.midiNote == midiNote)
            {
                v.ampEnv.noteOff();
                v.filterEnv.noteOff();
                v.pitchEnv.noteOff();
                v.modEnv.noteOff();
            }
        }
        
        // If Mono, play previous held note if it exists
        if (isMonoMode && !arpState.heldNotes.empty())
        {
            noteOn (arpState.heldNotes.back(), 0.8f);
        }
    }

    void SignalPath::updateParams()
    {
        auto getVal = [this] (const juce::String& path, float fallback = 0.0f) {
            if (auto* p = registry.getParameter (path))
                return p->getValue();
            return fallback;
        };

        auto updateGlobalOsc = [&](OscState& state, const juce::String& name) {
            state.waveIdx  = (int)getVal (name + "/Waveform", 0.0f);
            state.symmetry = getVal (name + "/Symmetry", 0.5f);
            state.detune   = getVal (name + "/Detune", 0.0f);
            state.transp   = getVal (name + "/Transp", 0.0f);
            state.phaseStart = getVal (name + "/Phase", 0.0f) / 360.0f;
            state.keySync    = getVal (name + "/KeySync", 1.0f) > 0.5f;
            state.volume   = getVal (name + "/Volume", 0.8f);
            state.pan      = getVal (name + "/Pan", 0.0f);
            state.drive    = getVal (name + "/Drive", 0.0f);
            state.bitRedux = getVal (name + "/BitRedux", 0.0f);
            state.fold     = getVal (name + "/Fold", 0.0f);
            state.unison   = (int)juce::jlimit(1.0f, 8.0f, getVal (name + "/Unison", 1.0f));
            state.uSpread  = getVal (name + "/USpread", 0.2f);
        };

        updateGlobalOsc (globalOsc1, "Oscillator 1");
        updateGlobalOsc (globalOsc2, "Oscillator 2");

        // LFOs
        auto updateGlobalLfo = [&](LfoSettings& settings, const juce::String& name) {
            settings.shape = (int)std::round (getVal (name + "/Shape", 0.0f));
            settings.syncMode = getVal (name + "/Sync", 0.0f) > 0.5f;
            settings.rateHz = getVal (name + "/Rate Hz", 1.0f);
            settings.rateNoteIdx = (int)std::round (getVal (name + "/Rate Note", 4.0f));
            settings.keySync = getVal (name + "/KeySync", 1.0f) > 0.5f;
            settings.phaseStart = getVal (name + "/Phase", 0.0f) / 360.0f;
            settings.delayMs = getVal (name + "/Delay", 0.0f);
            
            for (int i = 0; i < 4; ++i)
            {
                settings.slots[i].target = getVal (name + "/Slot " + juce::String (i + 1) + " Target", 0.0f);
                settings.slots[i].amount = getVal (name + "/Slot " + juce::String (i + 1) + " Amount", 0.0f);
            }
        };

        updateGlobalLfo (globalLfos[0], "LFO 1");
        updateGlobalLfo (globalLfos[1], "LFO 2");
        updateGlobalLfo (globalLfos[2], "LFO 3");

        // Filter
        filterType = (int)getVal ("Ladder Filter/Type", 0.0f);
        baseFilterCutoff = getVal ("Ladder Filter/Cutoff", 20000.0f);
        baseFilterRes = getVal ("Ladder Filter/Res", 0.0f);
        baseFilterDrive = getVal ("Ladder Filter/Drive", 1.0f);
        filterKeyTrack = getVal ("Ladder Filter/KeyTrack", 0.5f);
        filterIs24dB = getVal ("Ladder Filter/Slope", 1.0f) > 0.5f;

        // Env - Update ADSR from DAHDSR params
        auto getEnvParams = [&] (const juce::String& mod) {
            return juce::ADSR::Parameters({
                getVal (mod + "/Attack", 10.0f) / 1000.0f,
                getVal (mod + "/Decay", 500.0f) / 1000.0f,
                getVal (mod + "/Sustain", 0.7f),
                getVal (mod + "/Release", 500.0f) / 1000.0f
            });
        };

        ampParams = getEnvParams ("Amp Env");
        filterParams = getEnvParams ("Filter Env");
        pitchParams = getEnvParams ("Pitch Env");
        modParams = getEnvParams ("Mod Env");

        filterEnvAmount = getVal ("Filter Env/Amount", 0.0f);
        pitchEnvAmount = getVal ("Pitch Env/Amount", 0.0f);
        
        for (int i = 0; i < 4; ++i)
        {
            auto prefix = "Mod Env/Slot " + juce::String (i + 1);
            modSlots[i].target = getVal (prefix + " Target", 0.0f);
            modSlots[i].amount = getVal (prefix + " Amount", 0.0f);
        }

        for (int i = 0; i < 8; ++i)
        {
            auto prefix = "Mod/Slot " + juce::String (i + 1);
            ctrlSlots[i].source = getVal (prefix + " Source", 0.0f);
            ctrlSlots[i].target = getVal (prefix + " Target", 0.0f);
            ctrlSlots[i].amount = getVal (prefix + " Amount", 0.0f);
        }

        pbRange = getVal ("Control/PB Range", 2.0f);
        isMonoMode = (int)getVal ("Control/Mode", 0.0f) == 1;

        internalBpm = getVal ("Control/Tempo", 120.0f);
        useHostBpm = getVal ("Control/Tempo Sync", 1.0f) > 0.5f;

        if (!useHostBpm)
            bpm = internalBpm;

        ampLevel = getVal ("Amp Output/Level", 0.8f);

        // Arp
        bool wasArpEnabled = arpSettings.enabled;
        arpSettings.enabled = getVal ("Arp/Arp On", 0.0f) > 0.5f;

        if (wasArpEnabled && !arpSettings.enabled)
        {
            // Turn off all arp-triggered notes
            if (arpState.activeNote != -1)
            {
                for (auto& v : voices) if (v.isActive && v.midiNote == arpState.activeNote) v.ampEnv.noteOff();
            }
            arpState.reset();
        }

        arpSettings.rateNoteIdx = (int)getVal ("Arp/Rate Note", 2.0f);
        arpSettings.mode = (int)getVal ("Arp/Mode", 0.0f);
        arpSettings.octaves = (int)getVal ("Arp/Octave", 1.0f);
        arpSettings.gate = getVal ("Arp/Gate", 80.0f) / 100.0f;
        
        bool newLatch = getVal ("Arp/Latch", 0.0f) > 0.5f;
        if (arpSettings.latch && !newLatch && arpState.physicalKeysDown == 0)
        {
            arpState.heldNotes.clear();
            updateArpSequence();
        }
        arpSettings.latch = newLatch;

        // FX
        fxSettings.modType = (int)getVal ("FX/Mod Type", 1.0f);
        fxSettings.modRate = getVal ("FX/Mod Rate", 1.0f);
        fxSettings.modDepth = getVal ("FX/Mod Depth", 0.5f);
        fxSettings.modMix = getVal ("FX/Mod Mix", 0.0f);

        fxSettings.dlyTime = getVal ("FX/Dly Time", 400.0f);
        fxSettings.dlyFeedback = getVal ("FX/Dly FB", 0.3f);
        fxSettings.dlyMix = getVal ("FX/Dly Mix", 0.0f);
        fxSettings.dlySync = getVal ("FX/Dly Sync", 0.0f) > 0.5f;

        fxSettings.rvbSize = getVal ("FX/Rvb Size", 0.5f);
        fxSettings.rvbDamp = getVal ("FX/Rvb Damp", 0.5f);
        fxSettings.rvbWidth = getVal ("FX/Rvb Width", 0.5f);
        fxSettings.rvbMix = getVal ("FX/Rvb Mix", 0.0f);

        // Update DSP parameters
        chorus.setRate (fxSettings.modRate);
        chorus.setDepth (fxSettings.modDepth);
        chorus.setCentreDelay (fxSettings.modType == 3 ? 7.0f : 20.0f); // Flanger usually uses shorter delay
        chorus.setMix (fxSettings.modType == 2 ? 0.0f : fxSettings.modMix); // Phaser uses its own mix

        phaser.setRate (fxSettings.modRate);
        phaser.setDepth (fxSettings.modDepth);
        phaser.setMix (fxSettings.modType == 2 ? fxSettings.modMix : 0.0f);

        delay.feedback = fxSettings.dlyFeedback;
        delay.mix = fxSettings.dlyMix;
        float actualDlyTime = fxSettings.dlyTime;
        if (fxSettings.dlySync) {
            static constexpr float divs[] = { 0.0625f, 0.125f, 0.25f, 0.5f, 1.0f, 2.0f, 4.0f, 8.0f, 16.0f };
            int idx = juce::jlimit (0, 8, (int)std::round(getVal("FX/Dly Time", 4.0f))); // Note selector might be reusing time param
            // For now let's just use the MS value or map it. Actually Arp uses Rate Note idx.
            // Let's keep it simple and just use the MS value for now unless user asks for more sync.
        }
        delay.line.setDelay (actualDlyTime * (float)sampleRate / 1000.0f);

        juce::dsp::Reverb::Parameters rvbParams;
        rvbParams.roomSize = fxSettings.rvbSize;
        rvbParams.damping = fxSettings.rvbDamp;
        rvbParams.width = fxSettings.rvbWidth;
        rvbParams.wetLevel = fxSettings.rvbMix;
        rvbParams.dryLevel = 1.0f - (fxSettings.rvbMix * 0.5f);
        reverb.setParameters (rvbParams);
        
        // Push most critical per-block global params to active voices
        // (Envelopes are updated in noteOn or if we want real-time parameter changes we do it here)
        for (auto& v : voices)
        {
            if (v.isActive)
            {
                v.ampEnv.setParameters (ampParams);
                v.filterEnv.setParameters (filterParams);
                v.pitchEnv.setParameters (pitchParams);
                v.modEnv.setParameters (modParams);
                
                auto type = juce::dsp::StateVariableTPTFilterType::lowpass;
                if (filterType == 1) type = juce::dsp::StateVariableTPTFilterType::highpass;
                else if (filterType == 2) type = juce::dsp::StateVariableTPTFilterType::bandpass;

                v.filter1.setType (type);
                v.filter2.setType (type);
                
                // Set resonance (Q)
                float resonanceQ = 0.707f + (baseFilterRes * 10.0f);
                v.filter1.setResonance (resonanceQ);
                v.filter2.setResonance (resonanceQ);
            }
        }
    }

    float SignalPath::renderOscSample (float& phase, float actualFreq, const OscState& state, const juce::AudioBuffer<float>& table)
    {
        if (actualFreq <= 0.01f) return 0.0f;

        double phaseInc = (double)actualFreq / sampleRate;
        float val = 0.0f;

        if (table.getNumSamples() > 0)
        {
            auto* data = table.getReadPointer (0);
            int numSamples = table.getNumSamples();

            // Symmetry warping and Phase logic (Optimized math)
            float s = juce::jlimit (0.01f, 0.99f, state.symmetry);
            double dPhase = (double)phase;
            double warpedPhase = (dPhase < (double)s) ? 
                (dPhase * 0.5) / (double)s : 
                0.5 + ((dPhase - (double)s) * 0.5) / (1.0 - (double)s);

            int idx1 = (int)(warpedPhase * numSamples);
            int idx2 = (idx1 + 1) % numSamples;
            float frac = (float)(warpedPhase * (double)numSamples - (double)idx1);
            
            val = data[idx1] * (1.0f - frac) + data[idx2] * frac;
        }

        // Apply Drive
        if (state.drive > 0.01f)
            val = std::tanh (val * (1.0f + state.drive * 4.0f));

        // BitRedux
        if (state.bitRedux > 0.05f)
        {
            float steps = juce::jmap (1.0f - state.bitRedux, 2.0f, 32.0f);
            val = std::round (val * steps) / steps;
        }

        // Apply Fold
        if (state.fold > 0.01f)
        {
            val *= (1.0f + state.fold * 3.0f);
            if (val > 1.0f) val = 1.0f - (val - 1.0f);
            else if (val < -1.0f) val = -1.0f - (val + 1.0f);
        }

        phase = (float)std::fmod ((double)phase + phaseInc, 1.0);
        if (std::isnan(phase)) phase = 0.0f;

        return val * state.volume;
    }

    float calculateLfo (SignalPath::LfoState& state, const SignalPath::LfoSettings& settings, double sr, double bpm)
    {
        if (sr <= 0.0) return 0.0f;

        // Apply Delay
        if (settings.delayMs > 0.01f && state.noteOnTime > 0)
        {
            double elapsedMs = juce::Time::getMillisecondCounterHiRes() - state.noteOnTime;
            if (elapsedMs < (double)settings.delayMs)
                return 0.0f;
        }

        double rateHz = settings.rateHz;
        if (settings.syncMode)
        {
            // Note sync rates: 1/64, 1/32, 1/16, 1/8, 1/4, 1/2, 1/1, 2/1, 4/1
            static constexpr double divs[] = { 
                0.0625, 0.125, 0.25, 0.5, 1.0, 2.0, 4.0, 8.0, 16.0 
            };
            int idx = juce::jlimit (0, 8, settings.rateNoteIdx);
            double beatsPerSec = bpm / 60.0;
            // rate = cycles per second. 1 cycle = 'div' beats.
            // so rateHz = beatsPerSec / div
            rateHz = beatsPerSec / divs[idx];
        }

        double phaseInc = rateHz / sr;
        float out = 0.0f;
        float phase = state.phase;

        switch (settings.shape)
        {
            case 0: // Triangle
                out = (phase < 0.5f) ? (phase * 4.0f - 1.0f) : (3.0f - phase * 4.0f);
                break;
            case 1: // Ramp Up
                out = phase * 2.0f - 1.0f;
                break;
            case 2: // Ramp Down
                out = 1.0f - phase * 2.0f;
                break;
            case 3: // Square
                out = (phase < 0.5f) ? 1.0f : -1.0f;
                break;
            case 4: // S&H
                if (phase < phaseInc || !state.sampleHoldTriggered)
                {
                    state.lastOutput = (juce::Random::getSystemRandom().nextFloat() * 2.0f - 1.0f);
                    state.sampleHoldTriggered = true;
                }
                out = state.lastOutput;
                break;
        }

        state.phase = (float)std::fmod (phase + phaseInc, 1.0);
        if (state.phase < phase) state.sampleHoldTriggered = false; // Reset trigger on wrap
        
        return out;
    }

    void SignalPath::getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill)
    {
        bufferToFill.clearActiveBufferRegion();
        updateParams();
        
        int numSamples = bufferToFill.numSamples;
        handleArp (numSamples);
        
        auto* mainOutL = bufferToFill.buffer->getWritePointer (0, bufferToFill.startSample);
        auto* mainOutR = bufferToFill.buffer->getWritePointer (1, bufferToFill.startSample);

        // Constant power pan helpers
        auto getL = [] (float pan) { return std::cos ((pan + 1.0f) * (juce::MathConstants<float>::pi * 0.25f)); };
        auto getR = [] (float pan) { return std::sin ((pan + 1.0f) * (juce::MathConstants<float>::pi * 0.25f)); };

        for (auto& v : voices)
        {
            if (!v.isActive.load()) continue;

            // Voice-specific buffers
            tempBuffer.clear();
            auto* vL = tempBuffer.getWritePointer(0);
            auto* vR = tempBuffer.getWritePointer(1);

            // Fetch wavetables
            int tableCount = (int)wavetables.size();
            const auto& table1 = wavetables[juce::jlimit(0, juce::jmax(0, tableCount - 1), globalOsc1.waveIdx)];
            const auto& table2 = wavetables[juce::jlimit(0, juce::jmax(0, tableCount - 1), globalOsc2.waveIdx)];

            float midInOctaves = std::log2 (juce::jlimit(20.0f, (float)sampleRate * 0.45f, baseFilterCutoff) / 20.0f);
            float kTrackOctaves = ((v.midiNote - 60.0f) / 12.0f) * filterKeyTrack;
            
            for (int s = 0; s < numSamples; ++s)
            {
                float envA = v.ampEnv.getNextSample();
                float envF = v.filterEnv.getNextSample(); 
                float envP = v.pitchEnv.getNextSample();
                float rawMod = v.modEnv.getNextSample();

                float pbShift = pitchWheel * pbRange; // Apply Pitch Bend Range

                // Voice lifecycle: stay active as long as the Amp or Filter envelopes are running
                if (envA < 0.00001f && !v.ampEnv.isActive() && !v.filterEnv.isActive())
                {
                    v.isActive.store (false);
                    break;
                }
                
                // Optimized Modulation: Only update mod matrix every 8 samples
                if (v.decimationCounter++ % 8 == 0)
                {
                    v.mOsc1Pitch = pbShift; v.mOsc2Pitch = pbShift; // Start with Pitch Bend
                    v.mOsc1Sym = 0; v.mOsc1Fold = 0; v.mOsc1Drive = 0; v.mOsc1Bit = 0; v.mOsc1Lvl = 0; v.mOsc1Pan = 0;
                    v.mOsc2Sym = 0; v.mOsc2Fold = 0; v.mOsc2Drive = 0; v.mOsc2Bit = 0; v.mOsc2Lvl = 0; v.mOsc2Pan = 0;
                    v.mOsc1Det = 0; v.mOsc2Det = 0;
                    v.mFiltCut = 0; v.mFiltRes = 0;
                    
                    for (int i = 0; i < 3; ++i)
                        for (int j = 0; j < 4; ++j)
                            v.mLfoAmts[i][j] = 0.0f;

                    auto applyMod = [&](float target, float amount) {
                        if (!std::isfinite(amount)) return;
                        switch (static_cast<ModTarget>(std::round(target)))
                        {
                            case ModTarget::Osc1Pitch:    v.mOsc1Pitch += amount * 12.0f; break; 
                            case ModTarget::Osc1Symmetry: v.mOsc1Sym   += amount; break;
                            case ModTarget::Osc1Fold:     v.mOsc1Fold  += amount; break;
                            case ModTarget::Osc1Drive:    v.mOsc1Drive += amount; break;
                            case ModTarget::Osc1BitRedux: v.mOsc1Bit   += amount; break;
                            case ModTarget::Osc1Level:    v.mOsc1Lvl   += amount; break;
                            case ModTarget::Osc1Pan:      v.mOsc1Pan   += amount; break;
                            case ModTarget::Osc1Detune:   v.mOsc1Det   += amount * 100.0f; break; 
                            
                            case ModTarget::Osc2Pitch:    v.mOsc2Pitch += amount * 12.0f; break;
                            case ModTarget::Osc2Symmetry: v.mOsc2Sym   += amount; break;
                            case ModTarget::Osc2Fold:     v.mOsc2Fold  += amount; break;
                            case ModTarget::Osc2Drive:    v.mOsc2Drive += amount; break;
                            case ModTarget::Osc2BitRedux: v.mOsc2Bit   += amount; break;
                            case ModTarget::Osc2Level:    v.mOsc2Lvl   += amount; break;
                            case ModTarget::Osc2Pan:      v.mOsc2Pan   += amount; break;
                            case ModTarget::Osc2Detune:   v.mOsc2Det   += amount * 100.0f; break; 

                            case ModTarget::Osc1And2Pitch:    v.mOsc1Pitch += amount * 12.0f; v.mOsc2Pitch += amount * 12.0f; break;
                            case ModTarget::Osc1And2Symmetry: v.mOsc1Sym   += amount; v.mOsc2Sym   += amount; break;
                            case ModTarget::Osc1And2Fold:     v.mOsc1Fold  += amount; v.mOsc2Fold  += amount; break;
                            case ModTarget::Osc1And2Drive:    v.mOsc1Drive += amount; v.mOsc2Drive += amount; break;
                            case ModTarget::Osc1And2BitRedux: v.mOsc1Bit   += amount; v.mOsc2Bit   += amount; break;
                            case ModTarget::Osc1And2Level:    v.mOsc1Lvl   += amount; v.mOsc2Lvl   += amount; break;
                            case ModTarget::Osc1And2Pan:      v.mOsc1Pan   += amount; v.mOsc2Pan   += amount; break;
                            case ModTarget::Osc1And2Detune:   v.mOsc1Det   += amount * 100.0f; v.mOsc2Det += amount * 100.0f; break;
                            
                            case ModTarget::FilterCutoff: v.mFiltCut   += amount; break;
                            case ModTarget::FilterRes:    v.mFiltRes   += amount; break;

                            case ModTarget::Lfo1Amount1: v.mLfoAmts[0][0] += amount; break;
                            case ModTarget::Lfo1Amount2: v.mLfoAmts[0][1] += amount; break;
                            case ModTarget::Lfo1Amount3: v.mLfoAmts[0][2] += amount; break;
                            case ModTarget::Lfo1Amount4: v.mLfoAmts[0][3] += amount; break;
                            case ModTarget::Lfo2Amount1: v.mLfoAmts[1][0] += amount; break;
                            case ModTarget::Lfo2Amount2: v.mLfoAmts[1][1] += amount; break;
                            case ModTarget::Lfo2Amount3: v.mLfoAmts[1][2] += amount; break;
                            case ModTarget::Lfo2Amount4: v.mLfoAmts[1][3] += amount; break;
                            case ModTarget::Lfo3Amount1: v.mLfoAmts[2][0] += amount; break;
                            case ModTarget::Lfo3Amount2: v.mLfoAmts[2][1] += amount; break;
                            case ModTarget::Lfo3Amount3: v.mLfoAmts[2][2] += amount; break;
                            case ModTarget::Lfo3Amount4: v.mLfoAmts[2][3] += amount; break;

                            default: break;
                        }
                    };

                    // Mod Env Matrix
                    for (int i = 0; i < 4; ++i)
                        applyMod (modSlots[i].target, rawMod * (modSlots[i].amount / 100.0f));

                    // Control/MIDI Matrix
                    auto getCtrlVal = [&](CtrlSource src) -> float {
                        switch (src) {
                            case CtrlSource::PitchBend: return pitchWheel;
                            case CtrlSource::ModWheel:  return modWheel;
                            case CtrlSource::Aftertouch: return aftertouch;
                            case CtrlSource::Velocity:   return v.velocity;
                            case CtrlSource::KeyTrack:   return (v.midiNote - 60.0f) / 60.0f;
                            default: return 0.0f;
                        }
                    };

                    for (int i = 0; i < 8; ++i)
                        applyMod (ctrlSlots[i].target, getCtrlVal(static_cast<CtrlSource>(std::round(ctrlSlots[i].source))) * (ctrlSlots[i].amount / 100.0f));

                    // LFO Matrix
                    for (int l = 0; l < 3; ++l)
                    {
                        float lfoVal = calculateLfo (v.lfos[l], globalLfos[l], (float)sampleRate, bpm);
                        for (int i = 0; i < 4; ++i)
                        {
                            float modifiedAmount = (globalLfos[l].slots[i].amount / 100.0f) + v.mLfoAmts[l][i];
                            applyMod (globalLfos[l].slots[i].target, lfoVal * modifiedAmount);
                        }
                    }
                }

                // 1. Oscillators with Unison
                float osc1SumL = 0, osc1SumR = 0;
                float osc2SumL = 0, osc2SumR = 0;
                
                auto renderUnison = [&](OscState& gState, float& vFreq, float* vPhases, float* uFreqsCache, float* uGainsLCache, float* uGainsRCache, float pitchEnv, const juce::AudioBuffer<float>& table, float modDetune, float& rowL, float& rowR, int sIndex) {
                    int count = juce::jlimit(1, 8, gState.unison);
                    float sL = 0, sR = 0;
                    
                    if (sIndex % 8 == 0)
                    {
                        float envShift = pitchEnv * (pitchEnvAmount * 24.0f);
                        float pitchMult = std::pow(2.0f, envShift / 12.0f);
                        float baseFrequency = vFreq * pitchMult;
                        float norm = 1.0f / std::sqrt((float)count);

                        for (int i = 0; i < count; ++i)
                        {
                            float spreadOffset = (count > 1) ? (float)i / (float)(count - 1) * 2.0f - 1.0f : 0.0f;
                            float totalDetune = (gState.transp * 100.0f) + gState.detune + modDetune + (gState.uSpread * 50.0f * spreadOffset);
                            uFreqsCache[i] = baseFrequency * std::pow(2.0f, totalDetune / 1200.0f);
                            
                            float uPan = juce::jlimit(-1.0f, 1.0f, gState.pan + (spreadOffset * gState.uSpread));
                            uGainsLCache[i] = std::sqrt((1.0f - uPan) * 0.5f) * norm;
                            uGainsRCache[i] = std::sqrt((1.0f + uPan) * 0.5f) * norm;
                        }
                    }

                    for (int i = 0; i < count; ++i)
                    {
                        float samp = renderOscSample(vPhases[i], uFreqsCache[i], gState, table);
                        sL += samp * uGainsLCache[i];
                        sR += samp * uGainsRCache[i];
                    }
                    rowL = sL;
                    rowR = sR;
                };

                OscState tOsc1 = globalOsc1;
                tOsc1.symmetry = juce::jlimit(0.01f, 0.99f, tOsc1.symmetry + v.mOsc1Sym);
                tOsc1.fold     = juce::jlimit(0.0f, 1.0f, tOsc1.fold + v.mOsc1Fold);
                tOsc1.drive    = juce::jlimit(0.0f, 1.0f, tOsc1.drive + v.mOsc1Drive);
                tOsc1.bitRedux = juce::jlimit(0.0f, 1.0f, tOsc1.bitRedux + v.mOsc1Bit);
                tOsc1.volume   = juce::jlimit(0.0f, 1.0f, tOsc1.volume + v.mOsc1Lvl);
                tOsc1.pan      = juce::jlimit(-1.0f, 1.0f, tOsc1.pan + v.mOsc1Pan);

                OscState tOsc2 = globalOsc2;
                tOsc2.symmetry = juce::jlimit(0.01f, 0.99f, tOsc2.symmetry + v.mOsc2Sym);
                tOsc2.fold     = juce::jlimit(0.0f, 1.0f, tOsc2.fold + v.mOsc2Fold);
                tOsc2.drive    = juce::jlimit(0.0f, 1.0f, tOsc2.drive + v.mOsc2Drive);
                tOsc2.bitRedux = juce::jlimit(0.0f, 1.0f, tOsc2.bitRedux + v.mOsc2Bit);
                tOsc2.volume   = juce::jlimit(0.0f, 1.0f, tOsc2.volume + v.mOsc2Lvl);
                tOsc2.pan      = juce::jlimit(-1.0f, 1.0f, tOsc2.pan + v.mOsc2Pan);

                renderUnison(tOsc1, v.osc1.currentFrequency, v.osc1.phases, v.u1Freqs, v.u1GainsL, v.u1GainsR, envP, table1, v.mOsc1Det + (v.mOsc1Pitch * 100.0f), osc1SumL, osc1SumR, s);
                renderUnison(tOsc2, v.osc2.currentFrequency, v.osc2.phases, v.u2Freqs, v.u2GainsL, v.u2GainsR, envP, table2, v.mOsc2Det + (v.mOsc2Pitch * 100.0f), osc2SumL, osc2SumR, s);

                float sampL = (osc1SumL + osc2SumL) * 0.5f;
                float sampR = (osc1SumR + osc2SumR) * 0.5f;

                float driveGain = 1.0f + (baseFilterDrive - 1.0f) * 3.0f; // Aggressive gain multiplier

                // 2. Filter (State Variable Filter - Optimized)
                // We only update coefficients every 8 samples to save massive CPU
                if ((v.decimationCounter - 1) % 8 == 0)
                {
                    float totalOctaves = midInOctaves + kTrackOctaves + (envF * filterEnvAmount / 10.0f) + (v.mFiltCut * 5.0f);
                    float modulatedHz = 20.0f * std::pow (2.0f, totalOctaves);
                    modulatedHz = juce::jlimit (20.0f, (float)sampleRate * 0.45f, modulatedHz);

                    if (std::isfinite (modulatedHz))
                    {
                        v.filter1.setCutoffFrequency (modulatedHz);
                        
                        // Resonance modulation (mapped to SVF Q)
                        float currentRes = juce::jlimit(0.0f, 1.0f, baseFilterRes + v.mFiltRes);
                        float q = 0.707f + (currentRes * 15.0f);
                        v.filter1.setResonance (q); 
                        
                        if (filterIs24dB)
                        {
                            v.filter2.setCutoffFrequency (modulatedHz);
                            v.filter2.setResonance (q);
                        }
                    }
                }

                // Apply pre-filter drive for harmonic saturation
                sampL *= driveGain;
                sampR *= driveGain;

                sampL = v.filter1.processSample (0, sampL);
                sampR = v.filter1.processSample (1, sampR);
                
                // Intermediate non-linear stage (Aggressive!)
                if (baseFilterDrive > 1.1f)
                {
                    sampL = std::tanh (sampL);
                    sampR = std::tanh (sampR);
                }

                if (filterIs24dB)
                {
                    sampL = v.filter2.processSample (0, sampL);
                    sampR = v.filter2.processSample (1, sampR);

                    if (baseFilterDrive > 1.1f)
                    {
                        sampL = std::tanh (sampL);
                        sampR = std::tanh (sampR);
                    }
                }

                // 3. Final Amp & Sum
                float ampGain = envA * v.velocity * ampLevel;
                vL[s] = sampL * ampGain;
                vR[s] = sampR * ampGain;
            }

            for (int s = 0; s < numSamples; ++s)
            {
                mainOutL[s] += vL[s];
                mainOutR[s] += vR[s];
            }
        }

        // Apply Global Effects
        juce::dsp::AudioBlock<float> block (*bufferToFill.buffer);
        juce::dsp::ProcessContextReplacing<float> context (block);

        if (fxSettings.modType == 1 || fxSettings.modType == 3)
            chorus.process (context);
        else if (fxSettings.modType == 2)
            phaser.process (context);

        if (fxSettings.dlyMix > 0.01f)
            delay.process (context);

        if (fxSettings.rvbMix > 0.01f)
            reverb.process (context);
    }

    void SignalPath::DelayBlock::process (juce::dsp::ProcessContextReplacing<float>& context)
    {
        auto& block = context.getOutputBlock();
        auto* chL = block.getChannelPointer(0);
        auto* chR = block.getChannelPointer(1);
        int numSamples = (int)block.getNumSamples();

        for (int s = 0; s < numSamples; ++s)
        {
            float inL = chL[s];
            float inR = chR[s];

            float dlyL = line.popSample (0);
            float dlyR = line.popSample (1);

            line.pushSample (0, inL + dlyL * feedback);
            line.pushSample (1, inR + dlyR * feedback);

            chL[s] = inL + dlyL * mix;
            chR[s] = inR + dlyR * mix;
        }
    }

    void SignalPath::updateArpSequence()
    {
        arpState.sequence.clear();
        if (arpState.heldNotes.empty())
        {
            arpState.sequenceIndex = 0;
            return;
        }

        std::vector<int> sorted = arpState.heldNotes;
        std::sort (sorted.begin(), sorted.end());

        std::vector<int> baseNotes;
        int octs = (int)std::round (arpSettings.octaves);

        for (int o = 0; o < octs; ++o)
        {
            for (int n : sorted)
            {
                int note = n + (o * 12);
                if (note <= 127) baseNotes.push_back (note);
            }
        }

        if (arpSettings.mode == 0) // UP
        {
            arpState.sequence = baseNotes;
        }
        else if (arpSettings.mode == 1) // DOWN
        {
            arpState.sequence = baseNotes;
            std::reverse (arpState.sequence.begin(), arpState.sequence.end());
        }
        else if (arpSettings.mode == 2) // UP/DOWN
        {
            arpState.sequence = baseNotes;
            if (baseNotes.size() > 2)
            {
                for (int i = (int)baseNotes.size() - 2; i > 0; --i)
                    arpState.sequence.push_back (baseNotes[i]);
            }
        }
        else if (arpSettings.mode == 3) // RANDOM
        {
            arpState.sequence = baseNotes;
            auto seed = (unsigned int)juce::Time::getMillisecondCounter();
            std::shuffle (arpState.sequence.begin(), arpState.sequence.end(), std::default_random_engine(seed));
        }

        if (arpState.sequenceIndex >= (int)arpState.sequence.size())
            arpState.sequenceIndex = 0;
    }

    void SignalPath::handleArp (int numSamples)
    {
        if (!arpSettings.enabled || arpState.sequence.empty())
        {
            if (arpState.activeNote != -1)
            {
                for (auto& v : voices) 
                {
                    if (v.isActive && v.midiNote == arpState.activeNote)
                    {
                        v.ampEnv.noteOff();
                    }
                }
                arpState.activeNote = -1;
            }
            return;
        }

        // Arp is now always tempo synced
        static constexpr double divs[] = { 0.0625, 0.125, 0.25, 0.5, 1.0, 2.0, 4.0, 8.0, 16.0 };
        int idx = juce::jlimit (0, 8, arpSettings.rateNoteIdx);
        double beatsPerSec = bpm / 60.0;
        double rateHz = beatsPerSec / divs[idx];

        double samplesPerStep = sampleRate / rateHz;
        double gateSamples = samplesPerStep * arpSettings.gate;

        arpState.timer += numSamples;

        if (arpState.timer >= samplesPerStep)
        {
            arpState.timer -= samplesPerStep;
            
            if (!arpState.sequence.empty())
                arpState.sequenceIndex = (arpState.sequenceIndex + 1) % arpState.sequence.size();
            
            int nextNote = arpState.sequence[arpState.sequenceIndex];
            
            if (arpState.activeNote != -1)
            {
                for (auto& v : voices) 
                {
                    if (v.isActive && v.midiNote == arpState.activeNote)
                        v.ampEnv.noteOff();
                }
            }

            arpState.activeNote = nextNote;
            arpState.gateOpen = true;

            Voice* voiceToUse = nullptr;
            for (auto& v : voices) if (!v.isActive.load()) { voiceToUse = &v; break; }
            if (!voiceToUse) voiceToUse = &voices[0];

            voiceToUse->reset();
            voiceToUse->midiNote = nextNote;
            voiceToUse->velocity = 0.8f;
            voiceToUse->isActive.store (true);
            voiceToUse->noteOnTime = juce::Time::getMillisecondCounterHiRes();
            float freq = (float)juce::MidiMessage::getMidiNoteInHertz (nextNote);
            voiceToUse->osc1.currentFrequency = freq;
            voiceToUse->osc2.currentFrequency = freq;
            voiceToUse->ampEnv.setParameters (ampParams);
            voiceToUse->ampEnv.noteOn();
            voiceToUse->filterEnv.setParameters (filterParams);
            voiceToUse->filterEnv.noteOn();
            voiceToUse->pitchEnv.setParameters (pitchParams);
            voiceToUse->pitchEnv.noteOn();
            voiceToUse->modEnv.setParameters (modParams);
            voiceToUse->modEnv.noteOn();
        }
        else if (arpState.gateOpen && arpState.timer >= gateSamples)
        {
            arpState.gateOpen = false;
            if (arpState.activeNote != -1)
            {
                for (auto& v : voices) 
                {
                    if (v.isActive && v.midiNote == arpState.activeNote)
                        v.ampEnv.noteOff();
                }
            }
        }
    }
} // namespace neon
