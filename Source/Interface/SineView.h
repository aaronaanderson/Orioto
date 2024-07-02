#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_data_structures/juce_data_structures.h>
#include "../Identifiers.h"
#include "LookAndFeel.hpp"

namespace oi
{
class SineView : public juce::Component
{
public:
    SineView (juce::ValueTree curveBranch)
      : state (curveBranch)
    {
        jassert (state.getType() == id::CURVE);
    }

    void paint (juce::Graphics& g) override
    {
        auto laf = dynamic_cast<OriotoLookAndFeel*> (&getLookAndFeel());
        jassert (laf != nullptr);
        g.fillAll (laf->getBaseColour());

        g.setColour (laf->getBackgroundColour());
        auto zeroLine = juce::Line<float> ({0.0f, getHeight() / 2.0f}, 
                                           {static_cast<float> (getWidth()), getHeight() / 2.0f});
        g.drawLine (zeroLine);

        juce::Path sineCurve;
        juce::Point<float> previousPoint = {0.0f, getHeight() / 2.0f};
        juce::Point<float> nextPoint;
        sineCurve.startNewSubPath (previousPoint);
        for (int x = 0; x < getWidth(); x += 10)
        {
            nextPoint = {static_cast<float> (x), normalToY (std::sin (xToPhase (x)))};
            juce::Line<float> segment = {previousPoint, nextPoint};
            sineCurve.addLineSegment (segment, 1.0f);
            previousPoint = nextPoint;
        }
        nextPoint = {static_cast<float> (getWidth()), normalToY (std::sin (xToPhase (getWidth())))};
        juce::Line<float> finalSegment = {previousPoint, nextPoint};
        sineCurve.addLineSegment (finalSegment, 1.0f);
        g.strokePath (sineCurve.createPathWithRoundedCorners (2.0f), juce::PathStrokeType (2.0f));
    }

private:
    juce::ValueTree state;

    float xToPhase (int x) 
    {
        return juce::jmap (static_cast<float> (x), 
                           0.0f, static_cast<float> (getWidth()), 
                           0.0f, juce::MathConstants<float>::twoPi);
    }
    float normalToY (float normal)
    {
        return juce::jmap (normal, -1.0f, 1.0f, static_cast<float> (getHeight()), 0.0f);
    }
};
}