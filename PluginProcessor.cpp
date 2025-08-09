#include "PluginProcessor.h"
#include "PluginEditor.h"

// Parameter IDs
static constexpr auto paramInputGain = "inputGain";
static constexpr auto paramDrive = "drive";
static constexpr auto paramBass = "bass";
static constexpr auto paramMid = "mid";
static constexpr auto paramTreble = "treble";
static constexpr auto paramOutputGain = "outputGain";
static constexpr auto paramCabIRPath = "cabIRPath"; // stored as string in State


AmpSimAudioProcessor::AmpSimAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                       ),
#else
     : AudioProcessor (BusesProperties()),
#endif
       apvts(*this, nullptr, "PARAMETERS", {
           std::make_unique<juce::AudioParameterFloat>(paramInputGain, "Input Gain", juce::NormalisableRange<float>(-24.0f, 24.0f, 0.01f), 0.0f),
           std::make_unique<juce::AudioParameterFloat>(paramDrive, "Drive", juce::NormalisableRange<float>(0.0f, 10.0f, 0.01f), 2.0f),
           std::make_unique<juce::AudioParameterFloat>(paramBass, "Bass", juce::NormalisableRange<float>(-12.0f, 12.0f, 0.1f), 0.0f),
           std::make_unique<juce::AudioParameterFloat>(paramMid, "Mid", juce::NormalisableRange<float>(-12.0f, 12.0f, 0.1f), 0.0f),
           std::make_unique<juce::AudioParameterFloat>(paramTreble, "Treble", juce::NormalisableRange<float>(-12.0f, 12.0f, 0.1f), 0.0f),
           std::make_unique<juce::AudioParameterFloat>(paramOutputGain, "Output Gain", juce::NormalisableRange<float>(-24.0f, 24.0f, 0.01f), 0.0f)
       })
{
    // initialize filters with default coefficients
}

AmpSimAudioProcessor::~AmpSimAudioProcessor()
{
}

const juce::String AmpSimAudioProcessor::getName() const { return JucePlugin_Name; }

bool AmpSimAudioProcessor::acceptsMidi() const { return false; }
bool AmpSimAudioProcessor::producesMidi() const { return false; }
bool AmpSimAudioProcessor::isMidiEffect() const { return false; }
double AmpSimAudioProcessor::getTailLengthSeconds() const { return 0.0; }

int AmpSimAudioProcessor::getNumPrograms() { return 1; }
int AmpSimAudioProcessor::getCurrentProgram() { return 0; }
void AmpSimAudioProcessor::setCurrentProgram (int) {}
cons
