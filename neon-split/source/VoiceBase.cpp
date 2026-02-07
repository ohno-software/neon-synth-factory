#include "VoiceBase.h"

//==============================================================================
void VoiceBase::prepare(double sr, int samplesPerBlock)
{
    sampleRate = sr;
    blockSize = samplesPerBlock;
    reset();
}

void VoiceBase::reset()
{
    for (auto& voice : voices)
    {
        voice.noteNumber = -1;
        voice.frequency = 0.0f;
        voice.phase = 0.0f;
        voice.velocity = 0.0f;
        voice.envelope = 0.0f;
        voice.active = false;
        voice.releasing = false;
        voice.releasePhase = 0.0f;
    }
}

//==============================================================================
void VoiceBase::noteOn(int noteNumber, float velocity)
{
    auto* voice = findVoiceForNote(noteNumber);
    if (voice == nullptr)
        voice = findFreeVoice();
    
    if (voice != nullptr)
    {
        voice->noteNumber = noteNumber;
        voice->frequency = midiNoteToFrequency(noteNumber);
        voice->velocity = velocity;
        voice->active = true;
        voice->releasing = false;
        voice->releasePhase = 0.0f;
        // Don't reset phase for smooth transitions
    }
}

void VoiceBase::noteOff(int noteNumber)
{
    auto* voice = findVoiceForNote(noteNumber);
    if (voice != nullptr)
    {
        voice->releasing = true;
        voice->releasePhase = 0.0f;
    }
}

void VoiceBase::allNotesOff()
{
    for (auto& voice : voices)
    {
        voice.releasing = true;
        voice.releasePhase = 0.0f;
    }
}

//==============================================================================
float VoiceBase::generateSample(WaveformType waveform, float phase, float pulseWidth)
{
    switch (waveform)
    {
        case WaveformType::AnalogSine:
            return std::sin(phase * juce::MathConstants<float>::twoPi);
            
        case WaveformType::AnalogSaw:
        {
            // Band-limited saw approximation
            float saw = 2.0f * phase - 1.0f;
            // Simple anti-aliasing
            return saw - 0.5f * saw * saw * saw;
        }
        
        case WaveformType::AnalogPulse:
        {
            // Pulse wave with variable width
            return phase < pulseWidth ? 1.0f : -1.0f;
        }
        
        case WaveformType::FM:
        {
            // Simple 2-op FM
            float modulator = std::sin(phase * juce::MathConstants<float>::twoPi * 2.0f);
            return std::sin(phase * juce::MathConstants<float>::twoPi + modulator * 2.0f);
        }
        
        default:
            return 0.0f;
    }
}

float VoiceBase::midiNoteToFrequency(int noteNumber)
{
    return 440.0f * std::pow(2.0f, (noteNumber - 69) / 12.0f);
}

//==============================================================================
VoiceBase::Voice* VoiceBase::findFreeVoice()
{
    // First, look for inactive voice
    for (auto& voice : voices)
    {
        if (!voice.active)
            return &voice;
    }
    
    // Steal oldest releasing voice
    Voice* oldest = nullptr;
    float maxReleasePhase = -1.0f;
    
    for (auto& voice : voices)
    {
        if (voice.releasing && voice.releasePhase > maxReleasePhase)
        {
            maxReleasePhase = voice.releasePhase;
            oldest = &voice;
        }
    }
    
    if (oldest != nullptr)
        return oldest;
    
    // Steal first voice as last resort
    return &voices[0];
}

VoiceBase::Voice* VoiceBase::findVoiceForNote(int noteNumber)
{
    for (auto& voice : voices)
    {
        if (voice.active && voice.noteNumber == noteNumber)
            return &voice;
    }
    return nullptr;
}

void VoiceBase::updateEnvelope(Voice& voice, float attackTime, float releaseTime)
{
    if (voice.releasing)
    {
        // Release phase
        float releaseIncrement = 1.0f / (releaseTime * static_cast<float>(sampleRate));
        voice.releasePhase += releaseIncrement;
        voice.envelope = std::max(0.0f, 1.0f - voice.releasePhase);
        
        if (voice.envelope <= 0.0f)
        {
            voice.active = false;
            voice.releasing = false;
        }
    }
    else
    {
        // Attack phase
        float attackIncrement = 1.0f / (attackTime * static_cast<float>(sampleRate));
        voice.envelope = std::min(1.0f, voice.envelope + attackIncrement);
    }
}

float VoiceBase::fmOperator(float phase, float modIndex, float modPhase)
{
    float modulator = std::sin(modPhase * juce::MathConstants<float>::twoPi);
    return std::sin(phase * juce::MathConstants<float>::twoPi + modulator * modIndex);
}
