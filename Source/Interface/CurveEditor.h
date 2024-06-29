#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "../Identifiers.h"
#include "LookAndFeel.hpp"

namespace oi
{
class DraggablePoint : public juce::Component
{
public: 
    DraggablePoint (juce::ValueTree NodeBranch, juce::UndoManager& um) 
      : state (NodeBranch), 
        undoManager (um),
        x (state, id::x, &undoManager), 
        y (state, id::y, &undoManager)
    {
        jassert (state.getType() == id::endPoint ||
                 state.getType() == id::controlPoint1 ||
                 state.getType() == id::controlPoint2);
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
        informListeners (event);
    }
    void mouseDown (const juce::MouseEvent& event) override { juce::ignoreUnused (event); }
    const juce::Point<float> getPosition() const { return juce::Point<float> (x.get(), y.get()); }
    void setPosition (juce::Point<float> newPosition)
    {
        // position = newPosition;
        x.setValue (newPosition.getX(), &undoManager);
        y.setValue (newPosition.getY(), &undoManager);
    }
    struct Listener
    {
        virtual void onDrag (DraggablePoint*, const juce::MouseEvent&) = 0;
    };
    void addListener (Listener* newListener) { listeners.add (newListener); }
protected:
    juce::ValueTree state;
    juce::UndoManager& undoManager;
    juce::CachedValue<float> x,y;

    bool mouseOver = false;
    juce::Array<Listener*> listeners;
    void informListeners(const juce::MouseEvent& e) { for (auto l : listeners) l->onDrag (this, e); }
};

class EndPoint : public DraggablePoint
{
public:
    EndPoint (juce::ValueTree endpointBranch, juce::UndoManager& um)
      : DraggablePoint (endpointBranch, um)
    {
        jassert (state.getType() == id::endPoint);
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
};
class Node;
class ControlPoint : public DraggablePoint
{
public:
    ControlPoint (Node& node, juce::ValueTree controlPointBranch, juce::UndoManager& um)
      : DraggablePoint (controlPointBranch, um), 
        parentNode (node)
    {
        jassert (state.getType() == id::controlPoint1 || 
                 state.getType() == id::controlPoint2);
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
    Node& getNode() { return parentNode; }
private:
    Node& parentNode;
};
static juce::Point<float> scaleToBounds (const juce::Point<float> normalized, const juce::Rectangle<int> localBounds)
{
    auto output = juce::Point<float>();
    auto b = localBounds;
    output.setX (juce::jmap (normalized.getX(), -1.0f, 1.0f, 0.0f, static_cast<float> (b.getWidth())));
    output.setY (juce::jmap (normalized.getY(), -1.0f, 1.0f, static_cast<float> (b.getHeight()), 0.0f));
    return output;
}
static juce::Point<float> scaleFromBounds (const juce::Point<float> boundsPosition, const juce::Rectangle<int> localBounds)
{
    auto output = juce::Point<float>();
    auto b = localBounds;
    output.setX (juce::jmap (boundsPosition.getX(), 0.0f, static_cast<float> (b.getWidth()), -1.0f, 1.0f));
    output.setY (juce::jmap (boundsPosition.getY(), static_cast<float> (b.getHeight()), 0.0f, -1.0f, 1.0f));
    return output;
}
class Node : public juce::Component, 
             private juce::ValueTree::Listener
{
public:
    Node (juce::ValueTree nodeBranch, juce::UndoManager& um)
      : state (nodeBranch), 
        undoManager (um), 
        endPoint (nodeBranch.getChildWithName (id::endPoint), undoManager), 
        controlPointOne (*this, nodeBranch.getChildWithName (id::controlPoint1), undoManager), 
        controlPointTwo (*this, nodeBranch.getChildWithName (id::controlPoint2), undoManager)
    {
        jassert (state.getType() == id::NODE);
        addAndMakeVisible (endPoint);
        addAndMakeVisible (controlPointOne);
        addAndMakeVisible (controlPointTwo);
        state.addListener (this);
    }
    void resized() override
    {
        juce::Rectangle<int> ep (20, 20);
        endPoint.setBounds (ep.withCentre (scaleToBounds (endPoint.getPosition(), getLocalBounds()).toInt()));
        juce::Rectangle<int> cp (12, 12);
        controlPointOne.setBounds (cp.withCentre (scaleToBounds (endPoint.getPosition() + controlPointOne.getPosition(), getLocalBounds()).toInt()));
        controlPointTwo.setBounds (cp.withCentre (scaleToBounds (endPoint.getPosition() + controlPointTwo.getPosition(), getLocalBounds()).toInt()));
    }
    void paint (juce::Graphics& g) override
    {
        auto laf = dynamic_cast<OriotoLookAndFeel*> (&getLookAndFeel());
        jassert (laf != nullptr);

        g.setColour (laf->getAccentColour());
        juce::Line<float> toControlPointOne (scaleToBounds (endPoint.getPosition() + controlPointOne.getPosition(), getLocalBounds()), 
                                             scaleToBounds (endPoint.getPosition(), getLocalBounds()));
        g.drawLine (toControlPointOne, 1.0f);  
        juce::Line<float> toControlPointTwo (scaleToBounds (endPoint.getPosition() + controlPointTwo.getPosition(), getLocalBounds()), 
                                             scaleToBounds (endPoint.getPosition(), getLocalBounds()));
        g.drawLine (toControlPointTwo, 1.0f);

    }
    bool hitTest(int x, int y) override 
    { 
        juce::Point<int> hit (x, y);
        if (endPoint.getBounds().contains (hit)) return true;
        if (controlPointOne.getBounds().contains (hit)) return true;
        if (controlPointTwo.getBounds().contains (hit)) return true;
        return false;
    }
    void addListener (DraggablePoint::Listener* l)
    {
        endPoint.addListener (l);
        controlPointOne.addListener (l);
        controlPointTwo.addListener (l);
    }
    const juce::Point<float> getPosition() const { return endPoint.getPosition(); }
    void setPosition (juce::Point<float> position)
    {
        endPoint.setPosition (position);
    }
    EndPoint& getEndPoint() { return endPoint; }
private:
    juce::ValueTree state;
    juce::UndoManager& undoManager;

    EndPoint endPoint;
    ControlPoint controlPointOne, controlPointTwo;

    void valueTreePropertyChanged (juce::ValueTree& tree,
                                   const juce::Identifier& property) override
    {
        juce::ignoreUnused (property);
        if (tree.getType() == id::endPoint) resized();
        if (tree.getType() == id::controlPoint1) resized();
        if (tree.getType() == id::controlPoint2) resized();
    }
};
class Curve : public juce::Component, 
              private DraggablePoint::Listener
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
            n->addListener (this);
    }
    void paint (juce::Graphics& g) override
    {
        auto laf = dynamic_cast<OriotoLookAndFeel*> (&getLookAndFeel());
        jassert (laf != nullptr);
        g.fillAll (laf->getBaseColour());

        g.setColour (laf->getAccentColour());
        juce::Path curvePath;
        auto initialPoint = scaleToBounds (nodes.getFirst()->getPosition(), getLocalBounds());
        juce::Point<float> terminalPoint;
        for (const auto* node : nodes)
        {
            terminalPoint = scaleToBounds (node->getPosition(), getLocalBounds());
            auto l = juce::Line<float>(initialPoint, terminalPoint);
            curvePath.addLineSegment (l, 2.0f);
            initialPoint = terminalPoint;
        }
        g.strokePath (curvePath.createPathWithRoundedCorners (2.0f), juce::PathStrokeType (2.0f));
    }
    void resized() override
    {
        // juce::Rectangle<int> n (20, 20);
        // for (auto* node : nodes)
        //     node->setBounds (n.withCentre (scaleToBounds (node->getPosition(), getLocalBounds()).toInt()));
        for (auto* node : nodes)
            node->setBounds (getLocalBounds());
    }
private:
    juce::ValueTree state;
    juce::UndoManager& undoManager;
    juce::OwnedArray<Node> nodes;
    
