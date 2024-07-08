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
        listener->isDragging = true;
        listener->onDrag (this, event);
    }
    void mouseDown (const juce::MouseEvent& event) override { juce::ignoreUnused (event); }
    void mouseUp (const juce::MouseEvent& event) override 
    {
        juce::ignoreUnused (event);
        listener->isDragging = false;
        listener->onDragEnd();
    }
    const juce::Point<float> getPosition() const { return juce::Point<float> (x.get(), y.get()); }
    void setPosition (juce::Point<float> newPosition)
    {
        x.setValue (newPosition.getX(), &undoManager);
        y.setValue (newPosition.getY(), &undoManager);
    }
    struct Listener
    {
        virtual void onDrag (DraggablePoint*, const juce::MouseEvent&) = 0;
        virtual void onDragEnd () = 0;
        bool isDragging = false;
    };
    void setListener (Listener* newListener) { listener = newListener; }
protected:
    juce::ValueTree state;
    juce::UndoManager& undoManager;
    juce::CachedValue<float> x,y;

    bool mouseOver = false;
    Listener* listener;
};
class Node;
class EndPoint : public DraggablePoint
{
public:
    EndPoint (Node& parent, juce::ValueTree endpointBranch, juce::UndoManager& um)
      : DraggablePoint (endpointBranch, um), 
        parentNode (parent)
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
    Node& getNode() { return parentNode; }
private:
    Node& parentNode;
};

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
        endPoint (*this, nodeBranch.getChildWithName (id::endPoint), undoManager), 
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
        endPoint.setListener (l);
        controlPointOne.setListener (l);
        controlPointTwo.setListener (l);
    }
    const juce::Point<float> getPosition() const { return endPoint.getPosition(); }
    void setPosition (juce::Point<float> position)
    {
        endPoint.setPosition (position);
    }
    ControlPoint& getControlPointOne() { return controlPointOne; }
    ControlPoint& getControlPointTwo() { return controlPointTwo; }
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
        if (tree.getType() == id::controlPoint1)
        {
            if (property == id::x || property == id::y)
                controlPointTwo.setPosition (mirrorPosition (controlPointOne.getPosition()));
            
            resized();
        } 
        if (tree.getType() == id::controlPoint2)
        {
            if (property == id::x || property == id::y)
                controlPointOne.setPosition (mirrorPosition (controlPointTwo.getPosition()));

            resized();
        } 
    }

    juce::Point<float> mirrorPosition (const juce::Point<float> positionToMirror)
    {
        return {positionToMirror.getX() * -1.0f, positionToMirror.getY() * -1.0f};
    }
};
class Curve : public juce::Component, 
              private DraggablePoint::Listener,
              private juce::ValueTree::Listener
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

        state.addListener (this);
    }
    void paint (juce::Graphics& g) override
    {
        auto laf = dynamic_cast<OriotoLookAndFeel*> (&getLookAndFeel());
        jassert (laf != nullptr);
        g.fillAll (laf->getBaseColour());

        g.setColour (laf->getAccentColour());
        juce::Path curvePath;
        auto initialPoint = scaleToBounds (nodes.getFirst()->getPosition(), getLocalBounds());

        curvePath.startNewSubPath (initialPoint);
        auto previousControlPoint = scaleToBounds (nodes.getFirst()->getEndPoint().getPosition() + nodes.getFirst()->getControlPointTwo().getPosition(), getLocalBounds());
        for (int i = 1; i < nodes.size(); i++)
        {
            curvePath.cubicTo (previousControlPoint,
                               scaleToBounds (nodes[i]->getEndPoint().getPosition() + nodes[i]->getControlPointOne().getPosition(), getLocalBounds()),
                               scaleToBounds (nodes[i]->getEndPoint().getPosition(), getLocalBounds()));
            previousControlPoint = scaleToBounds (nodes[i]->getEndPoint().getPosition() + nodes[i]->getControlPointTwo().getPosition(), getLocalBounds());
        }
        g.strokePath (curvePath.createPathWithRoundedCorners (2.0f), juce::PathStrokeType (2.0f));

        if (isDragging)
        {
            g.setColour (laf->getBackgroundColour());
            auto cursorPosition = scaleToBounds (draggingPosition, getLocalBounds());
            g.drawLine (cursorPosition.getX(), 0.0f, cursorPosition.getX(), static_cast<float> (getLocalBounds().getHeight()));
            g.drawLine (0.0f, cursorPosition.getY(), static_cast<float> (getLocalBounds().getWidth()), cursorPosition.getY());
        }
    }
    void resized() override
    {
        for (auto* node : nodes)
            node->setBounds (getLocalBounds());
    }
