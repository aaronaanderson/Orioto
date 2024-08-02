#pragma once

#include <juce_data_structures/juce_data_structures.h>
#include <juce_gui_basics/juce_gui_basics.h>

#include "Identifiers.h"
struct NodeBranch
{
    static const juce::ValueTree create(juce::Point<float> endPoint, 
                                        juce::Point<float> controlOne, 
                                        juce::Point<float> controlTwo)
    {
        juce::ValueTree nodeBranch (id::NODE);
        
        juce::ValueTree endPointBranch (id::endPoint);
        endPointBranch.setProperty (id::x, endPoint.x, nullptr);
        endPointBranch.setProperty (id::y, endPoint.y, nullptr);
        nodeBranch.addChild (endPointBranch, -1, nullptr);

        juce::ValueTree controlOneBranch (id::controlPoint1);
        controlOneBranch.setProperty (id::x, controlOne.x, nullptr);
        controlOneBranch.setProperty (id::y, controlOne.y, nullptr);
        nodeBranch.addChild (controlOneBranch, -1, nullptr);

        juce::ValueTree controlTwoBranch (id::controlPoint2);
        controlTwoBranch.setProperty (id::x, controlTwo.x, nullptr);
        controlTwoBranch.setProperty (id::y, controlTwo.y, nullptr);
        nodeBranch.addChild (controlTwoBranch, -1, nullptr);
        return nodeBranch;
    }
};

struct CurveBranch
{
    static const juce::ValueTree create()
    {
        juce::ValueTree curveBranch (id::CURVE);

        juce::ValueTree activeBranch (id::ACTIVE_CURVE);
        activeBranch.addChild (NodeBranch::create ({-1.0f, -1.0f}, {-0.3333333333f, -0.3333333333f}, {0.3333333333f, 0.3333333333f}), -1, nullptr);
        activeBranch.addChild (NodeBranch::create ({0.0f, 0.0f}, {-0.3333333333f, -0.3333333333f}, {0.3333333333f, 0.3333333333f}), -1, nullptr);
        activeBranch.addChild (NodeBranch::create ({1.0f, 1.0f}, {-0.3333333333f, -0.3333333333f}, {0.3333333333f, 0.3333333333f}), -1, nullptr);
        activeBranch.setProperty (id::presetIndex, 0, nullptr);
        curveBranch.addChild (activeBranch, -1, nullptr);

        juce::ValueTree presetBranch (id::PRESETS);
        juce::ValueTree bypassCurve (id::CURVE);
        bypassCurve.setProperty (id::name, "Bypass", nullptr);
        bypassCurve.addChild (NodeBranch::create ({-1.0f, -1.0f}, {-0.3333333333f, -0.3333333333f}, {0.3333333333f, 0.3333333333f}), -1, nullptr);
        bypassCurve.addChild (NodeBranch::create ({0.0f, 0.0f}, {-0.3333333333f, -0.3333333333f}, {0.3333333333f, 0.3333333333f}), -1, nullptr);
        bypassCurve.addChild (NodeBranch::create ({1.0f, 1.0f}, {-0.3333333333f, -0.3333333333f}, {0.3333333333f, 0.3333333333f}), -1, nullptr);
        presetBranch.addChild (bypassCurve, -1, nullptr);
        curveBranch.addChild (presetBranch, -1, nullptr);

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