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
    Node (juce::ValueTree NodeBranch, juce::UndoManager& um) 
      : state (NodeBranch), 
        undoManager (um),
        x (state, id::x, &undoManager), 
        y (state, id::y, &undoManager)
    {
        jassert (state.getType() == id::NODE);
    }
    void paint (juce::Graphics& g) override
    {
        auto laf = dynamic_cast<OriotoLookAndFeel*> (&getLookAndFeel());
        jassert (laf != nullptr);

        g.setColour (laf->getAccentColour());
        auto b = getLocalBounds();
        g.fillEllipse (b.toFloat());

        g.setColour (laf->getBackgroundColour());
        mouseOver ? g.fillEllipse (b.reduced (4).toFloat()) : g.fillEllipse (b.reduced (2).toFloat());
    }
    void mouseEnter (const juce::MouseEvent& event) override 
    {
        juce::ignoreUnused (event);
        mouseOver = true;
        repaint();
    }
    void mouseExit (const juce::MouseEvent& event) override 
    {
        juce::ignoreUnused (event);
        mouseOver = false;
        repaint();
    }
    void mouseDrag (const juce::MouseEvent& event) override 
    { 
        juce::ignoreUnused (event); 
        listener->isDragging (this, event);
    }
    void mouseDown (const juce::MouseEvent& event) override { juce::ignoreUnused (event); }
    const juce::Point<float> getPosition() const { return juce::Point<float> (x.get(), y.get()); }
    void setPosition (juce::Point<float> newPosition)
    {
        jassert (newPosition.x >= -1.0f && newPosition.x <= 1.0f &&
                 newPosition.y >= -1.0f && newPosition.y <= 1.0f);
        // position = newPosition;
        x.setValue (newPosition.getX(), &undoManager);
        y.setValue (newPosition.getY(), &undoManager);
    }
    struct Listener
    {
        virtual void isDragging (Node*, const juce::MouseEvent&) = 0;
    };
    void setListener (Listener* newListener) { listener = newListener; }
private:
    juce::ValueTree state;
    juce::UndoManager& undoManager;
    juce::CachedValue<float> x,y;

    bool mouseOver = false;
    Listener* listener = nullptr;
};
class Curve : public juce::Component, 
              private Node::Listener
{
public:
    Curve (juce::ValueTree curveBranch, juce::UndoManager& um) 
      : state (curveBranch), 
        undoManager (um)
    {
        jassert (state.getType() == id::CURVE);
        for (int i = 0; i < state.getNumChildren(); i++)
            addAndMakeVisible (nodes.add (new Node (state.getChild (i), undoManager)));
        
        for (auto* n : nodes)
            n->setListener (this);
    }
    void paint (juce::Graphics& g) override
    {
        auto laf = dynamic_cast<OriotoLookAndFeel*> (&getLookAndFeel());
        jassert (laf != nullptr);
        g.fillAll (laf->getBaseColour());

        g.setColour (laf->getAccentColour());
        juce::Path curvePath;
        auto initialPoint = scaleToBounds (nodes.getFirst()->getPosition());
        juce::Point<float> terminalPoint;
        for (const auto* node : nodes)
        {
            terminalPoint = scaleToBounds (node->getPosition());
            auto l = juce::Line<float>(initialPoint, terminalPoint);
            curvePath.addLineSegment (l, 2.0f);
            initialPoint = terminalPoint;
        }
        g.strokePath (curvePath.createPathWithRoundedCorners (2.0f), juce::PathStrokeType (2.0f));
    }
    void resized() override
    {
        juce::Rectangle<int> n (20, 20);
        for (auto* node : nodes)
            node->setBounds (n.withCentre (scaleToBounds (node->getPosition()).toInt()));
    }
private:
    juce::ValueTree state;
    juce::UndoManager& undoManager;
    juce::OwnedArray<Node> nodes;
    
    void isDragging (Node* node, const juce::MouseEvent& event) override
    {
        auto adjustedEvent = event.getEventRelativeTo (this);
        auto newPosition = scaleFromBounds (adjustedEvent.getPosition().toFloat());
        if (node == nodes.getFirst() || node == nodes.getLast()) 
            newPosition.setX (node->getPosition().getX());

        newPosition.x = juce::jlimit (-1.0f, 1.0f, newPosition.getX());
        newPosition.y = juce::jlimit (-1.0f, 1.0f, newPosition.getY());
        node->setPosition (newPosition);
        resized(); repaint();
    }

    juce::Point<float> scaleToBounds (juce::Point<float> normalized)
    {
        auto output = juce::Point<float>();
        auto b = getLocalBounds();
        output.setX (juce::jmap (normalized.getX(), -1.0f, 1.0f, 0.0f, static_cast<float> (b.getWidth())));
        output.setY (juce::jmap (normalized.getY(), -1.0f, 1.0f, static_cast<float> (b.getHeight()), 0.0f));
        return output;
    }
    juce::Point<float> scaleFromBounds (juce::Point<float> boundsPosition)
    {
        auto output = juce::Point<float>();
        auto b = getLocalBounds();
        output.setX (juce::jmap (boundsPosition.getX(), 0.0f, static_cast<float> (b.getWidth()), -1.0f, 1.0f));
        output.setY (juce::jmap (boundsPosition.getY(), static_cast<float> (b.getHeight()), 0.0f, -1.0f, 1.0f));
        return output;
    }
};
}