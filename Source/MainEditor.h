#pragma once

#include "MainProcessor.h"
#include "Interface/LookAndFeel.hpp"
#include "Interface/CurveEditor.h"
#include "Interface/SineView.h"

//==============================================================================
class MainEditor final : public juce::AudioProcessorEditor
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

    oi::Curve curveEditor;
    oi::SineView sineView;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainEditor)
};
