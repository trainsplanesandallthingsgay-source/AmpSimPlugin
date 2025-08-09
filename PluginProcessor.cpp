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
const juce::String AmpSimAudioProcessor::getProgramName (int) { return {}; }
void AmpSimAudioProcessor::changeProgramName (int, const juce::String&) {}

void AmpSimAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;

    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = (juce::uint32) samplesPerBlock;
    spec.numChannels = (juce::uint32) getTotalNumOutputChannels();

    lowShelf.reset(); lowShelf.prepare(spec);
    midPeaking.reset(); midPeaking.prepare(spec);
    highShelf.reset(); highShelf.prepare(spec);

    // Setup default filter coefficients (simple shelving/peaking)
    auto lowCoeffs = juce::dsp::IIR::Coefficients<float>::makeLowShelf(sampleRate, 120.0f, 0.7071f, juce::Decibels::decibelsToGain(0.0f));
    auto midCoeffs = juce::dsp::IIR::Coefficients<float>::makePeakFilter(sampleRate, 800.0f, 0.7071f, juce::Decibels::decibelsToGain(0.0f));
    auto highCoeffs = juce::dsp::IIR::Coefficients<float>::makeHighShelf(sampleRate, 3000.0f, 0.7071f, juce::Decibels::decibelsToGain(0.0f));

    *lowShelf.state = *lowCoeffs;
    *midPeaking.state = *midCoeffs;
    *highShelf.state = *highCoeffs;

    // prepare convolution (cab) - empty for now
    cabinetConvolver.reset();
    cabinetLoaded = false;
}

void AmpSimAudioProcessor::releaseResources()
{
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool AmpSimAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    // Only support stereo in/out for now
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
    return true;
}
#endif

// Simple waveshaper: tanh-based soft clipping controlled by drive
float AmpSimAudioProcessor::applyWaveshaper(float x, float drive) noexcept
{
    // drive is a multiplier >0
    float shaped = std::tanh(x * (1.0f + drive * 0.5f));
    return shaped;
}

void AmpSimAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // fetch parameter values
    float inGainDb = apvts.getRawParameterValue(paramInputGain)->load();
    float drive = apvts.getRawParameterValue(paramDrive)->load();
    float bassDb = apvts.getRawParameterValue(paramBass)->load();
    float midDb = apvts.getRawParameterValue(paramMid)->load();
    float trebleDb = apvts.getRawParameterValue(paramTreble)->load();
    float outGainDb = apvts.getRawParameterValue(paramOutputGain)->load();

    float inGain = juce::Decibels::decibelsToGain(inGainDb);
    float outGain = juce::Decibels::decibelsToGain(outGainDb);

    // update filter gains (recalculate coefficients)
    // Note: for performance, recalc only when values change - simplified here for clarity
    *lowShelf.state = *juce::dsp::IIR::Coefficients<float>::makeLowShelf((float)currentSampleRate, 120.0f, 0.7071f, juce::Decibels::decibelsToGain(bassDb));
    *midPeaking.state = *juce::dsp::IIR::Coefficients<float>::makePeakFilter((float)currentSampleRate, 800.0f, 0.7071f, juce::Decibels::decibelsToGain(midDb));
    *highShelf.state = *juce::dsp::IIR::Coefficients<float>::makeHighShelf((float)currentSampleRate, 3000.0f, 0.7071f, juce::Decibels::decibelsToGain(trebleDb));

    juce::dsp::AudioBlock<float> block (buffer);
    juce::dsp::ProcessContextReplacing<float> context (block);

    // Apply input gain
    buffer.applyGain(inGain);

    // Waveshaper (per sample)
    for (int ch = 0; ch < totalNumInputChannels; ++ch)
    {
        auto* channelData = buffer.getWritePointer(ch);
        for (int n = 0; n < buffer.getNumSamples(); ++n)
        {
            channelData[n] = applyWaveshaper(channelData[n], drive);
        }
    }

    // Tone stack (filters)
    lowShelf.process(context);
    midPeaking.process(context);
    highShelf.process(context);

    // Cabinet convolution if loaded
    if (cabinetLoaded)
    {
        cabinetConvolver.process(context);
    }

    // Output gain
    buffer.applyGain(outGain);

    // Clear unused channels
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());
}

// Editor
juce::AudioProcessorEditor* AmpSimAudioProcessor::createEditor() { return new class AmpSimAudioProcessorEditor (*this); }
bool AmpSimAudioProcessor::hasEditor() const { return true; }

void AmpSimAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // Save APVTS state
    if (auto xml = apvts.state.createXml())
        copyXmlToBinary (*xml, destData);
}

void AmpSimAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xmlState = getXmlFromBinary (data, sizeInBytes))
        apvts.replaceState (juce::ValueTree::fromXml (*xmlState));
}
