#pragma once
#include <JuceHeader.h>
#include "Presets.h"

class AmpSimAudioProcessor  : public juce::AudioProcessor {
public:
    AmpSimAudioProcessor();
    ~AmpSimAudioProcessor() override {}

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "AmpSim"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return factoryPresets.size(); }
    int getCurrentProgram() override { return currentPresetIndex; }
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState apvts;

private:
    int currentPresetIndex = 0;
    juce::dsp::Oversampling<float> oversampler;
    juce::dsp::Convolution cabIR;

    juce::AudioProcessorValueTreeState::ParameterLayout createParams();
    float waveshaper(float x);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AmpSimAudioProcessor)
};
