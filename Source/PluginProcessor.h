#pragma once
#include <JuceHeader.h>

class OrionSpaceFuzzAudioProcessor : public juce::AudioProcessor
{
public:
    OrionSpaceFuzzAudioProcessor();
    ~OrionSpaceFuzzAudioProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

#ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
#endif

    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int index) override {}
    const juce::String getProgramName(int index) override { return {}; }
    void changeProgramName(int index, const juce::String& newName) override {}

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    // --- APVTS: Thread-safe parameter management ---
    juce::AudioProcessorValueTreeState apvts;

private:
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    // --- 1. Custom Delay (Ring Buffer) Variables ---
    juce::AudioBuffer<float> mDelayBuffer;
    int mWritePosition{ 0 };

    // --- 2. JUCE DSP Chorus Module ---
    juce::dsp::Chorus<float> mChorus;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OrionSpaceFuzzAudioProcessor)
};