    void onDrag (DraggablePoint* draggablePoint, const juce::MouseEvent& event) override
    {
        if (dynamic_cast<EndPoint*> (draggablePoint) != nullptr)
        {
            auto adjustedEvent = event.getEventRelativeTo (this);
            auto newPosition = scaleFromBounds (adjustedEvent.getPosition().toFloat(), getLocalBounds());
            if (draggablePoint == &nodes.getFirst()->getEndPoint() || 
                draggablePoint == &nodes.getLast()->getEndPoint()) 
            {
                newPosition.setX (draggablePoint->getPosition().getX());
            }
    
            newPosition.x = juce::jlimit (-1.0f, 1.0f, newPosition.getX());
            newPosition.y = juce::jlimit (-1.0f, 1.0f, newPosition.getY());
            draggablePoint->setPosition (newPosition);
            repaint(); 
        }else if (dynamic_cast<ControlPoint*> (draggablePoint) != nullptr)
        {
            auto adjustedEvent = event.getEventRelativeTo (this);
            auto newPosition = scaleFromBounds (adjustedEvent.getPosition().toFloat(), getLocalBounds());
            
            auto* controlPoint = dynamic_cast<ControlPoint*> (draggablePoint);
            auto& node = controlPoint->getNode();
            newPosition -= node.getEndPoint().getPosition();
            draggablePoint->setPosition (newPosition);
            repaint();         
        }
    }
};
}