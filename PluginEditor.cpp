#include "PluginEditor.h"

AmpSimAudioProcessorEditor::AmpSimAudioProcessorEditor (AmpSimAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    setSize (560, 220);

    // Configure sliders
    auto setupSlider = [this](juce::Slider& s)
    {
        s.setSliderStyle (juce::Slider::Rotary);
        s.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 60, 18);
        addAndMakeVisible (s);
    };

    setupSlider(inputGainSlider);
    setupSlider(driveSlider);
    setupSlider(bassSlider);
    setupSlider(midSlider);
    setupSlider(trebleSlider);
    setupSlider(outputGainSlider);

    // Attach to parameters
    inputGainAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "inputGain", inputGainSlider);
    driveAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "drive", driveSlider);
    bassAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "bass", bassSlider);
    midAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "mid", midSlider);
    trebleAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "treble", trebleSlider);
    outputGainAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "outputGain", outputGainSlider);
}

AmpSimAudioProcessorEditor::~AmpSimAudioProcessorEditor()
{
}

void AmpSimAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::black);
    g.setColour (juce::Colours::white);
    g.setFont (15.0f);
    g.drawFittedText ("AmpSim Starter", getLocalBounds(), juce::Justification::centredTop, 1);
}

void AmpSimAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced (10);
    int knobW = 90;
    int knobH = 110;

    juce::Rectangle<int> r = area.removeFromTop(knobH);
    inputGainSlider.setBounds(r.removeFromLeft(knobW).reduced(8));
    driveSlider.setBounds(r.removeFromLeft(knobW).reduced(8));
    bassSlider.setBounds(r.removeFromLeft(knobW).reduced(8));
    midSlider.setBounds(r.removeFromLeft(knobW).reduced(8));
    trebleSlider.setBounds(r.removeFromLeft(knobW).reduced(8));
    outputGainSlider.setBounds(r.removeFromLeft(knobW).reduced(8));
}
