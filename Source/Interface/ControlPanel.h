#pragma once 

#include <juce_gui_basics/juce_gui_basics.h>

namespace oi
{
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
private:
    juce::String name;
};
class LowShelfPanel : public Panel
{
public:
    LowShelfPanel()
      : Panel ("Low Shelf")
    {}
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
    ControlPanel()
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