#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_dsp/juce_dsp.h>
#include <neon_ui_components/neon_ui_components.h>
#include <atomic>

#include "BassEngine.h"
#include "PadEngine.h"
#include "ArpEngine.h"
#include "DrumEngine.h"
#include "PatternEngine.h"

namespace neon
{
    /**
     * SplitSignalPath
     * The master audio engine for Neon Split.
     * Wraps Bass, Pad, Arp, Drum, and Pattern engines.
     * Polls the ParameterRegistry to drive the DSP.
     * Routes MIDI based on keyboard split point.
     */
    class SplitSignalPath
    {
    public:
        SplitSignalPath();
        ~SplitSignalPath() = default;

        void prepareToPlay (double sampleRate, int samplesPerBlock);
        void processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages);
        void releaseResources();

        void setBpm (double newBpm) { bpm = newBpm; }
        void setPpqPosition (double ppq) { ppqPosition = ppq; }
        void setIsPlaying (bool playing) { isPlaying = playing; }

        int getActiveVoicesCount() const;

    private:
        void updateParams();

        double currentSampleRate = 44100.0;
        int currentBlockSize = 512;
        double bpm = 120.0;
        double ppqPosition = 0.0;
        bool isPlaying = false;

        // Keyboard split
        int splitPoint = 60; // Middle C

        // Master
        float masterVolume = 0.8f;

        // Sync
        enum class SyncMode { HostSync, FreeRun };
        SyncMode syncMode = SyncMode::HostSync;

        // Voice engines
        BassEngine bassEngine;
        PadEngine padEngine;
        ArpEngine arpEngine;
        DrumEngine drumEngine;
        PatternEngine patternEngine;

        // Drum step tracking
        int lastDrumStep = -1;
    };

} // namespace neon
