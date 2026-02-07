#include "PadEngine.h"

//==============================================================================
PadEngine::PadEngine()
{
    for (auto& state : voiceStates)
    {
        state.oscPhases.fill(0.0f);
        state.modPhases.fill(0.0f);
    }
}

//==============================================================================
void PadEngine::prepare(double sr, int samplesPerBlock)
{
    VoiceBase::prepare(sr, samplesPerBlock);
    chorus.prepare(sr, samplesPerBlock);
    delay.prepare(sr, samplesPerBlock);
    reverb.prepare(sr, samplesPerBlock);
}

void PadEngine::reset()
{
    VoiceBase::reset();
    chorus.reset();
    delay.reset();
    reverb.reset();
    
    for (auto& state : voiceStates)
    {
        state.oscPhases.fill(0.0f);
        state.modPhases.fill(0.0f);
    }
}

//==============================================================================
void PadEngine::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    // Handle MIDI
    for (const auto metadata : midiMessages)
    {
        handleMidiEvent(metadata.getMessage());
    }
    
    // Render audio
    auto* leftChannel = buffer.getWritePointer(0);
    auto* rightChannel = buffer.getNumChannels() > 1 ? buffer.getWritePointer(1) : nullptr;
    
    for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
    {
        float output = renderVoices();
        
        leftChannel[sample] = output;
        if (rightChannel)
            rightChannel[sample] = output;
    }
    
    // Apply effects chain
    chorus.processBlock(buffer);
    
    if (delayEnabled)
        delay.processBlock(buffer);
    
    if (reverbEnabled)
        reverb.processBlock(buffer);
}

//==============================================================================
void PadEngine::handleMidiEvent(const juce::MidiMessage& message)
{
    if (message.isNoteOn())
    {
        // Shift down 1 octave (12 semitones)
        noteOn(message.getNoteNumber() - 12, message.getFloatVelocity());
    }
    else if (message.isNoteOff())
    {
        noteOff(message.getNoteNumber() - 12);
    }
    else if (message.isAllNotesOff() || message.isAllSoundOff())
    {
        allNotesOff();
    }
}

float PadEngine::renderVoices()
{
    float output = 0.0f;
    
    const auto& preset = PadPresets::getPreset(currentPreset);
    float phaseIncrement = 1.0f / static_cast<float>(sampleRate);
    
    for (int i = 0; i < maxVoices; ++i)
    {
        auto& voice = voices[i];
        if (!voice.active)
            continue;
        
        // Update envelope - pads have longer attack/release
        updateEnvelope(voice, preset.attackTime, preset.releaseTime);
        
        if (!voice.active)
            continue;
        
        auto& state = voiceStates[i];
        float voiceSample = 0.0f;
        
        // Pad presets use multiple detuned oscillators
        for (int osc = 0; osc < preset.numOscillators; ++osc)
        {
            float detune = preset.detuneAmounts[osc];
            float oscFreq = voice.frequency * std::pow(2.0f, detune / 1200.0f); // cents to ratio
            float freqIncrement = oscFreq * phaseIncrement;
            
            state.oscPhases[osc] += freqIncrement;
            if (state.oscPhases[osc] >= 1.0f)
                state.oscPhases[osc] -= 1.0f;
            
            // Generate based on preset waveform
            float oscSample = 0.0f;
            
            switch (preset.waveform)
            {
                case WaveformType::FM:
                {
                    state.modPhases[osc] += freqIncrement * preset.modRatio;
                    if (state.modPhases[osc] >= 1.0f)
                        state.modPhases[osc] -= 1.0f;
                    oscSample = fmOperator(state.oscPhases[osc], preset.modIndex, state.modPhases[osc]);
                    break;
                }
                    
                case WaveformType::AnalogPulse:
                    oscSample = generateSample(WaveformType::AnalogPulse, state.oscPhases[osc], preset.pulseWidth);
                    break;
                    
                case WaveformType::AnalogSaw:
                    oscSample = generateSample(WaveformType::AnalogSaw, state.oscPhases[osc]);
                    break;
                    
                case WaveformType::AnalogSine:
                    oscSample = generateSample(WaveformType::AnalogSine, state.oscPhases[osc]);
                    break;
            }
            
            voiceSample += oscSample * preset.oscLevels[osc];
        }
        
        // Normalize by number of oscillators
        voiceSample /= static_cast<float>(preset.numOscillators);
        
        // Apply envelope and velocity
        output += voiceSample * voice.envelope * voice.velocity * preset.level;
    }
    
    // Apply part volume
    return output * volume;
}

//==============================================================================
void PadEngine::setPreset(int presetIndex)
{
    currentPreset = juce::jlimit(0, getNumPresets() - 1, presetIndex);
    applyPresetParameters();
}

juce::String PadEngine::getPresetName() const
{
    return PadPresets::getPreset(currentPreset).name;
}

int PadEngine::getNumPresets() const
{
    return PadPresets::getNumPresets();
}

void PadEngine::nextPreset()
{
    int next = currentPreset + 1;
    if (next >= getNumPresets())
        next = 0;
    setPreset(next);
}

void PadEngine::previousPreset()
{
    int prev = currentPreset - 1;
    if (prev < 0)
        prev = getNumPresets() - 1;
    setPreset(prev);
}

void PadEngine::applyPresetParameters()
{
    // Presets define their own parameters internally
    // No user-adjustable synthesis parameters
}
