/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
TheHorsePluginAudioProcessor::TheHorsePluginAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
}

TheHorsePluginAudioProcessor::~TheHorsePluginAudioProcessor()
{
}

//==============================================================================
const juce::String TheHorsePluginAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool TheHorsePluginAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool TheHorsePluginAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool TheHorsePluginAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double TheHorsePluginAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int TheHorsePluginAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int TheHorsePluginAudioProcessor::getCurrentProgram()
{
    return 0;
}

void TheHorsePluginAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String TheHorsePluginAudioProcessor::getProgramName (int index)
{
    return {};
}

void TheHorsePluginAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void TheHorsePluginAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    smoothed_dry_wet.reset(sampleRate, 0.01);
    smoothed_delayt_time_ms.reset(sampleRate, 0.0025);
    verb.prepare_to_play(sampleRate);
}

void TheHorsePluginAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool TheHorsePluginAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void TheHorsePluginAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    if(buffer.getNumChannels() != 2){
        DBG("channel count mismatch.");
        return;
    }

    verb.set_parameters(smoothed_delayt_time_ms.getNextValue());
    verb.process_block(buffer);
}

//==============================================================================
bool TheHorsePluginAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* TheHorsePluginAudioProcessor::createEditor()
{
    return new TheHorsePluginAudioProcessorEditor (*this);
}

//==============================================================================
void TheHorsePluginAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void TheHorsePluginAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}
void TheHorsePluginAudioProcessor::update_dt(float in_time){
    smoothed_delayt_time_ms.setTargetValue(std::abs(in_time));
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new TheHorsePluginAudioProcessor();
}
