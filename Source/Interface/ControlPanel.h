#pragma once 

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_basics/juce_audio_basics.h>
namespace oi
{

typedef juce::AudioProcessorValueTreeState::SliderAttachment SliderAttachment;

class Panel : public juce::Component
{
public:
    Panel (juce::String n)
      : name (n)
    {}
    void paint (juce::Graphics& g) override
    {
        auto laf = dynamic_cast<OriotoLookAndFeel*> (&getLookAndFeel());
        jassert (laf != nullptr);
        g.fillAll (laf->getBackgroundColour());
        g.setColour (laf->getBaseColour().brighter (2.5f));
        juce::Rectangle<float> b = {static_cast<float> (getLocalBounds().getWidth()), 
                                    static_cast<float> (getLocalBounds().getHeight() * 0.2)};
        g.drawText (name, b, juce::Justification::left);
    }
    juce::Rectangle<int> getAdjustedBounds()
    {
      return getLocalBounds().removeFromBottom ((int) (getHeight() * 0.8));
    }
private:
    juce::String name;
};
class LowShelfPanel : public Panel
{
public:
    LowShelfPanel (juce::AudioProcessorValueTreeState& vts)
      : Panel ("Low Shelf"),
        valueTreeState (vts)
    {
        frequencyLabel.setText ("Frequency", juce::dontSendNotification);
        frequencyLabel.setJustificationType (juce::Justification::centred);
        addAndMakeVisible (frequencyLabel);
        frequencySlider.setSliderStyle (juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
        frequencySlider.setTextBoxStyle (juce::Slider::TextEntryBoxPosition::NoTextBox, true, 200, 20);
        addAndMakeVisible (frequencySlider);
        frequencyAttachment.reset (new SliderAttachment (valueTreeState, "LowShelfFrequency", frequencySlider));
    
        gainLabel.setText ("Gain", juce::dontSendNotification);
        gainLabel.setJustificationType (juce::Justification::centred);
        addAndMakeVisible (gainLabel);
        gainSlider.setSliderStyle (juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
        gainSlider.setTextBoxStyle (juce::Slider::TextEntryBoxPosition::NoTextBox, true, 200, 20);
        addAndMakeVisible (gainSlider);
        gainAttachment.reset (new SliderAttachment (valueTreeState, "LowShelfGain", gainSlider));

        qLabel.setText ("Q", juce::dontSendNotification);
        qLabel.setJustificationType (juce::Justification::centred);
        addAndMakeVisible (qLabel);
        qSlider.setSliderStyle (juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
        qSlider.setTextBoxStyle (juce::Slider::TextEntryBoxPosition::NoTextBox, true, 200, 20);
        addAndMakeVisible (qSlider);
        qAttachment.reset (new SliderAttachment (valueTreeState, "LowShelfQ", qSlider));
    }
    void resized()
    {
        auto b = getAdjustedBounds();
        auto unitWidth = b.getWidth() / 3;
        
        auto frequencyBounds = b.removeFromLeft (unitWidth);
        frequencyLabel.setBounds (frequencyBounds.removeFromTop (20));
        frequencySlider.setBounds (frequencyBounds);
        
        auto gainBounds = b.removeFromLeft (unitWidth);
        gainLabel.setBounds (gainBounds.removeFromTop (20));
        gainSlider.setBounds (gainBounds);

        auto qBounds = b.removeFromLeft (unitWidth);
        qLabel.setBounds (qBounds.removeFromTop (20));
        qSlider.setBounds (qBounds);
    }
private:
    juce::AudioProcessorValueTreeState& valueTreeState;
    
    juce::Label frequencyLabel;
    juce::Slider frequencySlider;
    std::unique_ptr<SliderAttachment> frequencyAttachment;
    
    juce::Label gainLabel;
    juce::Slider gainSlider;
    std::unique_ptr<SliderAttachment> gainAttachment;
    
    juce::Label qLabel;
    juce::Slider qSlider;
    std::unique_ptr<SliderAttachment> qAttachment;
};
class InputCompressionPanel : public Panel
{
public:
    InputCompressionPanel()
      : Panel ("Input Compression")
    {}
};
class BlendPanel : public Panel
{
public:
    BlendPanel() 
      : Panel ("Blend")
    {}
};
class HighShelfPanel : public Panel
{
public:
    HighShelfPanel()
      : Panel ("High Shelf")
    {}
};
class LowPassPanel : public Panel
{
public:
    LowPassPanel()
      : Panel ("Low Pass")
    {}
};
class OutputCompressionPanel : public Panel
{
public:
    OutputCompressionPanel()
      : Panel ("Output Compression")
    {}
};
class OutputLevevlPanel : public Panel
{
public:
    OutputLevevlPanel()
      : Panel ("Output Level")
    {}
};


class ControlPanel : public juce::Component 
{
public:
    ControlPanel (juce::AudioProcessorValueTreeState& vts)
      : lowShelfPanel (vts)
    { 
        addAndMakeVisible (lowShelfPanel);
        addAndMakeVisible (inputCompressionPanel);
        addAndMakeVisible (blendPanel);
        addAndMakeVisible (highShelfPanel);
        addAndMakeVisible (lowPassPanel);
        addAndMakeVisible (outputCompressionPanel);
        addAndMakeVisible (outputLevelPanel);
    }
    void resized()
    {
        auto b = getLocalBounds();
        auto unitHeight = b.getHeight() / 7;
        lowShelfPanel.setBounds (b.removeFromTop (unitHeight).reduced (2));
        inputCompressionPanel.setBounds (b.removeFromTop (unitHeight).reduced (2));
        blendPanel.setBounds (b.removeFromTop (unitHeight).reduced (2));
        highShelfPanel.setBounds (b.removeFromTop (unitHeight).reduced (2));
        lowPassPanel.setBounds (b.removeFromTop (unitHeight).reduced (2));
        outputCompressionPanel.setBounds (b.removeFromTop (unitHeight).reduced (2));
        outputLevelPanel.setBounds (b.removeFromTop (unitHeight).reduced (2));
    }
private:
    LowShelfPanel lowShelfPanel;
    InputCompressionPanel inputCompressionPanel;
    BlendPanel blendPanel;
    HighShelfPanel highShelfPanel;
    LowPassPanel lowPassPanel;
    OutputCompressionPanel outputCompressionPanel;
    OutputLevevlPanel outputLevelPanel;

};
}