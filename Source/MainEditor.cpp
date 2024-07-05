#include "MainProcessor.h"
#include "MainEditor.h"

//==============================================================================
MainEditor::MainEditor (MainProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p), 
      curveEditor (processorRef.getState().getChildWithName (id::CURVE), processorRef.getUndoManager()),
      sineView (processorRef.getState().getChildWithName (id::CURVE)), 
      controlPanel (processorRef.getValueTreeState())
{
    juce::ignoreUnused (processorRef);

    setLookAndFeel (&lookAndFeel);
    
    addAndMakeVisible (curveEditor);
    addAndMakeVisible (sineView);
    addAndMakeVisible (controlPanel);

    setResizable (true, true);
    setResizeLimits (300, 200, 2400, 1600);
    setSize (800, 600);
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
    auto thirdWidth = b.getWidth() / 3;
    auto controlBounds = b.removeFromLeft (thirdWidth);
    auto viewBounds = b.removeFromLeft (thirdWidth * 2);
    sineView.setBounds (viewBounds.removeFromBottom (viewBounds.getHeight() / 5).reduced (2));
    curveEditor.setBounds (viewBounds.reduced (2));

    controlPanel.setBounds (controlBounds);
}
