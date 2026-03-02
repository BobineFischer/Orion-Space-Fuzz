#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
OrionSpaceFuzzAudioProcessor::OrionSpaceFuzzAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
    : AudioProcessor(BusesProperties()
#if ! JucePlugin_IsMidiEffect
#if ! JucePlugin_IsSynth
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
    ),
    apvts(*this, nullptr, "Parameters", createParameterLayout())
#endif
{
}

OrionSpaceFuzzAudioProcessor::~OrionSpaceFuzzAudioProcessor() {}

// --- Define Parameters (6 Knobs total) ---
juce::AudioProcessorValueTreeState::ParameterLayout OrionSpaceFuzzAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    // Fuzz
    layout.add(std::make_unique<juce::AudioParameterFloat>("DRIVE", "Fuzz Drive", 1.0f, 20.0f, 3.0f));
    // Chorus
    layout.add(std::make_unique<juce::AudioParameterFloat>("CRATE", "Chorus Rate", 0.1f, 10.0f, 1.5f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("CDEPTH", "Chorus Depth", 0.0f, 1.0f, 0.5f));
    // Delay
    layout.add(std::make_unique<juce::AudioParameterFloat>("DELAY", "Delay Time", 0.0f, 2000.0f, 500.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("FEED", "Feedback", 0.0f, 0.95f, 0.4f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("DMIX", "Delay Mix", 0.0f, 1.0f, 0.5f));

    return layout;
}

void OrionSpaceFuzzAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    // 1. Prepare Custom Delay Buffer (2 seconds max)
    mDelayBuffer.setSize(getTotalNumOutputChannels(), (int)(2.0 * sampleRate));
    mDelayBuffer.clear();
    mWritePosition = 0;

    // 2. Prepare JUCE Chorus Module
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = getTotalNumOutputChannels();

    mChorus.prepare(spec);
    mChorus.setMix(0.5f); // Set standard internal chorus mix
}

void OrionSpaceFuzzAudioProcessor::releaseResources() {}

#ifndef JucePlugin_PreferredChannelConfigurations
bool OrionSpaceFuzzAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
        && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
    return true;
}
#endif

// --- CORE DSP LOOP ---
void OrionSpaceFuzzAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    // --- 1. Load Parameters Thread-Safely ---
    float drive = apvts.getRawParameterValue("DRIVE")->load();
    float cRate = apvts.getRawParameterValue("CRATE")->load();
    float cDepth = apvts.getRawParameterValue("CDEPTH")->load();
    float delayTimeMS = apvts.getRawParameterValue("DELAY")->load();
    float feedback = apvts.getRawParameterValue("FEED")->load();
    float mix = apvts.getRawParameterValue("DMIX")->load();

    // Update Chorus parameters
    mChorus.setRate(cRate);
    mChorus.setDepth(cDepth);

    float sampleRate = getSampleRate();
    int bufferLength = buffer.getNumSamples();
    int delayBufferLength = mDelayBuffer.getNumSamples();

    // --- 2. Fuzz Stage (Non-linear Waveshaping) ---
    // Signal Flow: Input -> Fuzz
    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer(channel);
        for (int sample = 0; sample < bufferLength; ++sample)
        {
            float inSample = channelData[sample];
            // Apply hyperbolic tangent for soft clipping / fuzz
            // Divided by tanh(drive) to compensate for volume gain
            channelData[sample] = std::tanh(inSample * drive) / std::tanh(drive);
        }
    }

    // --- 3. Chorus Stage (JUCE DSP Module) ---
    // Signal Flow: Fuzz -> Chorus
    juce::dsp::AudioBlock<float> audioBlock(buffer);
    juce::dsp::ProcessContextReplacing<float> context(audioBlock);
    mChorus.process(context);

    // --- 4. Custom Delay Stage (Ring Buffer) ---
    // Signal Flow: Chorus -> Delay
    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer(channel);
        auto* delayData = mDelayBuffer.getWritePointer(juce::jmin(channel, mDelayBuffer.getNumChannels() - 1));
        int tempWritePos = mWritePosition;

        for (int sample = 0; sample < bufferLength; ++sample)
        {
            float inSample = channelData[sample]; // Now contains Fuzz + Chorus

            // Calculate integer read position
            float delayInSamples = (delayTimeMS / 1000.0f) * sampleRate;
            int readPosition = tempWritePos - (int)delayInSamples;
            if (readPosition < 0) readPosition += delayBufferLength;

            float delayedSample = delayData[readPosition];

            // Mix Dry and Wet
            float outSample = (inSample * (1.0f - mix)) + (delayedSample * mix);

            // Feedback into the delay line
            delayData[tempWritePos] = inSample + (delayedSample * feedback);

            tempWritePos++;
            if (tempWritePos >= delayBufferLength) tempWritePos = 0;

            channelData[sample] = outSample;
        }
    }

    // Update global write pointer
    mWritePosition += bufferLength;
    if (mWritePosition >= delayBufferLength) mWritePosition %= delayBufferLength;
}

// --- Save/Load State (DAW Automation) ---
void OrionSpaceFuzzAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void OrionSpaceFuzzAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName(apvts.state.getType()))
            apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

juce::AudioProcessorEditor* OrionSpaceFuzzAudioProcessor::createEditor() { return new OrionSpaceFuzzAudioProcessorEditor(*this); }
bool OrionSpaceFuzzAudioProcessor::hasEditor() const { return true; }
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new OrionSpaceFuzzAudioProcessor(); }