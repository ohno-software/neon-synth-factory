#include "BassEngine.h"

//==============================================================================
BassEngine::BassEngine()
{
    modPhases.fill(0.0f);
    osc2Phases.fill(0.0f);
    subPhases.fill(0.0f);
    filterEnvs.fill(0.0f);
    for (auto& state : lpfStates)
    {
        state.z1 = 0.0f;
        state.z2 = 0.0f;
    }
}

//==============================================================================
void BassEngine::prepare(double sr, int samplesPerBlock)
{
    VoiceBase::prepare(sr, samplesPerBlock);
    delay.prepare(sr, samplesPerBlock);
    
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sr;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = 2;
    lpFilter.prepare(spec);
    setLPFCutoff(lpfCutoff);
}

void BassEngine::reset()
{
    VoiceBase::reset();
    lpFilter.reset();
    delay.reset();
    modPhases.fill(0.0f);
    osc2Phases.fill(0.0f);
    subPhases.fill(0.0f);
    filterEnvs.fill(0.0f);
    for (auto& state : lpfStates)
    {
        state.z1 = 0.0f;
        state.z2 = 0.0f;
    }
}

//==============================================================================
// Monophonic note on - cuts off any previous notes immediately
void BassEngine::noteOn(int noteNumber, float velocity)
{
    // Kill all active voices first (monophonic behavior)
    for (auto& voice : voices)
    {
        voice.active = false;
        voice.releasing = false;
    }
    
    // Reset filter state for voice 0 to avoid clicks
    lpfStates[0].z1 = 0.0f;
    lpfStates[0].z2 = 0.0f;
    filterEnvs[0] = 0.0f;
    
    // Start new note on voice 0
    voices[0].noteNumber = noteNumber;
    // Shift down 1 octave (12 semitones)
    voices[0].frequency = midiNoteToFrequency(noteNumber - 12);
    voices[0].velocity = velocity;
    voices[0].active = true;
    voices[0].releasing = false;
    voices[0].releasePhase = 0.0f;
    voices[0].phase = 0.0f;  // Reset phase for consistent attack
    osc2Phases[0] = 0.0f;
    subPhases[0] = 0.0f;
    modPhases[0] = 0.0f;
}

//==============================================================================
void BassEngine::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
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
    
    // Apply effects
    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);
    lpFilter.process(context);
    
    if (delayEnabled)
        delay.processBlock(buffer);
}

void BassEngine::setLPFCutoff(float hz)
{
    lpfCutoff = juce::jlimit(20.0f, 20000.0f, hz);
    lpFilter.state = juce::dsp::IIR::Coefficients<float>::makeLowPass(sampleRate, lpfCutoff, 0.7071f);
}

//==============================================================================
void BassEngine::handleMidiEvent(const juce::MidiMessage& message)
{
    if (message.isNoteOn())
    {
        noteOn(message.getNoteNumber(), message.getFloatVelocity());
    }
    else if (message.isNoteOff())
    {
        noteOff(message.getNoteNumber());
    }
    else if (message.isAllNotesOff() || message.isAllSoundOff())
    {
        allNotesOff();
    }
}

