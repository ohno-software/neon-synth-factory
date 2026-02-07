#include "SplitSignalPath.h"

namespace neon
{
    SplitSignalPath::SplitSignalPath()
    {
    }

    void SplitSignalPath::prepareToPlay (double sampleRate, int samplesPerBlock)
    {
        currentSampleRate = sampleRate;
        currentBlockSize = samplesPerBlock;

        bassEngine.prepare (sampleRate, samplesPerBlock);
        padEngine.prepare (sampleRate, samplesPerBlock);
        arpEngine.prepare (sampleRate, samplesPerBlock);
        drumEngine.prepare (sampleRate, samplesPerBlock);
        patternEngine.prepare (sampleRate);
    }

    void SplitSignalPath::releaseResources()
    {
        bassEngine.reset();
        padEngine.reset();
        arpEngine.reset();
        patternEngine.reset();
    }

    int SplitSignalPath::getActiveVoicesCount() const
    {
        return 0;
    }

    void SplitSignalPath::updateParams()
    {
        auto& reg = ParameterRegistry::getInstance();

        auto getVal = [&](const juce::String& path, float fallback) -> float {
            if (auto* p = reg.getParameter (path))
                return p->getValue();
            return fallback;
        };

        auto getInt = [&](const juce::String& path, int fallback) -> int {
            if (auto* p = reg.getParameter (path))
                return static_cast<int> (std::round (p->getValue()));
            return fallback;
        };

        // ===== GLOBAL =====
        splitPoint = getInt ("Split/Split Point", 60);
        masterVolume = getVal ("Split/Master Vol", 0.8f);

        int syncIdx = getInt ("Split/Sync Mode", 0);
        syncMode = (syncIdx == 0) ? SyncMode::HostSync : SyncMode::FreeRun;

        // ===== BASS =====
        bassEngine.setPreset (getInt ("Bass/Preset", 0));
        bassEngine.setLPFCutoff (getVal ("Bass/LPF", 20000.0f));
        bassEngine.setVolume (getVal ("Bass/Volume", 0.8f));
        bassEngine.setDelayEnabled (getVal ("Bass/Delay", 0.0f) > 0.5f);
        bassEngine.setDelayTime (getInt ("Bass/Dly Time", 1));
        bassEngine.setDelayMix (getVal ("Bass/Dly Mix", 0.3f));
        bassEngine.setTempo (bpm);

        patternEngine.setBassPatternEnabled (getVal ("Bass/Pattern On", 0.0f) > 0.5f);
        patternEngine.setBassPattern (static_cast<PatternEngine::BassPattern> (getInt ("Bass/Pattern", 0)));
        patternEngine.setSequencerStepLength (getInt ("Bass/Step Len", 4));

        for (int i = 0; i < 16; ++i)
        {
            bool stepOn = getVal ("Bass/Step " + juce::String (i + 1), 1.0f) > 0.5f;
            patternEngine.setSequencerStep (i, stepOn);
        }

        // ===== PAD =====
        padEngine.setPreset (getInt ("Pad/Preset", 0));
        padEngine.setChorusType (getInt ("Pad/Chorus", 0));
        padEngine.setChorusMix (getVal ("Pad/Chorus Mix", 0.5f));
        padEngine.setVolume (getVal ("Pad/Volume", 0.8f));
        padEngine.setDelayEnabled (getVal ("Pad/Delay", 0.0f) > 0.5f);
        padEngine.setDelayTime (getInt ("Pad/Dly Time", 2));
        padEngine.setDelayMix (getVal ("Pad/Dly Mix", 0.25f));
        padEngine.setReverbEnabled (getVal ("Pad/Reverb", 0.0f) > 0.5f);
        padEngine.setReverbTime (getVal ("Pad/Rvb Time", 2.5f));
        padEngine.setReverbMix (getVal ("Pad/Rvb Mix", 0.35f));

        // ===== ARP =====
        bool arpOn = getVal ("Arp/Arp On", 0.0f) > 0.5f;
        arpEngine.setEnabled (arpOn);
        if (arpOn)
        {
            arpEngine.setWaveform (getInt ("Arp/Waveform", 2));
            arpEngine.setPattern (getInt ("Arp/Pattern", 0));
            arpEngine.setFilterCutoff (getVal ("Arp/Filter", 20000.0f));
            arpEngine.setResonanceEnabled (getVal ("Arp/Resonance", 0.0f) > 0.5f);
            arpEngine.setVolume (getVal ("Arp/Volume", 0.7f));
            arpEngine.setDelayEnabled (getVal ("Arp/Delay", 0.0f) > 0.5f);
            arpEngine.setDelayTime (getInt ("Arp/Dly Time", 1));
            arpEngine.setDelayMix (getVal ("Arp/Dly Mix", 0.3f));
            arpEngine.setReverbEnabled (getVal ("Arp/Reverb", 0.0f) > 0.5f);
            arpEngine.setReverbTime (getVal ("Arp/Rvb Time", 1.5f));
            arpEngine.setReverbMix (getVal ("Arp/Rvb Mix", 0.25f));
        }

        // ===== DRUMS =====
        bool drumOn = getVal ("Drums/Drum On", 0.0f) > 0.5f;
        drumEngine.setEnabled (drumOn);
        if (drumOn)
        {
            drumEngine.setHiHatTone (getVal ("Drums/HH Tone", 5000.0f));
            drumEngine.setSnareReverbMix (getVal ("Drums/Snare Rev", 0.3f));
        }
    }

