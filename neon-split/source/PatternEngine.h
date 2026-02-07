#pragma once

#include <JuceHeader.h>

/**
 * Pattern Engine - Handles bass and arp patterns with tempo sync
 */
class PatternEngine
{
public:
    //==============================================================================
    enum class BassPattern
    {
        Off,
        EighthDrive,
        OctaveBounce,
        SyncPulse,
        Pumping8ths,
        Staccato16ths,
        UserSequencer
    };
    
    //==============================================================================
    PatternEngine();
    ~PatternEngine() = default;
    
    //==============================================================================
    void prepare(double sampleRate);
    void reset();
    
    //==============================================================================
    // Tempo sync
    void setTempo(double bpm) { tempo = bpm; }
    void setPpqPosition(double ppq) { ppqPosition = ppq; }
    void setPlaying(bool playing) { isPlaying = playing; }
    
    //==============================================================================
    // Bass pattern
    void setBassPattern(BassPattern pattern) { bassPattern = pattern; }
    void setBassPatternEnabled(bool enabled) { bassPatternEnabled = enabled; }
    bool isBassPatternEnabled() const { return bassPatternEnabled; }
    
    //==============================================================================
    // User Sequencer
    void setSequencerStep(int step, bool active) { if (step >= 0 && step < 16) sequencerSteps[step] = active; }
    bool getSequencerStep(int step) const { return (step >= 0 && step < 16) ? sequencerSteps[step] : false; }
    
    void setSequencerStepLength(int lengthIndex) { sequencerStepLengthIndex = juce::jlimit(0, 5, lengthIndex); }
    int getSequencerStepLength() const { return sequencerStepLengthIndex; }
    static juce::StringArray getStepLengthNames() { return { "1/1", "1/2", "1/4", "1/8", "1/16", "1/32" }; }
    
    //==============================================================================
    /**
     * Process bass pattern - modifies MIDI based on pattern
     * Applies velocity/gate patterns to incoming notes
     */
    void processBassPattern(juce::MidiBuffer& midiBuffer, int numSamples);
    
    //==============================================================================
    static juce::StringArray getBassPatternNames();

private:
    //==============================================================================
    double sampleRate = 44100.0;
    double tempo = 120.0;
    double ppqPosition = 0.0;
    bool isPlaying = false;
    
    //==============================================================================
    BassPattern bassPattern = BassPattern::Off;
    bool bassPatternEnabled = false;
    
    //==============================================================================
    // User Sequencer state
    std::array<bool, 16> sequencerSteps;
    int sequencerStepLengthIndex = 4; // Default 1/16
    
    //==============================================================================
    // Pattern state
    int lastHeldNote = -1;
    double lastPatternStep = -1.0;
    bool noteCurrentlyOn = false;
    
    //==============================================================================
    // Pattern definitions
    struct PatternStep
    {
        float velocity;
        float gateLength; // 0-1, portion of step
        int octaveOffset; // semitones
    };
    
    std::vector<PatternStep> getPatternSteps(BassPattern pattern, int stepIndex);
    
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PatternEngine)
};
