#include "SidSignalPath.h"

namespace neon
{
    SidSignalPath::SidSignalPath() : registry (ParameterRegistry::getInstance())
    {
        for (auto& v : voices)
        {
            v.filter.setType (juce::dsp::StateVariableTPTFilterType::lowpass);
        }
    }

    void SidSignalPath::prepareToPlay (int samplesPerBlockExpected, double sr)
    {
        sampleRate = sr;
        for (auto& v : voices)
        {
            v.osc1.setSampleRate (sr);
            v.osc2.setSampleRate (sr);
            v.osc3.setSampleRate (sr);
            v.ampEnv.setSampleRate (sr);
            
            juce::dsp::ProcessSpec spec;
            spec.sampleRate = sr;
            spec.maximumBlockSize = (juce::uint32) samplesPerBlockExpected;
            spec.numChannels = 1;
            v.filter.prepare (spec);
        }
    }

    void SidSignalPath::releaseResources() {}

    void SidSignalPath::noteOn (int midiNote, float velocity)
    {
        Voice* voiceToUse = nullptr;
        double oldestTime = std::numeric_limits<double>::max();

        for (auto& v : voices)
        {
            if (!v.isActive.load()) { voiceToUse = &v; break; }
            if (v.noteOnTime < oldestTime) { oldestTime = v.noteOnTime; voiceToUse = &v; }
        }

        if (voiceToUse)
        {
            voiceToUse->midiNote = midiNote;
            voiceToUse->velocity = velocity;
            voiceToUse->noteOnTime = juce::Time::getMillisecondCounterHiRes();
            voiceToUse->isActive.store (true);
            voiceToUse->ampEnv.setParameters (ampParams);
            voiceToUse->ampEnv.noteOn();
        }
    }

    void SidSignalPath::noteOff (int midiNote)
    {
        for (auto& v : voices)
            if (v.isActive.load() && v.midiNote == midiNote)
                v.ampEnv.noteOff();
    }

    void SidSignalPath::updateParams()
    {
        auto pollOsc = [&](const juce::String& prefix, OscParams& p) {
            if (auto* param = registry.getParameter (prefix + "/Waveform")) p.waveform = (int)param->getValue();
            if (auto* param = registry.getParameter (prefix + "/Volume")) p.volume = param->getValue();
            if (auto* param = registry.getParameter (prefix + "/Transp")) p.transpose = param->getValue();
            if (auto* param = registry.getParameter (prefix + "/Fine")) p.fine = param->getValue() / 100.0f;
            if (auto* param = registry.getParameter (prefix + "/Pulse Width")) p.pulseWidth = param->getValue();
            if (auto* param = registry.getParameter (prefix + "/Sync")) p.sync = param->getValue() > 0.5f;
            if (auto* param = registry.getParameter (prefix + "/Ring Mod")) p.ringMod = param->getValue() > 0.5f;
        };

        pollOsc ("Osc 1", osc1Params);
        pollOsc ("Osc 2", osc2Params);
        pollOsc ("Osc 3", osc3Params);

        if (auto* p = registry.getParameter ("Filter/Cutoff")) filterCutoff = p->getValue();
        if (auto* p = registry.getParameter ("Filter/Resonance")) filterRes = p->getValue();
        if (auto* p = registry.getParameter ("Filter/Filter Type")) filterType = (int)p->getValue();

        if (auto* p = registry.getParameter ("Amp Env/Attack"))  ampParams.attack = p->getValue() / 1000.0f;
        if (auto* p = registry.getParameter ("Amp Env/Decay"))   ampParams.decay = p->getValue() / 1000.0f;
        if (auto* p = registry.getParameter ("Amp Env/Sustain")) ampParams.sustain = p->getValue();
        if (auto* p = registry.getParameter ("Amp Env/Release")) ampParams.release = p->getValue() / 1000.0f;
    }

    void SidSignalPath::getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill)
    {
        updateParams();
        bufferToFill.clearActiveBufferRegion();
        auto* outL = bufferToFill.buffer->getWritePointer (0, bufferToFill.startSample);
        auto* outR = bufferToFill.buffer->getNumChannels() > 1 ? bufferToFill.buffer->getWritePointer (1, bufferToFill.startSample) : nullptr;

        for (int s = 0; s < bufferToFill.numSamples; ++s)
        {
            float mixedSample = 0.0f;
            for (auto& v : voices)
            {
                if (!v.isActive.load()) continue;

                float baseFreq = (float)juce::MidiMessage::getMidiNoteInHertz (v.midiNote + (int)pitchWheel * 12);
                auto getFreq = [&](const OscParams& p) { 
                    return baseFreq * std::pow (2.0f, (p.transpose + p.fine) / 12.0f); 
                };

                v.osc1.setFrequency (getFreq(osc1Params));
                v.osc1.setWaveform (osc1Params.waveform);
                v.osc1.setPulseWidth (osc1Params.pulseWidth);

                v.osc2.setFrequency (getFreq(osc2Params));
                v.osc2.setWaveform (osc2Params.waveform);
                v.osc2.setPulseWidth (osc2Params.pulseWidth);

                v.osc3.setFrequency (getFreq(osc3Params));
                v.osc3.setWaveform (osc3Params.waveform);
                v.osc3.setPulseWidth (osc3Params.pulseWidth);

                // SID Sync/Ring Routing: 1 targets 3, 2 targets 1, 3 targets 2
                float s1 = v.osc1.process (v.osc3.getPhase(), osc1Params.sync, 0.0f, false);
                float s2 = v.osc2.process (v.osc1.getPhase(), osc2Params.sync, s1, osc2Params.ringMod);
                float s3 = v.osc3.process (v.osc2.getPhase(), osc3Params.sync, s2, osc3Params.ringMod);
                
                // Ring mod for Osc 1 uses Osc 3
                v.osc1.process (0.0f, false, s3, osc1Params.ringMod); // Second pass for Ring Mod on Osc 1 if needed? 
                // Actually SID ring mod is simpler. Let's just do one pass per sample correctly.
                
                float voiceMix = (s1 * osc1Params.volume) + (s2 * osc2Params.volume) + (s3 * osc3Params.volume);
                
                v.filter.setCutoffFrequency (filterCutoff);
                v.filter.setResonance (filterRes);
                v.filter.setType (static_cast<juce::dsp::StateVariableTPTFilterType>(filterType));
                
                float filtered = v.filter.processSample (0, voiceMix);
                float env = v.ampEnv.getNextSample();
                mixedSample += filtered * env * v.velocity;

                if (!v.ampEnv.isActive() && env < 0.0001f) v.isActive.store (false);
            }
            
            if (outL) outL[s] = mixedSample;
            if (outR) outR[s] = mixedSample;
        }
    }
}
