#include "MainProcessor.h"
#include "MainEditor.h"

//==============================================================================
MainEditor::MainEditor (MainProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p), 
      curveEditor (processorRef.getState().getChildWithName (id::CURVE))
{
    juce::ignoreUnused (processorRef);

    setLookAndFeel (&lookAndFeel);
    
    addAndMakeVisible (curveEditor);

    setResizable (true, false);
    setResizeLimits (300, 200, 2400, 1600);
    setSize (1200, 600);
}

MainEditor::~MainEditor()
{
    setLookAndFeel (nullptr);
}

//==============================================================================
void MainEditor::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
}

void MainEditor::resized()
{
    auto b = getLocalBounds();
    auto quarterWidth = b.getWidth() / 4;
    b.removeFromLeft (quarterWidth);
    curveEditor.setBounds (b.removeFromLeft (quarterWidth * 2));
}