float BassEngine::renderVoices()
{
    float output = 0.0f;
    
    const auto& preset = BassPresets::getPreset(currentPreset);
    float phaseIncrement = 1.0f / static_cast<float>(sampleRate);
    
    // Convert detune from cents to frequency ratio
    float detuneRatio = std::pow(2.0f, preset.osc2Detune / 1200.0f);
    
    for (int i = 0; i < maxVoices; ++i)
    {
        auto& voice = voices[i];
        if (!voice.active)
            continue;
        
        // Update amplitude envelope
        updateEnvelope(voice, preset.attackTime, preset.releaseTime);
        
        // Update filter envelope (faster attack, follows amp env)
        if (!voice.releasing)
        {
            // Attack phase - quick rise (increased for snappier response)
            filterEnvs[i] += (1.0f - filterEnvs[i]) * (100.0f / static_cast<float>(sampleRate));
        }
        else
        {
            // Release phase - follow amp envelope
            filterEnvs[i] = voice.envelope;
        }
        
        if (!voice.active)
            continue;
        
        // Calculate frequency increments
        float freqIncrement = voice.frequency * phaseIncrement;
        float osc2FreqIncrement = voice.frequency * detuneRatio * phaseIncrement;
        float subFreqIncrement = voice.frequency * 0.5f * phaseIncrement;  // 1 octave down
        
        // Update oscillator phases
        voice.phase += freqIncrement;
        if (voice.phase >= 1.0f) voice.phase -= 1.0f;
        
        osc2Phases[i] += osc2FreqIncrement;
        if (osc2Phases[i] >= 1.0f) osc2Phases[i] -= 1.0f;
        
        subPhases[i] += subFreqIncrement;
        if (subPhases[i] >= 1.0f) subPhases[i] -= 1.0f;
        
        // Update modulator phase for FM
        modPhases[i] += freqIncrement * preset.modRatio;
        if (modPhases[i] >= 1.0f) modPhases[i] -= 1.0f;
        
        // Generate samples from all oscillators
        float sample = 0.0f;
        
        if (preset.waveform == WaveformType::FM)
        {
            // FM synthesis mode
            sample = fmOperator(voice.phase, preset.modIndex, modPhases[i]);
            
            // Still add sub if specified
            if (preset.subOscLevel > 0.0f)
            {
                float subSample = std::sin(subPhases[i] * juce::MathConstants<float>::twoPi);
                sample = sample * (1.0f - preset.subOscLevel) + subSample * preset.subOscLevel;
            }
        }
        else
        {
            // 3-oscillator analog mode
            // Osc 1: Main oscillator
            float osc1 = generateSample(preset.waveform, voice.phase, preset.pulseWidth);
            
            // Osc 2: Detuned oscillator (same waveform)
            float osc2 = 0.0f;
            if (preset.osc2Level > 0.0f)
            {
                osc2 = generateSample(preset.waveform, osc2Phases[i], preset.pulseWidth);
            }
            
            // Osc 3: Sub oscillator (sine, 1 octave down)
            float subOsc = 0.0f;
            if (preset.subOscLevel > 0.0f)
            {
                subOsc = std::sin(subPhases[i] * juce::MathConstants<float>::twoPi);
            }
            
            // Mix oscillators
            float osc1Level = 1.0f;
            float totalLevel = osc1Level + preset.osc2Level + preset.subOscLevel;
            
            sample = (osc1 * osc1Level + osc2 * preset.osc2Level + subOsc * preset.subOscLevel) / totalLevel;
        }
        
        // Apply resonant lowpass filter
        float filterEnvMod = filterEnvs[i] * preset.filterEnvAmount;
        float dynamicCutoff = preset.filterCutoff + filterEnvMod;
        dynamicCutoff = juce::jlimit(0.0f, 1.0f, dynamicCutoff);
        
        sample = processLPF(sample, dynamicCutoff, preset.filterResonance, lpfStates[i]);
        
        // Apply envelope and velocity
        output += sample * voice.envelope * voice.velocity * preset.level;
    }
    
    // Apply part volume
    return output * volume;
}

//==============================================================================
// Simple 2-pole resonant lowpass filter (attempt at Moog-style response)
float BassEngine::processLPF(float input, float cutoff, float resonance, LPFState& state)
{
    // Convert normalized cutoff to filter coefficient
    // Map 0-1 to roughly 20Hz - 20kHz in a musically useful curve
    float freq = 20.0f * std::pow(1000.0f, cutoff);
    float w = 2.0f * juce::MathConstants<float>::pi * freq / static_cast<float>(sampleRate);
    float g = std::tan(w * 0.5f);
    
    // Resonance (0-1 maps to Q of 0.5 to 20)
    float Q = 0.5f + resonance * 19.5f;
    float k = 1.0f / Q;
    
    // 2-pole SVF coefficients
    float a1 = 1.0f / (1.0f + g * (g + k));
    float a2 = g * a1;
    float a3 = g * a2;
    
    // Process
    float v3 = input - state.z2;
    float v1 = a1 * state.z1 + a2 * v3;
    float v2 = state.z2 + a2 * state.z1 + a3 * v3;
    
    // Update state
    state.z1 = 2.0f * v1 - state.z1;
    state.z2 = 2.0f * v2 - state.z2;
    
    // Return lowpass output
    return v2;
}

//==============================================================================
void BassEngine::setPreset(int presetIndex)
{
    currentPreset = juce::jlimit(0, getNumPresets() - 1, presetIndex);
    applyPresetParameters();
}

juce::String BassEngine::getPresetName() const
{
    return BassPresets::getPreset(currentPreset).name;
}

int BassEngine::getNumPresets() const
{
    return BassPresets::getNumPresets();
}

void BassEngine::nextPreset()
{
    int next = currentPreset + 1;
    if (next >= getNumPresets())
        next = 0;
    setPreset(next);
}

void BassEngine::previousPreset()
{
    int prev = currentPreset - 1;
    if (prev < 0)
        prev = getNumPresets() - 1;
    setPreset(prev);
}

void BassEngine::setPattern(int patternIndex)
{
    currentPattern = juce::jlimit(0, 5, patternIndex);
}

void BassEngine::applyPresetParameters()
{
    // Presets define their own parameters internally
    // No user-adjustable synthesis parameters
}
