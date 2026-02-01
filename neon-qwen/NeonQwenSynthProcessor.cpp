/*
  ==============================================================================

    NeonQwenSynthProcessor.cpp
    Created: 31 Jan 2026 3:51:06pm
    Author:  Adrian

  ==============================================================================
*/

#include "NeonQwenSynthProcessor.h"
#include "NeonQwenSynthEditor.h"

//==============================================================================
NeonQwenSynthProcessor::NeonQwenSynthProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true))
#endif
{
    // Add voices to our synth
    for (int i = 0; i < 16; ++i)
    {
        synth.addVoice(new NeonQwenSynthVoice());
    }
    
    // Add a sound for them to play
    synth.addSound(new NeonQwenSynthSound());
    
    // Initialize parameters
    volume = 0.5f;
    attack = 0.1f;
    release = 0.3f;
}

NeonQwenSynthProcessor::~NeonQwenSynthProcessor()
{
}

//==============================================================================
void NeonQwenSynthProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    synth.setCurrentPlaybackSampleRate(sampleRate);
}

void NeonQwenSynthProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JUCE_LINUX
bool NeonQwenSynthProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    if (layouts.getNumInputChannels() != 0 || layouts.getNumOutputChannels() != 0)
        return false;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    if (layouts.getMainInputChannelSet() != juce::AudioChannelSet::disabled()
        && layouts.getMainInputChannelSet() != layouts.getMainOutputChannelSet())
        return false;
  #endif
    return true;
}
#endif

void NeonQwenSynthProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // This is the place where you process the audio
    synth.renderNextBlock(buffer, midiMessages, 0, buffer.getNumSamples());
    
    // Apply volume control
    auto numSamples = buffer.getNumSamples();
    for (int i = 0; i < buffer.getNumChannels(); ++i)
    {
        buffer.applyGain(i, 0, numSamples, volume);
    }
}

//==============================================================================
bool NeonQwenSynthProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* NeonQwenSynthProcessor::createEditor()
{
    return new NeonQwenSynthEditor(*this);
}

//==============================================================================
const juce::String NeonQwenSynthProcessor::getName() const
{
    return JucePlugin_Name;
}

bool NeonQwenSynthProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool NeonQwenSynthProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool NeonQwenSynthProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double NeonQwenSynthProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

//==============================================================================
int NeonQwenSynthProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int NeonQwenSynthProcessor::getCurrentProgram()
{
    return 0;
}

void NeonQwenSynthProcessor::setCurrentProgram (int index)
{
}

const juce::String NeonQwenSynthProcessor::getProgramName (int index)
{
    return {};
}

void NeonQwenSynthProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void NeonQwenSynthProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void NeonQwenSynthProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new NeonQwenSynthProcessor();
}