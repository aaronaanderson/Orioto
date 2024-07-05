#pragma once 

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_basics/juce_audio_basics.h>

typedef juce::AudioProcessorValueTreeState::SliderAttachment SliderAttachment;

namespace oi
{
class AttachedSlider : public juce::Component
{
public:
    AttachedSlider (juce::String name, juce::StringRef paramID, juce::AudioProcessorValueTreeState& vts)
    {
        label.setText (name, juce::dontSendNotification);
        label.setJustificationType (juce::Justification::centred);
        addAndMakeVisible (label);
        slider.setSliderStyle (juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
        slider.setTextBoxStyle (juce::Slider::TextEntryBoxPosition::NoTextBox, true, 200, 20);
        addAndMakeVisible (slider);
        sliderAttachment.reset (new SliderAttachment (vts, paramID, slider));
    }
    void resized() override
    {
        auto b = getLocalBounds();
        label.setBounds (b.removeFromTop (20));
        slider.setBounds (b);
    }
private:
    juce::Label label;
    juce::Slider slider;
    std::unique_ptr<SliderAttachment> sliderAttachment;
};
}