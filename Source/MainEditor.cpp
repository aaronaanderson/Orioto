#include "MainProcessor.h"
#include "MainEditor.h"

//==============================================================================
MainEditor::MainEditor (MainProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p), 
      transferFunction (processorRef.getTransferFunction())
{
    juce::ignoreUnused (processorRef);

    setLookAndFeel (&lookAndFeel);

    addAndMakeVisible (transferFunction);

    setResizable (true, false);
    setResizeLimits (300, 200, 2400, 1600);
    setSize (1200, 600);
}

MainEditor::~MainEditor()
{
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
    transferFunction.setBounds (b.removeFromLeft (quarterWidth * 2));
}
