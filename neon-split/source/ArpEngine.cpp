#include "ArpEngine.h"
#include <algorithm>
#include <cmath>

//==============================================================================
ArpEngine::ArpEngine()
{
    heldNotes.reserve(16);
}

//==============================================================================
void ArpEngine::prepare(double sr, int samplesPerBlock)
{
    VoiceBase::prepare(sr, samplesPerBlock);
    delay.prepare(sr, samplesPerBlock);
    reverb.prepare(sr, samplesPerBlock);

    filter.reset();
    setFilterCutoff(filterCutoff);
}

void ArpEngine::reset()
{
    VoiceBase::reset();
    delay.reset();
    reverb.reset();
    
    heldNotes.clear();
    currentArpIndex = -1;
    currentArpNote = -1;
    arpDirectionUp = true;
    currentPhase = 0.0f;
    modPhase = 0.0f;
    currentEnvelope = 0.0f;
    gateEnvelope = 0.0f;
    lastStepPpq = 0.0;
}

//==============================================================================
void ArpEngine::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages,
                             double tempo, double ppqPosition)
{
    if (!arpEnabled)
        return;
    
    // Handle MIDI
    for (const auto metadata : midiMessages)
    {
        handleMidiEvent(metadata.getMessage());
    }
    
    // Update arpeggiator timing
    updateArpeggiator(tempo, ppqPosition);
    
    // Render audio
    auto* leftChannel = buffer.getWritePointer(0);
    auto* rightChannel = buffer.getNumChannels() > 1 ? buffer.getWritePointer(1) : nullptr;
    
    for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
    {
        float output = renderCurrentNote() * volume;
        
        // Apply filter
        output = filter.processSample(output);

        leftChannel[sample] += output;
        if (rightChannel)
            rightChannel[sample] += output;
    }
    
    // Apply effects
    if (delayEnabled)
        delay.processBlock(buffer);
    
    if (reverbEnabled)
        reverb.processBlock(buffer);
}

//==============================================================================
void ArpEngine::handleMidiEvent(const juce::MidiMessage& message)
{
    if (message.isNoteOn())
    {
        int note = message.getNoteNumber();
        if (std::find(heldNotes.begin(), heldNotes.end(), note) == heldNotes.end())
        {
            heldNotes.push_back(note);
            std::sort(heldNotes.begin(), heldNotes.end());
        }
    }
    else if (message.isNoteOff())
    {
        int note = message.getNoteNumber();
        auto it = std::find(heldNotes.begin(), heldNotes.end(), note);
        if (it != heldNotes.end())
        {
            heldNotes.erase(it);
        }
        
        if (heldNotes.empty())
        {
            currentArpNote = -1;
            currentArpIndex = -1;
            arpDirectionUp = true;
        }
    }
    else if (message.isAllNotesOff() || message.isAllSoundOff())
    {
        heldNotes.clear();
        currentArpNote = -1;
        currentArpIndex = -1;
        arpDirectionUp = true;
        currentEnvelope = 0.0f;
    }
}

void ArpEngine::updateArpeggiator(double tempo, double ppqPosition)
{
    if (heldNotes.empty())
        return;
    
    // Calculate step position (16th notes = 0.25 beats)
    double currentStep = std::floor(ppqPosition / stepLength);
    double lastStep = std::floor(lastPpqPosition / stepLength);
    
    if (currentStep != lastStep || currentArpNote == -1)
    {
        // Advance to next note
        currentArpNote = getNextArpNote();
        if (currentArpNote >= 0)
        {
            currentFrequency = midiNoteToFrequency(currentArpNote);
            gateEnvelope = 1.0f;
        }
        lastStepPpq = ppqPosition;
    }
    
    // Gate envelope for synthwave pattern
    if (currentPattern == ArpPattern::SynthwaveGate)
    {
        double posInStep = std::fmod(ppqPosition, stepLength) / stepLength;
        gateEnvelope = posInStep < 0.7f ? 1.0f : 0.0f; // 70% gate
    }
    
    lastPpqPosition = ppqPosition;
}

