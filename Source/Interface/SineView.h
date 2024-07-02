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
        calculateTransferFunction();
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

        int skip = 10;
        for (int x = 0; x < getWidth(); x += skip)
        {
            // paint background sine
            g.setColour (laf->getBackgroundColour());
            nextPoint = {static_cast<float> (x), normalToY (std::sin (xToPhase (x)))};
            juce::Line<float> segment = {previousPoint, nextPoint};
            sineCurve.addLineSegment (segment, 1.0f);
            previousPoint = nextPoint;
        }
        nextPoint = {static_cast<float> (getWidth()), normalToY (std::sin (xToPhase (getWidth())))};
        juce::Line<float> finalSegment = {previousPoint, nextPoint};
        sineCurve.addLineSegment (finalSegment, 1.0f);
        g.strokePath (sineCurve.createPathWithRoundedCorners (2.0f), juce::PathStrokeType (2.0f));

        // paint output
        juce::Path outputPath;
        previousPoint = {0.0f, normalToY (transferFunction.getClippedLine ({{xToNormal (0), -1.0f},
                                                                            {xToNormal (0), 1.0f}}, 
                                                                            false).getEndY())};
        outputPath.startNewSubPath (previousPoint);
        for (int x = 0; x < getWidth(); x += skip)
        {
            g.setColour (laf->getAccentColour());
            auto l = transferFunction.getClippedLine ({{xToNormal (x), -1.0f},
                                                       {xToNormal (x), 1.0f}}, 
                                                        false);
            nextPoint = {static_cast<float> (x), normalToY (l.getEndY())};
            outputPath.addLineSegment ({previousPoint, nextPoint}, 1.0f);
            previousPoint = nextPoint;
        }
        nextPoint = {static_cast<float> (getWidth()), 
                     normalToY (transferFunction.getClippedLine ({{xToNormal (getWidth()), -1.0f},
                                                                  {xToNormal (getWidth()), 1.0f}}, 
                                                                   false).getEndY())};
        finalSegment = {previousPoint, nextPoint};
        g.strokePath (outputPath.createPathWithRoundedCorners (2.0f), juce::PathStrokeType (2.0f));
    }

private:
    juce::ValueTree state;
    juce::Path transferFunction;

    float xToPhase (int x) 
    {
        return juce::jmap (static_cast<float> (x), 
                           0.0f, static_cast<float> (getWidth()), 
                           0.0f, juce::MathConstants<float>::twoPi);
    }
    float xToNormal (int x)
    {
        return juce::jmap (static_cast<float> (x), 
                           0.0f, static_cast<float> (getWidth()), 
                           -1.0f, 1.0f);
    }
    float normalToY (float normal)
    {
        return juce::jmap (normal, -1.0f, 1.0f, static_cast<float> (getHeight()), 0.0f);
    }
    float indexToNormal (int index)
    {
        return index / static_cast<float> (getWidth());
    }
    struct Node
    {
        juce::Point<float> endPoint;
        juce::Point<float> controlPointOne;
        juce::Point<float> controlPointTwo;
    };
    Node nodeFromIndex (int index)
    {
        auto nodeBranch = state.getChild (index);
        Node node;
        auto endPoint = nodeBranch.getChildWithName (id::endPoint);
        node.endPoint = {static_cast<float> (endPoint.getProperty (id::x)), 
                         static_cast<float> (endPoint.getProperty (id::y))};
        auto controlPointOne = nodeBranch.getChildWithName (id::controlPoint1);
        node.controlPointOne = {static_cast<float> (controlPointOne.getProperty (id::x)), 
                                static_cast<float> (controlPointOne.getProperty (id::y))};
        auto controlPointTwo = nodeBranch.getChildWithName (id::controlPoint2);
        node.controlPointTwo = {static_cast<float> (controlPointTwo.getProperty (id::x)), 
                                static_cast<float> (controlPointTwo.getProperty (id::y))};
        return node;
    }

    void calculateTransferFunction()
    {
        transferFunction.clear();
        auto firstNode = nodeFromIndex (0);
        transferFunction.startNewSubPath (firstNode.endPoint);
        juce::Point<float> previousControlPoint = firstNode.endPoint + firstNode.controlPointTwo;
        for (int i = 1; i < state.getNumChildren(); i++)
        {
            auto nextNode = nodeFromIndex (i);
            transferFunction.cubicTo (previousControlPoint, 
                                      nextNode.endPoint + nextNode.controlPointOne, 
                                      nextNode.endPoint);
            previousControlPoint = nextNode.endPoint + nextNode.controlPointOne;
        }

        // fill shape to find intersections
        transferFunction.lineTo ({1.0f, -2.0f});
        transferFunction.lineTo ({-1.0f, -2.0f});
        transferFunction.lineTo (firstNode.endPoint);
    }
};
}