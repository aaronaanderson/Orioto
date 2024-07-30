#include "MainProcessor.h"
#include "MainEditor.h"

//==============================================================================
MainEditor::MainEditor (MainProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p), 
      undoManager (processorRef.getUndoManager()),
      curveEditor (processorRef.getState().getChildWithName (id::CURVE), processorRef.getUndoManager()),
      sineView (processorRef.getState().getChildWithName (id::CURVE)), 
      controlPanel (processorRef.getValueTreeState())
{
    juce::ignoreUnused (processorRef);

    setLookAndFeel (&lookAndFeel);
    
    addAndMakeVisible (curveEditor);
    addAndMakeVisible (sineView);
    addAndMakeVisible (controlPanel);

    setWantsKeyboardFocus (true);
    addKeyListener (this);

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

bool MainEditor::keyPressed (const juce::KeyPress& key,
                             juce::Component* originatingComponent)
{
    juce::ignoreUnused (originatingComponent);
    if(key.getModifiers().isCommandDown() && (key.getKeyCode() == 'z' || key.getKeyCode() == 'Z'))
    {
        undoManager.undo();
    } else if(key.getModifiers().isCommandDown() && ((key.getKeyCode() == 'y' || key.getKeyCode() == 'Y') ||
                                                     (key.getKeyCode() == 'r' || key.getKeyCode() == 'R')))
    {
        undoManager.redo();
    }
    return false;
}