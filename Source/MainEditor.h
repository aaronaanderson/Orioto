#pragma once

#include "MainProcessor.h"
#include "Interface/LookAndFeel.hpp"
#include "Interface/TransferFunction.hpp"

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

    oi::TransferFunctionComponent transferFunction;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainEditor)
};