int ArpEngine::getNextArpNote()
{
    if (heldNotes.empty())
        return -1;
    
    int note = -1;
    
    switch (currentPattern)
    {
        case ArpPattern::Up:
            currentArpIndex = (currentArpIndex + 1) % static_cast<int>(heldNotes.size());
            note = heldNotes[currentArpIndex];
            break;
            
        case ArpPattern::Down:
            currentArpIndex--;
            if (currentArpIndex < 0)
                currentArpIndex = static_cast<int>(heldNotes.size()) - 1;
            note = heldNotes[currentArpIndex];
            break;
            
        case ArpPattern::UpDown:
            if (heldNotes.size() == 1)
            {
                note = heldNotes[0];
            }
            else
            {
                if (arpDirectionUp)
                {
                    currentArpIndex++;
                    if (currentArpIndex >= static_cast<int>(heldNotes.size()))
                    {
                        currentArpIndex = static_cast<int>(heldNotes.size()) - 2;
                        arpDirectionUp = false;
                    }
                }
                else
                {
                    currentArpIndex--;
                    if (currentArpIndex < 0)
                    {
                        currentArpIndex = 1;
                        arpDirectionUp = true;
                    }
                }
                currentArpIndex = juce::jlimit(0, static_cast<int>(heldNotes.size()) - 1, currentArpIndex);
                note = heldNotes[currentArpIndex];
            }
            break;
            
        case ArpPattern::SynthwaveGate:
            // Play all notes simultaneously with gate
            if (!heldNotes.empty())
                note = heldNotes[currentArpIndex % heldNotes.size()];
            currentArpIndex = (currentArpIndex + 1) % static_cast<int>(heldNotes.size());
            break;

        case ArpPattern::Random:
            if (!heldNotes.empty())
            {
                currentArpIndex = juce::Random::getSystemRandom().nextInt(static_cast<int>(heldNotes.size()));
                note = heldNotes[currentArpIndex];
            }
            break;
    }
    
    return note;
}

float ArpEngine::renderCurrentNote()
{
    if (currentArpNote < 0 || heldNotes.empty())
    {
        // Fade out
        currentEnvelope *= 0.999f;
        if (currentEnvelope < 0.001f)
            return 0.0f;
    }
    else
    {
        // Quick attack
        currentEnvelope += (1.0f - currentEnvelope) * 0.01f;
    }
    
    if (currentFrequency <= 0.0f)
        return 0.0f;
    
    float phaseIncrement = currentFrequency / static_cast<float>(sampleRate);
    currentPhase += phaseIncrement;
    if (currentPhase >= 1.0f)
        currentPhase -= 1.0f;
    
    // Generate waveform
    float sample = 0.0f;
    
    switch (currentWaveform)
    {
        case WaveformType::FM:
        {
            modPhase += phaseIncrement * 2.0f;
            if (modPhase >= 1.0f)
                modPhase -= 1.0f;
            sample = fmOperator(currentPhase, 3.0f, modPhase);
            break;
        }
            
        case WaveformType::AnalogPulse:
            sample = generateSample(WaveformType::AnalogPulse, currentPhase, 0.5f);
            break;
            
        case WaveformType::AnalogSaw:
            sample = generateSample(WaveformType::AnalogSaw, currentPhase);
            break;
            
        case WaveformType::AnalogSine:
            sample = generateSample(WaveformType::AnalogSine, currentPhase);
            break;
    }
    
    // Apply gate envelope for synthwave pattern
    float envMod = (currentPattern == ArpPattern::SynthwaveGate) ? gateEnvelope : 1.0f;
    
    return sample * currentEnvelope * envMod * 0.4f; // Lower level to blend with pad
}

//==============================================================================
void ArpEngine::setWaveform(int waveformIndex)
{
    currentWaveform = static_cast<WaveformType>(juce::jlimit(0, 3, waveformIndex));
}

juce::String ArpEngine::getWaveformName() const
{
    switch (currentWaveform)
    {
        case WaveformType::FM: return "FM";
        case WaveformType::AnalogPulse: return "Analog Pulse";
        case WaveformType::AnalogSaw: return "Analog Saw";
        case WaveformType::AnalogSine: return "Analog Sine";
        default: return "Unknown";
    }
}

void ArpEngine::setPattern(int patternIndex)
{
    currentPattern = static_cast<ArpPattern>(juce::jlimit(0, 4, patternIndex));
}

juce::String ArpEngine::getPatternName() const
{
    switch (currentPattern)
    {
        case ArpPattern::Up: return "Up";
        case ArpPattern::Down: return "Down";
        case ArpPattern::UpDown: return "Up/Down";
        case ArpPattern::SynthwaveGate: return "Synthwave Gate";
        case ArpPattern::Random: return "Random";
        default: return "Unknown";
    }
}

void ArpEngine::setFilterCutoff(float hz)
{
    filterCutoff = juce::jlimit(20.0f, 20000.0f, hz);
    float q = resonanceEnabled ? 1.2f : 0.7071f; // Slight resonance bump if enabled
    filter.coefficients = juce::dsp::IIR::Coefficients<float>::makeLowPass(sampleRate, filterCutoff, q);
}

void ArpEngine::setResonanceEnabled(bool enabled)
{
    resonanceEnabled = enabled;
    setFilterCutoff(filterCutoff);
}
