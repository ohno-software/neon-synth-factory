#pragma once

#include <JuceHeader.h>
#include <array>

/**
 * Base class for all NeonSplit voice engines
 * Provides common voice management and synthesis infrastructure
 */
class VoiceBase
{
public:
    //==============================================================================
    enum class WaveformType
    {
        FM,
        AnalogPulse,
        AnalogSaw,
        AnalogSine
    };
    
    //==============================================================================
    VoiceBase() = default;
    virtual ~VoiceBase() = default;
    
    //==============================================================================
    virtual void prepare(double sampleRate, int samplesPerBlock);
    virtual void reset();
    
    //==============================================================================
    virtual void noteOn(int noteNumber, float velocity);
    virtual void noteOff(int noteNumber);
    virtual void allNotesOff();
    
    //==============================================================================
    float generateSample(WaveformType waveform, float phase, float pulseWidth = 0.5f);
    static float midiNoteToFrequency(int noteNumber);
    
protected:
    //==============================================================================
    struct Voice
    {
        int noteNumber = -1;
        float frequency = 0.0f;
        float phase = 0.0f;
        float velocity = 0.0f;
        float envelope = 0.0f;
        bool active = false;
        bool releasing = false;
        float releasePhase = 0.0f;
    };
    
    //==============================================================================
    static constexpr int maxVoices = 8;
    std::array<Voice, maxVoices> voices;
    
    double sampleRate = 44100.0;
    int blockSize = 512;
    
    //==============================================================================
    Voice* findFreeVoice();
    Voice* findVoiceForNote(int noteNumber);
    void updateEnvelope(Voice& voice, float attackTime, float releaseTime);
    
    //==============================================================================
    // FM synthesis helpers
    float fmOperator(float phase, float modIndex, float modPhase);
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VoiceBase)
};
