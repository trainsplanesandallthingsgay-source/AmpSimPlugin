#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

class AmpSimAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    explicit AmpSimAudioProcessorEditor (AmpSimAudioProcessor&);
    ~AmpSimAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    AmpSimAudioProcessor& audioProcessor;

    // Sliders and attachments
    juce::Slider inputGainSlider;
    juce::Slider driveSlider;
    juce::Slider bassSlider;
    juce::Slider midSlider;
    juce::Slider trebleSlider;
    juce::Slider outputGainSlider;

    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    std::unique_ptr<SliderAttachment> inputGainAttachment;
    std::unique_ptr<SliderAttachment> driveAttachment;
    std::unique_ptr<SliderAttachment> bassAttachment;
    std::unique_ptr<SliderAttachment> midAttachment;
    std::unique_ptr<SliderAttachment> trebleAttachment;
    std::unique_ptr<SliderAttachment> outputGainAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AmpSimAudioProcessorEditor)
};
