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
    // this doesn't work because the opengl context is rendering on top of everything
    // TODO: you will need to make a quad that always faces the camera and has the png as a texture.
    //hud = juce::ImageCache::getFromMemory(BinaryData::hud_png, BinaryData::hud_pngSize);
    //g.drawImageWithin(hud, 0, 0, getWidth(), getHeight(), juce::RectanglePlacement::stretchToFit);
    //g.fillAll(juce::Colours::whitesmoke);
}

void TheHorsePluginAudioProcessorEditor::resized()
{
    // might have a pointer issue here.
    gl->setBounds(getLocalBounds());
}