private:
    juce::ValueTree state;
    juce::UndoManager& undoManager;
    juce::OwnedArray<Node> nodes;
    
    juce::Point<float> draggingPosition;
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
            auto* endPoint = dynamic_cast<EndPoint*> (draggablePoint);
            auto& node = endPoint->getNode();
            auto nodeIndex = nodes.indexOf (&node);
            // bind endpointX to neighboring controlPoints
            if (nodeIndex > 0 && nodeIndex < nodes.size() - 1)
            {
                // make sure endPoint doesn't pass previousNode's controlPoint
                auto* preceedingNode = nodes[nodeIndex - 1];
                auto leftBounds = preceedingNode->getEndPoint().getPosition().getX() +
                                  preceedingNode->getControlPointTwo().getPosition().getX();
                if (newPosition.getX() < leftBounds) newPosition.setX (leftBounds);

                // make sure currentNode's controlPoint doesn't pass previousNode's endPoint
                auto leftGapMax = node.getControlPointOne().getPosition().getX();
                if (newPosition.getX() - preceedingNode->getPosition().getX() < -leftGapMax) 
                    newPosition.setX (preceedingNode->getPosition().getX() + -leftGapMax);
                
                // make sure endPoint doesn't pass proceedingNode's controlPoint
                auto* proceedingNode = nodes[nodeIndex + 1];
                auto rightBounds = proceedingNode->getEndPoint().getPosition().getX() + 
                                   proceedingNode->getControlPointOne().getPosition().getX();
                if (newPosition.getX() > rightBounds) newPosition.setX (rightBounds);

                //make sure currentNodes' controlPoint doesn't pass proceedingNode's endPoint
                auto rightGapMax = node.getControlPointTwo().getPosition().getX();
                if (proceedingNode->getPosition().getX() - newPosition.getX() < rightGapMax)
                    newPosition.setX (proceedingNode->getPosition().getX() - rightGapMax);
            }
            newPosition.x = juce::jlimit (-1.0f, 1.0f, newPosition.getX());
            newPosition.y = juce::jlimit (-1.0f, 1.0f, newPosition.getY());
            draggablePoint->setPosition (newPosition);
            draggingPosition = newPosition;
            repaint(); 
        }else if (dynamic_cast<ControlPoint*> (draggablePoint) != nullptr)
        {
            auto adjustedEvent = event.getEventRelativeTo (this);
            auto newPosition = scaleFromBounds (adjustedEvent.getPosition().toFloat(), getLocalBounds());
            
            auto* controlPoint = dynamic_cast<ControlPoint*> (draggablePoint);
            auto& node = controlPoint->getNode();
            newPosition -= node.getEndPoint().getPosition();
            // don't allow control points to cross endPointX
            auto nodeIndex = nodes.indexOf (&node);
            if (controlPoint == &node.getControlPointOne())
            {
                // Don't let controlPoint pass over Endpoint
                if (newPosition.getX() > 0.0f) newPosition = {0.0f, controlPoint->getPosition().getY()};
                // Don't let controlPoint pass preceeding EndPoint
                auto* preceedingNode = nodes[nodeIndex - 1];
                float leftGapMax = node.getPosition().getX() - preceedingNode->getPosition().getX();
                if (newPosition.getX() < -leftGapMax)
                    newPosition = {-leftGapMax, newPosition.getY()};
                // Don't let mirrored controlPoint pass proceeding endPoint
                if (nodeIndex < nodes.size() - 1)
                {
                    auto* proceedingNode = nodes[nodeIndex + 1];
                    float rightGapMax = proceedingNode->getPosition().getX() - node.getPosition().getX();
                    if (-newPosition.getX() > rightGapMax)
                        newPosition.setX (-rightGapMax);
                }
            } else if (controlPoint == &node.getControlPointTwo())
            { 
                if (newPosition.getX() < 0.0f) newPosition = {0.0f, controlPoint->getPosition().getY()};

                auto* proceedingNode = nodes[nodeIndex + 1];
                float rightGapMax = proceedingNode->getPosition().getX() - node.getEndPoint().getPosition().getX();
                if (newPosition.getX() > rightGapMax)
                    newPosition = {rightGapMax, newPosition.getY()};
                
                if (nodeIndex > 0)
                {
                    auto* preceedingNode = nodes[nodeIndex - 1];
                    float leftGapMax = node.getPosition().getX() - preceedingNode->getPosition().getX();
                    if (newPosition.getX() > leftGapMax)
                        newPosition.setX (leftGapMax); 
                }
                
            }

            draggablePoint->setPosition (newPosition);
            draggingPosition = newPosition + node.getEndPoint().getPosition();
            repaint();         
        }
    }
    void onDragEnd() override { repaint(); }
    void valueTreeParentChanged (juce::ValueTree& treeWhoseParentHasChanged) override 
    {
        juce::ignoreUnused (treeWhoseParentHasChanged);
        std::cout << "Parent changed" << std::endl;
        std::cout << state.toXmlString() << std::endl;
    }
};
}