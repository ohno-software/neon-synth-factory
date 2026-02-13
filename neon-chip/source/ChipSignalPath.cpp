#include "ChipSignalPath.h"
#include "ModulationTargets.h"
#include <cmath>
#include <algorithm>

namespace neon
{
    ChipSignalPath::ChipSignalPath()
        : registry (ParameterRegistry::getInstance())
    {
    }

    void ChipSignalPath::prepareToPlay (int samplesPerBlockExpected, double sr)
    {
        sampleRate = sr;
        samplesPerBlock = samplesPerBlockExpected;
        tempBuffer.setSize (2, samplesPerBlockExpected);

        juce::dsp::ProcessSpec spec;
        spec.sampleRate    = sr;
        spec.maximumBlockSize = (juce::uint32) samplesPerBlockExpected;
        spec.numChannels   = 1;

        for (auto& v : voices)
        {
            v.osc.setSampleRate (sr);
            v.ampEnv.setSampleRate (sr);
            v.filter.prepare (spec);
        }
    }

    void ChipSignalPath::releaseResources() {}

    // ─── MIDI ─────────────────────────────────────────────
    void ChipSignalPath::noteOn (int midiNote, float velocity)
    {
        Voice* voiceToUse = nullptr;
        double oldestTime = std::numeric_limits<double>::max();
        Voice* oldestVoice = nullptr;

        for (auto& v : voices)
        {
            if (!v.isActive.load() || v.midiNote == midiNote)
            {
                voiceToUse = &v;
                break;
            }
            if (v.noteOnTime < oldestTime)
            {
                oldestTime = v.noteOnTime;
                oldestVoice = &v;
            }
        }

        if (voiceToUse == nullptr)
            voiceToUse = oldestVoice;

        if (voiceToUse == nullptr) return;

        voiceToUse->reset();
        voiceToUse->midiNote = midiNote;
        voiceToUse->velocity = velocity;
        voiceToUse->noteOnTime = juce::Time::getMillisecondCounterHiRes();

        // Ensure params are fresh before note start
        updateParams();

        float freq = (float) juce::MidiMessage::getMidiNoteInHertz (midiNote);

        voiceToUse->osc.setWaveform (waveformIndex);
        voiceToUse->osc.setBitDepth (bitDepth);
        voiceToUse->osc.noteOn (freq, true);

        voiceToUse->ampEnv.setParameters (ampParams);
        voiceToUse->isActive.store (true);
        voiceToUse->ampEnv.noteOn();
    }

    void ChipSignalPath::noteOff (int midiNote)
    {
        for (auto& v : voices)
        {
            if (v.isActive.load() && v.midiNote == midiNote)
            {
                v.ampEnv.noteOff();
            }
        }
    }

    void ChipSignalPath::setPolyAftertouch (int midiNote, float value)
    {
        for (auto& v : voices)
            if (v.isActive.load() && v.midiNote == midiNote)
                v.aftertouch = value;
    }

    void ChipSignalPath::setChannelAftertouch (float value)
    {
        for (auto& v : voices)
            v.aftertouch = value;
    }

