#pragma once

#include <juce_data_structures/juce_data_structures.h>
#include <juce_gui_basics/juce_gui_basics.h>

struct Node
{
    juce::Point<float> endPoint;
    juce::Point<float> controlPointOne;
    juce::Point<float> controlPointTwo;
};
class CurvePositionCalculator
{
public:
    CurvePositionCalculator (juce::ValueTree curveBranch)
      : state (curveBranch)
    {
        jassert (state.getType() == id::CURVE);
        initializeState();
    }
    float getYatX (const float x)
    {
        return calculateYbetweenNodes (getLeftNode(), getRightNode(), x);
    }
private:
    juce::ValueTree state;
    juce::Array<Node> nodes;

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
    void initializeState()
    {
        for (int i = 0; i < state.getNumChildren(); i++)
            nodes.add (nodeFromIndex (i));
    }
    float lerp (const float a, const float b, const float position)
    {
        jassert (position >= 0.0f && position <= 1.0f);
        return ((1.0f - position) * a) + (b * position);
    }
    juce::Point<float> lerp (const juce::Point<float> a, const juce::Point<float> b, const float position)
    {
        jassert (position >= 0.0f && position <= 1.0f);
        return {lerp (a.x, b.x, position), lerp (a.y, b.y, position)};
    }

    const Node* getLeftNode (float x)
    {
        for (int i = 0; i < nodes.size(); i++)
            if (nodes[i].endPoint.getX() > x)
                return &nodes[i - 1];
    }
    const Node* getRightNode (float x)
    {
        for (int i = 0; i < nodes.size(); i++)
            if (nodes[i].endPoint.getX() > x)
                return &nodes[i];
    }
    float calculateYBetweenNodes (const Node* leftNode, const Node* rightNode, const float position)
    {
        jassert (position >= leftNode->endPoint.getX() &&
                 position < rightNode->endPoint.getX());
        float normalizedPosition = juce::jmap (position, 
                                               leftNode->endPoint.getX(), 
                                               rightNode->endPoint.getX(), 
                                               0.0f, 1.0f);
        auto a = lerp (leftNode->endPoint, leftNode->controlPointTwo, normalizedPosition);
        auto b = lerp (leftNode->controlPointTwo, rightNode->controlPointOne, normalizedPosition);
        auto c = lerp (rightNode->controlPointOne, rightNode->endPoint, normalizedPosition);
        auto d = lerp (a, b, normalizedPosition);
        auto e = lerp (b, c, normalizedPosition);
        return lerp (d, e, normalizedPosition).getY();
    }
};