    void SplitSignalPath::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
    {
        int numSamples = buffer.getNumSamples();
        int numChannels = buffer.getNumChannels();

        // Poll parameters from the registry
        updateParams();

        // Split MIDI by zone: bass (below split) vs upper (at/above split)
        juce::MidiBuffer bassMidi, upperMidi;

        for (const auto metadata : midiMessages)
        {
            auto msg = metadata.getMessage();
            int samplePos = metadata.samplePosition;

            if (msg.isNoteOnOrOff())
            {
                if (msg.getNoteNumber() < splitPoint)
                    bassMidi.addEvent (msg, samplePos);
                else
                    upperMidi.addEvent (msg, samplePos);
            }
            else if (msg.isAllNotesOff() || msg.isAllSoundOff())
            {
                bassMidi.addEvent (msg, samplePos);
                upperMidi.addEvent (msg, samplePos);
            }
            else
            {
                // Controllers, pitch wheel etc. go to all zones
                bassMidi.addEvent (msg, samplePos);
                upperMidi.addEvent (msg, samplePos);
            }
        }

        // Update tempo/transport
        double currentTempo = (syncMode == SyncMode::HostSync) ? bpm : 120.0;
        patternEngine.setTempo (currentTempo);
        patternEngine.setPpqPosition (ppqPosition);
        patternEngine.setPlaying (isPlaying);

        // --- Bass engine ---
        juce::AudioBuffer<float> bassBuffer (numChannels, numSamples);
        bassBuffer.clear();
        patternEngine.processBassPattern (bassMidi, numSamples);
        bassEngine.processBlock (bassBuffer, bassMidi);

        // --- Pad engine ---
        juce::AudioBuffer<float> padBuffer (numChannels, numSamples);
        padBuffer.clear();
        padEngine.processBlock (padBuffer, upperMidi);

        // --- Arp engine ---
        juce::AudioBuffer<float> arpBuffer (numChannels, numSamples);
        arpBuffer.clear();
        if (arpEngine.isEnabled())
            arpEngine.processBlock (arpBuffer, upperMidi, currentTempo, ppqPosition);

        // --- Drum machine ---
        juce::AudioBuffer<float> drumBuffer (numChannels, numSamples);
        drumBuffer.clear();

        auto& reg = ParameterRegistry::getInstance();

        if (drumEngine.getEnabled() && isPlaying)
        {
            int currentStep = static_cast<int> (std::floor (ppqPosition * 4.0)) % 16;
            if (currentStep != lastDrumStep)
            {
                auto getStepVal = [&](const juce::String& path) -> bool {
                    if (auto* p = reg.getParameter (path))
                        return p->getValue() > 0.5f;
                    return false;
                };

                if (getStepVal ("Drums/K " + juce::String (currentStep + 1)))
                    drumEngine.triggerKick (0.8f);
                if (getStepVal ("Drums/S " + juce::String (currentStep + 1)))
                    drumEngine.triggerSnare (0.7f);
                if (getStepVal ("Drums/H " + juce::String (currentStep + 1)))
                    drumEngine.triggerHiHat (0.6f);

                lastDrumStep = currentStep;
            }
        }
        else if (!isPlaying)
        {
            lastDrumStep = -1;
        }

        if (drumEngine.getEnabled())
            drumEngine.processBlock (drumBuffer);

        // --- Mix all engines into output ---
        buffer.clear();
        for (int ch = 0; ch < numChannels; ++ch)
        {
            buffer.addFrom (ch, 0, bassBuffer, ch, 0, numSamples, masterVolume);
            buffer.addFrom (ch, 0, padBuffer, ch, 0, numSamples, masterVolume);
            buffer.addFrom (ch, 0, arpBuffer, ch, 0, numSamples, masterVolume);
            buffer.addFrom (ch, 0, drumBuffer, ch, 0, numSamples, masterVolume);
        }
    }

} // namespace neon