    // ─── Parameter polling ────────────────────────────────
    void ChipSignalPath::updateParams()
    {
        // Oscillator
        if (auto* p = registry.getParameter ("Chip Osc/Waveform"))
            waveformIndex = (int) p->getValue();
        if (auto* p = registry.getParameter ("Chip Osc/Bit Depth"))
            bitDepth = (int) p->getValue();
        if (auto* p = registry.getParameter ("Chip Osc/Volume"))
            oscVolume = p->getValue();
        else
            oscVolume = 0.8f;

        // Filter
        if (auto* p = registry.getParameter ("Filter/Filter Type"))
            filterType = (int) p->getValue();
        if (auto* p = registry.getParameter ("Filter/Cutoff"))
            filterCutoff = p->getValue();
        else
            filterCutoff = 20000.0f;
        if (auto* p = registry.getParameter ("Filter/Resonance"))
            filterRes = p->getValue();

        // Amp envelope
        float a = 0.005f, d = 0.3f, s = 1.0f, r = 0.3f;
        if (auto* p = registry.getParameter ("Amp Env/Attack"))
            a = std::max (0.001f, p->getValue() / 1000.0f);
        if (auto* p = registry.getParameter ("Amp Env/Decay"))
            d = std::max (0.001f, p->getValue() / 1000.0f);
        if (auto* p = registry.getParameter ("Amp Env/Sustain"))
            s = p->getValue(); 
        if (auto* p = registry.getParameter ("Amp Env/Release"))
            r = std::max (0.001f, p->getValue() / 1000.0f);
        ampParams = { a, d, s, r };

        // Amp
        if (auto* p = registry.getParameter ("Amp/Level"))
            ampLevel = p->getValue();
        else
            ampLevel = 0.8f;
        if (auto* p = registry.getParameter ("Amp/Velocity"))
            ampVelocity = p->getValue();
    }

    // ─── Audio callback ───────────────────────────────────
    void ChipSignalPath::getNextAudioBlock (const juce::AudioSourceChannelInfo& info)
    {
        auto* buffer = info.buffer;
        const int numSamples  = info.numSamples;
        const int startSample = info.startSample;
        const int numChannels = buffer->getNumChannels();

        if (numSamples <= 0 || buffer == nullptr) return;

        // Clear output
        buffer->clear (startSample, numSamples);

        // Poll parameters once per block
        updateParams();

        // Set up filter mode string
        auto filterMode = juce::dsp::StateVariableTPTFilterType::lowpass;
        switch (filterType)
        {
            case 0: filterMode = juce::dsp::StateVariableTPTFilterType::lowpass;  break;
            case 1: filterMode = juce::dsp::StateVariableTPTFilterType::highpass; break;
            case 2: filterMode = juce::dsp::StateVariableTPTFilterType::bandpass; break;
        }

        // Pitch-bend factor
        float pbSemis = pitchWheel * pbRange;
        float pbFactor = std::pow (2.0f, pbSemis / 12.0f);

        for (auto& v : voices)
        {
            if (!v.isActive.load()) continue;

            // Update oscillator settings each block
            v.osc.setWaveform (waveformIndex);
            v.osc.setBitDepth (bitDepth);

            // Set up filter for this voice
            float cutoffHz = std::clamp (filterCutoff, 20.0f, (float) std::max (100.0, sampleRate) * 0.49f);
            v.filter.setType (filterMode);
            v.filter.setCutoffFrequency (cutoffHz);
            v.filter.setResonance (std::clamp (filterRes, 0.0f, 2.5f));

            // Re-pitch the oscillator for pitch bend
            float baseFreq = (float) juce::MidiMessage::getMidiNoteInHertz (v.midiNote);
            float freq = baseFreq * pbFactor;

            // Velocity scaling
            float velScale = 1.0f - ampVelocity + ampVelocity * v.velocity;

            for (int i = 0; i < numSamples; ++i)
            {
                // Drive oscillator at correct frequency
                v.osc.noteOn (freq, false);  
                float sample = v.osc.process();

                // Skip filter if cutoff is fully open to guarantee signal pass
                if (filterCutoff < 19800.0f || filterType != 0)
                    sample = v.filter.processSample (0, sample);

                // Envelope
                float env = v.ampEnv.getNextSample();
                
                // Final gain stage (generous 2.0x gain)
                float gain = env * oscVolume * ampLevel * velScale * 2.0f;
                sample *= gain;

                // Mix into all available stereo channels
                for (int ch = 0; ch < numChannels; ++ch)
                    buffer->addSample (ch, startSample + i, sample);

                // Check if voice is done
                if (!v.ampEnv.isActive())
                {
                    v.isActive.store (false);
                    break;
                }
            }
        }
    }

} // namespace neon
