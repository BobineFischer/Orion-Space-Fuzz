#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
OrionSpaceFuzzAudioProcessorEditor::OrionSpaceFuzzAudioProcessorEditor(OrionSpaceFuzzAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    // A wider window for a 2x3 grid of knobs
    setSize(600, 400);

    // Setup Fuzz
    setupSlider(driveSlider, driveLabel, "Fuzz Drive");
    driveSlider.setColour(juce::Slider::thumbColourId, juce::Colours::red); // Make the fuzz knob red!
    driveAttach = std::make_unique<SliderAttachment>(audioProcessor.apvts, "DRIVE", driveSlider);

    // Setup Chorus
    setupSlider(cRateSlider, cRateLabel, "Chorus Rate");
    cRateAttach = std::make_unique<SliderAttachment>(audioProcessor.apvts, "CRATE", cRateSlider);
    setupSlider(cDepthSlider, cDepthLabel, "Chorus Depth");
    cDepthAttach = std::make_unique<SliderAttachment>(audioProcessor.apvts, "CDEPTH", cDepthSlider);

    // Setup Delay
    setupSlider(delaySlider, delayLabel, "Delay Time");
    delayAttach = std::make_unique<SliderAttachment>(audioProcessor.apvts, "DELAY", delaySlider);
    setupSlider(feedSlider, feedLabel, "Feedback");
    feedAttach = std::make_unique<SliderAttachment>(audioProcessor.apvts, "FEED", feedSlider);
    setupSlider(mixSlider, mixLabel, "Delay Mix");
    mixAttach = std::make_unique<SliderAttachment>(audioProcessor.apvts, "DMIX", mixSlider);
}

OrionSpaceFuzzAudioProcessorEditor::~OrionSpaceFuzzAudioProcessorEditor() {}

// Helper function implementation
void OrionSpaceFuzzAudioProcessorEditor::setupSlider(juce::Slider& slider, juce::Label& label, const juce::String& text)
{
    slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
    addAndMakeVisible(slider);

    label.setText(text, juce::dontSendNotification);
    label.setJustificationType(juce::Justification::centred);
    label.setColour(juce::Label::textColourId, juce::Colours::lightgrey);
    addAndMakeVisible(label);
}

void OrionSpaceFuzzAudioProcessorEditor::paint(juce::Graphics& g)
{
    // Dark Space-like background
    g.fillAll(juce::Colour::fromRGB(20, 20, 30));

    // Title styling
    g.setColour(juce::Colours::cyan);
    g.setFont(juce::FontOptions(28.0f, juce::Font::bold));
    g.drawText("ORION SPACE FUZZ", getLocalBounds().removeFromTop(60), juce::Justification::centred, true);

    // Subtitle / Credit
    g.setColour(juce::Colours::grey);
    g.setFont(juce::FontOptions(14.0f, juce::Font::italic));
    g.drawText("A Tribute to Cliff Burton", getLocalBounds().removeFromTop(100).withTrimmedTop(50), juce::Justification::centred, true);
}

void OrionSpaceFuzzAudioProcessorEditor::resized()
{
    auto area = getLocalBounds();
    area.removeFromTop(80); // Leave space for the title
    area.reduce(20, 20);    // Margins

    int sliderWidth = area.getWidth() / 3;
    int sliderHeight = area.getHeight() / 2;

    // A lambda to easily place our slider+label combos
    auto placeKnob = [&](juce::Rectangle<int>& bounds, juce::Slider& s, juce::Label& l)
        {
            auto localArea = bounds;
            l.setBounds(localArea.removeFromTop(20));
            s.setBounds(localArea);
        };

    // Row 1: Fuzz Drive | Chorus Rate | Chorus Depth
    auto row1 = area.removeFromTop(sliderHeight);
    placeKnob(row1.removeFromLeft(sliderWidth), driveSlider, driveLabel);
    placeKnob(row1.removeFromLeft(sliderWidth), cRateSlider, cRateLabel);
    placeKnob(row1.removeFromLeft(sliderWidth), cDepthSlider, cDepthLabel);

    // Row 2: Delay Time | Delay Feedback | Delay Mix
    auto row2 = area;
    placeKnob(row2.removeFromLeft(sliderWidth), delaySlider, delayLabel);
    placeKnob(row2.removeFromLeft(sliderWidth), feedSlider, feedLabel);
    placeKnob(row2.removeFromLeft(sliderWidth), mixSlider, mixLabel);
}