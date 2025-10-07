/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
TheHorsePluginAudioProcessorEditor::TheHorsePluginAudioProcessorEditor (TheHorsePluginAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    gl = std::make_unique<OpenGLOut>();
    gl->update_callback = [&](float val){p.update_dt(std::abs(val));};
    gl->toBack();
    addAndMakeVisible(gl.get());
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (500, 300);
}

TheHorsePluginAudioProcessorEditor::~TheHorsePluginAudioProcessorEditor()
{
}

//==============================================================================
void TheHorsePluginAudioProcessorEditor::paint (juce::Graphics& g)
{
}

void TheHorsePluginAudioProcessorEditor::resized()
{
    // might have a pointer issue here.
    gl->setBounds(getLocalBounds());
}
