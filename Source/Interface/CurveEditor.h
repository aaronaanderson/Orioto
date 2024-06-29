#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "../Identifiers.h"
#include "LookAndFeel.hpp"

namespace oi
{
class ControlPoint
{
public:

private:
};

class Node : public juce::Component
{
public: 
    Node (juce::ValueTree NodeBranch) 
      : state (NodeBranch)
    {
        jassert (state.getType() == id::NODE);
        initializeState();
    }
    void paint (juce::Graphics& g) override
    {
        auto laf = dynamic_cast<OriotoLookAndFeel*> (&getLookAndFeel());
        jassert (laf != nullptr);

        g.setColour (laf->getAccentColour());
        auto b = getLocalBounds();
        g.fillEllipse (b.toFloat());

        g.setColour (laf->getBackgroundColour());
        g.fillEllipse (b.reduced (2).toFloat());
    }
    void mouseEnter (const juce::MouseEvent& event) override 
    {
        juce::ignoreUnused (event);
        mouseOver = true;
    }
    void mouseExit (const juce::MouseEvent& event) override 
    {
        juce::ignoreUnused (event);
        mouseOver = false;
    }
    void mouseDrag (const juce::MouseEvent& event) override { juce::ignoreUnused (event); }
    void mouseDown (const juce::MouseEvent& event) override { juce::ignoreUnused (event); }
    const juce::Point<float> getPosition() const { return position; }
private:
    juce::ValueTree state;
    juce::Point<float> position;
    bool mouseOver = false;

    void initializeState()
    {
        position.setX (state.getProperty (id::x));
        position.setY (state.getProperty (id::y));
    }
};
class Curve : public juce::Component
{
public:
    Curve (juce::ValueTree curveBranch) 
      : state (curveBranch)
    {
        jassert (state.getType() == id::CURVE);
        for (int i = 0; i < state.getNumChildren(); i++)
        {
            Nodes.add (new Node (state.getChild (i)));
        }
    }
    void paint (juce::Graphics& g) override
    {
        auto laf = dynamic_cast<OriotoLookAndFeel*> (&getLookAndFeel());
        jassert (laf != nullptr);
        g.fillAll (laf->getBaseColour());

        g.setColour (laf->getAccentColour());
        juce::Path curvePath;
        auto initialPoint = scaleToBounds (Nodes.getFirst()->getPosition());
        juce::Point<float> terminalPoint;
        for (const auto* Node : Nodes)
        {
            terminalPoint = scaleToBounds (Node->getPosition());
            auto l = juce::Line<float>(initialPoint, terminalPoint);
            curvePath.addLineSegment (l, 2.0f);
            initialPoint = terminalPoint;
        }
        g.strokePath (curvePath.createPathWithRoundedCorners (2.0f), juce::PathStrokeType (2.0f));
    }
private:
    juce::ValueTree state;
    juce::OwnedArray<Node> Nodes;

    juce::Point<float> scaleToBounds (juce::Point<float> normalized)
    {
        auto output = juce::Point<float>();
        auto b = getLocalBounds();
        output.setX (juce::jmap (normalized.getX(), -1.0f, 1.0f, 0.0f, static_cast<float> (b.getWidth())));
        output.setY (juce::jmap (normalized.getY(), -1.0f, 1.0f, static_cast<float> (b.getHeight()), 0.0f));
        return output;
    }
};
}