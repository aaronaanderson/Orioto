#pragma once 

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include "AttachedSlider.h"
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
class InputGainPanel : public Panel
{
public:
    InputGainPanel (juce::AudioProcessorValueTreeState& vts)
      : Panel ("Input Gain"), 
        gain ("Gain", "InputGain", vts)
    {
        addAndMakeVisible (gain);
    }
    void resized() override
    {
        gain.setBounds (getAdjustedBounds());
    }
private:
    AttachedSlider gain;
};
class LowShelfPanel : public Panel
{
public:
    LowShelfPanel (juce::AudioProcessorValueTreeState& vts)
      : Panel ("Low Shelf"),
        frequency ("Frequency", "LowShelfFrequency", vts), 
        gain ("Gain", "LowShelfGain", vts), 
        q ("Q", "LowShelfQ", vts)
    {
        addAndMakeVisible (frequency);
        addAndMakeVisible (gain);
        addAndMakeVisible (q);
    }
    void resized()
    {
        auto b = getAdjustedBounds();
        auto unitWidth = b.getWidth() / 3;
        
        frequency.setBounds (b.removeFromLeft (unitWidth));
        gain.setBounds (b.removeFromLeft (unitWidth));
        q.setBounds (b.removeFromLeft (unitWidth));
    }
private:
    AttachedSlider frequency;
    AttachedSlider gain;
    AttachedSlider q;
};
class InputCompressionPanel : public Panel
{
public:
    InputCompressionPanel (juce::AudioProcessorValueTreeState& vts)
      : Panel ("Input Compression"), 
        threshold ("Threshold", "InputCompressionThreshold", vts), 
        ratio ("Ratio",         "InputCompressionRatio", vts), 
        attack ("Attack",       "InputCompressionAttack", vts), 
        release ("Release",     "InputCompressionRelease", vts)
    {
        addAndMakeVisible (threshold);
        addAndMakeVisible (ratio);
        addAndMakeVisible (attack);
        addAndMakeVisible (release);
    }
    void resized()
    {
        auto b = getAdjustedBounds();
        auto unitWidth = b.getWidth() / 4;
        threshold.setBounds (b.removeFromLeft (unitWidth));
        ratio.setBounds (b.removeFromLeft (unitWidth));
        attack.setBounds (b.removeFromLeft (unitWidth));
        release.setBounds (b.removeFromLeft (unitWidth));
    }
private:
    AttachedSlider threshold;
    AttachedSlider ratio;
    AttachedSlider attack;
    AttachedSlider release;
};
class BlendPanel : public Panel
{
public:
    BlendPanel (juce::AudioProcessorValueTreeState& vts) 
      : Panel ("Blend"), 
        blend ("Blend", "Blend", vts)
    {
        addAndMakeVisible (blend);
    }
    void resized () override
    {
        blend.setBounds (getAdjustedBounds());
    }
private:
    AttachedSlider blend;
};
class HighShelfPanel : public Panel
{
public:
    HighShelfPanel (juce::AudioProcessorValueTreeState& vts)
      : Panel ("High Shelf"), 
        frequency ("Frequency", "HighShelfFrequency", vts),
        gain ("Gain", "HighShelfGain", vts),
        q ("Q", "HighShelfQ", vts)
    {
        addAndMakeVisible (frequency);
        addAndMakeVisible (gain);
        addAndMakeVisible (q);
    }
    void resized()
    {
        auto b = getAdjustedBounds();
        auto unitWidth = b.getWidth() / 3;
        
        frequency.setBounds (b.removeFromLeft (unitWidth));
        gain.setBounds (b.removeFromLeft (unitWidth));
        q.setBounds (b.removeFromLeft (unitWidth));
    }
private:
    AttachedSlider frequency;
    AttachedSlider gain;
    AttachedSlider q;
};
class LowPassPanel : public Panel
{
public:
    LowPassPanel (juce::AudioProcessorValueTreeState& vts)
      : Panel ("Low Pass"),
        frequency ("Frequency", "LowPassFrequency", vts)
    {
        addAndMakeVisible (frequency);
    }
    void resized() override
    {
        frequency.setBounds (getAdjustedBounds());
    }
private:
    AttachedSlider frequency;
};
class OutputCompressionPanel : public Panel
{
public:
    OutputCompressionPanel (juce::AudioProcessorValueTreeState& vts)
      : Panel ("Output Compression"), 
        threshold ("Threshold", "OutputCompressionThreshold", vts),
        ratio ("Ratio", "OutputCompressionRatio", vts),
        attack ("Attack", "OutputCompressionAttack", vts),
        release ("Release", "OutputCompressionRelease", vts)
    {
        addAndMakeVisible (threshold);
        addAndMakeVisible (ratio);
        addAndMakeVisible (attack);
        addAndMakeVisible (release);
    }
    void resized()
    {
        auto b = getAdjustedBounds();
        auto unitWidth = b.getWidth() / 4;
        threshold.setBounds (b.removeFromLeft (unitWidth));
        ratio.setBounds (b.removeFromLeft (unitWidth));
        attack.setBounds (b.removeFromLeft (unitWidth));
        release.setBounds (b.removeFromLeft (unitWidth));
    }
private:
    AttachedSlider threshold;
    AttachedSlider ratio;
    AttachedSlider attack;
    AttachedSlider release;
};
class OutputLevelPanel : public Panel
{
public:
    OutputLevelPanel (juce::AudioProcessorValueTreeState& vts)
      : Panel ("Output Level"), 
        valueTreeState (vts)
    {
        outputLevelSlider.setSliderStyle (juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
        outputLevelSlider.setTextBoxStyle (juce::Slider::TextEntryBoxPosition::NoTextBox, true, 200, 20);
        addAndMakeVisible (outputLevelSlider);
        outputLevelAttachment.reset (new SliderAttachment (valueTreeState, "OutputLevel", outputLevelSlider));
    }
    void resized()
    {
        outputLevelSlider.setBounds (getAdjustedBounds());
    }
private:
    juce::AudioProcessorValueTreeState& valueTreeState;
    juce::Label outputLeveLabel;
    juce::Slider outputLevelSlider;
    std::unique_ptr<SliderAttachment> outputLevelAttachment;
};
class InnerControlPannel : public juce::Component 
{
public:
    InnerControlPannel (juce::AudioProcessorValueTreeState& vts)
      : inputGainPanel (vts),
        lowShelfPanel (vts),
        inputCompressionPanel (vts), 
        blendPanel (vts),
        highShelfPanel (vts),
        lowPassPanel (vts),
        outputCompressionPanel (vts)
    { 
        addAndMakeVisible (inputGainPanel);
        addAndMakeVisible (lowShelfPanel);
        addAndMakeVisible (inputCompressionPanel);
        addAndMakeVisible (blendPanel);
        addAndMakeVisible (highShelfPanel);
        addAndMakeVisible (lowPassPanel);
        addAndMakeVisible (outputCompressionPanel);
    }
    void resized()
    {
        auto b = getLocalBounds();
        auto unitHeight = b.getHeight() / 7;
        inputGainPanel.setBounds (b.removeFromTop (unitHeight).reduced (2));
        lowShelfPanel.setBounds (b.removeFromTop (unitHeight).reduced (2));
        inputCompressionPanel.setBounds (b.removeFromTop (unitHeight).reduced (2));
        blendPanel.setBounds (b.removeFromTop (unitHeight).reduced (2));
        highShelfPanel.setBounds (b.removeFromTop (unitHeight).reduced (2));
        lowPassPanel.setBounds (b.removeFromTop (unitHeight).reduced (2));
        outputCompressionPanel.setBounds (b.removeFromTop (unitHeight).reduced (2));
    }
private:
    InputGainPanel inputGainPanel;
    LowShelfPanel lowShelfPanel;
    InputCompressionPanel inputCompressionPanel;
    BlendPanel blendPanel;
    HighShelfPanel highShelfPanel;
    LowPassPanel lowPassPanel;
    OutputCompressionPanel outputCompressionPanel;
};
class ControlPanel : public juce::Component
{
public:
    ControlPanel (juce::AudioProcessorValueTreeState& vts)
      : viewPort ("Inner Control Pannel"),
        outputLevelPanel (vts)
    {
        viewPort.setViewedComponent(new InnerControlPannel (vts));
        viewPort.setScrollBarThickness (10);
        viewPort.setScrollBarsShown (true, false);
        addAndMakeVisible (viewPort);
        addAndMakeVisible (outputLevelPanel);
    }
    void resized() override
    { 
        auto b = getLocalBounds();
        outputLevelPanel.setBounds (b.removeFromBottom (100).reduced (2));
        viewPort.setBounds (b);
        juce::Rectangle<int> innerViewBounds = {getLocalBounds().getWidth(), 800};
        auto vc = viewPort.getViewedComponent();
        vc->setBounds (innerViewBounds);
    }
private:
    juce::Viewport viewPort;
    OutputLevelPanel outputLevelPanel;
};
}