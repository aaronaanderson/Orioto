#include "MainProcessor.h"
#include "MainEditor.h"

//==============================================================================
MainEditor::MainEditor (MainProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p)
{
    juce::ignoreUnused (processorRef);

    setLookAndFeel (&lookAndFeel);

    setResizable (true, false);
    setResizeLimits (300, 200, 2400, 1600);
    setSize (900, 600);
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
}
