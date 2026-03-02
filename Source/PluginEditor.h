#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

class OrionSpaceFuzzAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    OrionSpaceFuzzAudioProcessorEditor(OrionSpaceFuzzAudioProcessor&);
    ~OrionSpaceFuzzAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    OrionSpaceFuzzAudioProcessor& audioProcessor;

    // --- Helper function to create sliders easily ---
    void setupSlider(juce::Slider& slider, juce::Label& label, const juce::String& text);

    // --- UI Elements ---
    juce::Slider driveSlider;
    juce::Slider cRateSlider;
    juce::Slider cDepthSlider;
    juce::Slider delaySlider;
    juce::Slider feedSlider;
    juce::Slider mixSlider;

    juce::Label driveLabel;
    juce::Label cRateLabel;
    juce::Label cDepthLabel;
    juce::Label delayLabel;
    juce::Label feedLabel;
    juce::Label mixLabel;

    // --- APVTS Attachments ---
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    std::unique_ptr<SliderAttachment> driveAttach;
    std::unique_ptr<SliderAttachment> cRateAttach;
    std::unique_ptr<SliderAttachment> cDepthAttach;
    std::unique_ptr<SliderAttachment> delayAttach;
    std::unique_ptr<SliderAttachment> feedAttach;
    std::unique_ptr<SliderAttachment> mixAttach;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OrionSpaceFuzzAudioProcessorEditor)
};