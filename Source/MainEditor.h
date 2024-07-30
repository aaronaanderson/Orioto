#pragma once

#include "MainProcessor.h"
#include "Interface/LookAndFeel.hpp"
#include "Interface/CurveEditor.h"
#include "Interface/SineView.h"
#include "Interface/ControlPanel.h"

//==============================================================================
class MainEditor final : public juce::AudioProcessorEditor,
                         private juce::KeyListener
{
public:
    explicit MainEditor (MainProcessor&);
    ~MainEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    MainProcessor& processorRef;
    OriotoLookAndFeel lookAndFeel;
    juce::UndoManager& undoManager;

    oi::Curve curveEditor;
    oi::SineView sineView;
    oi::ControlPanel controlPanel;

    bool keyPressed (const juce::KeyPress& key,
                     juce::Component* originatingComponent) override;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainEditor)
};
