#pragma once

#include <juce_data_structures/juce_data_structures.h>
#include <juce_gui_basics/juce_gui_basics.h>

#include "Identifiers.h"
struct NodeBranch
{
    static const juce::ValueTree create(juce::Point<float> position)
    {
        juce::ValueTree nodeBranch (id::NODE);
        nodeBranch.setProperty (id::x, position.x, nullptr);
        nodeBranch.setProperty (id::y, position.y, nullptr);
        return nodeBranch;
    }
};

struct CurveBranch
{
    static const juce::ValueTree create()
    {
        juce::ValueTree curveBranch (id::CURVE);
        curveBranch.addChild (NodeBranch::create ({-1.0f, -1.0f}), -1, nullptr);
        curveBranch.addChild (NodeBranch::create ({0.0f, 0.0f}), -1, nullptr);
        curveBranch.addChild (NodeBranch::create ({1.0f, 1.0f}), -1, nullptr);

        return curveBranch;
    }
};
struct DefaultTree
{
    static const juce::ValueTree create()
    {
        auto tree = juce::ValueTree (id::ORIOTO);
        tree.addChild (CurveBranch::create(), -1, nullptr);

        return tree;
    }
};