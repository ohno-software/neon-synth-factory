#include "PatternEngine.h"

//==============================================================================
PatternEngine::PatternEngine()
{
    sequencerSteps.fill(true);
}

//==============================================================================
void PatternEngine::prepare(double sr)
{
    sampleRate = sr;
    reset();
}

void PatternEngine::reset()
{
    lastHeldNote = -1;
    lastPatternStep = -1.0;
    noteCurrentlyOn = false;
}

//==============================================================================
void PatternEngine::processBassPattern(juce::MidiBuffer& midiBuffer, int numSamples)
{
    if (!bassPatternEnabled || bassPattern == BassPattern::Off)
        return;
    
    // Track held notes from input
    for (const auto metadata : midiBuffer)
    {
        auto message = metadata.getMessage();
        if (message.isNoteOn())
        {
            lastHeldNote = message.getNoteNumber();
        }
    }
    
    if (lastHeldNote < 0 || !isPlaying)
        return;
    
    // Clear input MIDI - pattern will generate notes
    midiBuffer.clear();
    
    // Calculate current step based on pattern type
    double stepLength = 0.0;
    int numSteps = 8; // Default for built-in patterns
    
    if (bassPattern == BassPattern::UserSequencer)
    {
        numSteps = 16;
        switch (sequencerStepLengthIndex)
        {
            case 0: stepLength = 4.0; break;  // 1/1
            case 1: stepLength = 2.0; break;  // 1/2
            case 2: stepLength = 1.0; break;  // 1/4
            case 3: stepLength = 0.5; break;  // 1/8
            case 4: stepLength = 0.25; break; // 1/16
            case 5: stepLength = 0.125; break;// 1/32
            default: stepLength = 0.25; break;
        }
    }
    else
    {
        switch (bassPattern)
        {
            case BassPattern::EighthDrive:
            case BassPattern::OctaveBounce:
            case BassPattern::Pumping8ths:
                stepLength = 0.5; // 8th notes
                break;
            case BassPattern::SyncPulse:
            case BassPattern::Staccato16ths:
                stepLength = 0.25; // 16th notes
                break;
            default:
                return;
        }
    }
    
    double currentStep = std::floor(ppqPosition / stepLength);
    int stepIndex = static_cast<int>(currentStep) % numSteps;
    
    // Position within step (0-1)
    double posInStep = std::fmod(ppqPosition, stepLength) / stepLength;
    
    // Get pattern definition
    PatternStep step;
    if (bassPattern == BassPattern::UserSequencer)
    {
        step.velocity = sequencerSteps[stepIndex] ? 0.8f : 0.0f;
        step.gateLength = 0.9f; // Fixed gate for user sequencer
        step.octaveOffset = 0;
    }
    else
    {
        auto steps = getPatternSteps(bassPattern, stepIndex);
        if (steps.empty()) return;
        step = steps[0];
    }
    
    // Generate note events
    if (currentStep != lastPatternStep)
    {
        // New step - trigger note on
        if (step.velocity > 0.0f)
        {
            int noteToPlay = lastHeldNote + step.octaveOffset;
            int velocity = static_cast<int>(step.velocity * 127.0f);
            midiBuffer.addEvent(juce::MidiMessage::noteOn(1, noteToPlay, (juce::uint8)velocity), 0);
            noteCurrentlyOn = true;
        }
        lastPatternStep = currentStep;
    }
    
    // Check if we need to send note off based on gate length
    if (noteCurrentlyOn && posInStep >= step.gateLength)
    {
        int noteToPlay = lastHeldNote + step.octaveOffset;
        midiBuffer.addEvent(juce::MidiMessage::noteOff(1, noteToPlay), 0);
        noteCurrentlyOn = false;
    }
}

//==============================================================================
std::vector<PatternEngine::PatternStep> PatternEngine::getPatternSteps(BassPattern pattern, int stepIndex)
{
    std::vector<PatternStep> steps;
    
    switch (pattern)
    {
        case BassPattern::EighthDrive:
            // Consistent 8th notes with slight accent on beat
            steps.push_back({stepIndex % 2 == 0 ? 1.0f : 0.8f, 0.8f, 0});
            break;
            
        case BassPattern::OctaveBounce:
            // Alternating octaves
            steps.push_back({0.9f, 0.7f, stepIndex % 2 == 0 ? 0 : 12});
            break;
            
        case BassPattern::SyncPulse:
            // 16th note pulse with accents
            {
                float vel = (stepIndex % 4 == 0) ? 1.0f : (stepIndex % 2 == 0) ? 0.7f : 0.5f;
                steps.push_back({vel, 0.5f, 0});
            }
            break;
            
        case BassPattern::Pumping8ths:
            // 8th notes with sidechain-style velocity curve
            {
                float vel = stepIndex % 2 == 0 ? 1.0f : 0.6f;
                float gate = stepIndex % 2 == 0 ? 0.9f : 0.6f;
                steps.push_back({vel, gate, 0});
            }
            break;
            
        case BassPattern::Staccato16ths:
            // Short 16th notes
            steps.push_back({0.85f, 0.3f, 0});
            break;
            
        default:
            break;
    }
    
    return steps;
}

//==============================================================================
juce::StringArray PatternEngine::getBassPatternNames()
{
    return { "Off", "8th Drive", "Octave Bounce", "Sync Pulse", "Pumping 8ths", "Staccato 16ths", "User Sequencer" };
}
