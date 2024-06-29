#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_dsp/juce_dsp.h>

namespace oi
{

class TransferFunctionComponent : public juce::Component
{
public:
    TransferFunctionComponent ()
    {

    }
    void paint (juce::Graphics& g) override
    {
        auto b = getLocalBounds();
        g.fillAll (getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId).brighter (0.5f));
        g.setColour (getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId).darker (0.5f));
        g.drawLine (0.0f, 
                    static_cast<float> (b.getHeight()), 
                    static_cast<float> (b.getWidth()), 
                    0.0f, 
                    2.0f);
        
        // auto laf = dynamic_cast<OriotoLookAndFeel*> (&getLookAndFeel());
        // jassert (laf != nullptr);
        // g.setColour (laf->getAccentColour());

        // juce::Path transferFunctionPath;
        // auto initialPoint = juce::Point<float> 
        //     (
        //         0.0f, 
        //         juce::jmap<float> (transferFunction[0], -1.0f, 1.0f, static_cast<float> (b.getHeight()), 0.0f)
        //     ); 
        // juce::Point<float> terminalPoint;
        // // transferFunctionPath.add
        // for (unsigned long i = 0; i < transferFunction.getNumPoints() - 1; i += 20)
        // {
        //     terminalPoint = juce::Point<float> 
        //         (
        //             juce::jmap<float> (static_cast<float> (i + 1), 0.0f, static_cast<float> (transferFunction.getNumPoints()), 0.0f, static_cast<float> (b.getWidth())),
        //             juce::jmap<float> (transferFunction[static_cast<float> (i + 1)], -1.0f, 1.0f, static_cast<float> (b.getHeight()), 0.0f)
        //         );
        //     auto l = juce::Line<float>(initialPoint, terminalPoint);
        //     transferFunctionPath.addLineSegment (l, 2.0f);
        //     initialPoint = terminalPoint;
        // }
        // auto l = juce::Line<float> (terminalPoint.getX(), 
        //                             terminalPoint.getY(), 
        //                             static_cast<float> (b.getWidth()), 
        //                             juce::jmap<float> (transferFunction[static_cast<float> (transferFunction.getNumPoints() - 1)], -1.0f, 1.0f, static_cast<float> (b.getHeight()), 0.0f));
        // transferFunctionPath.addLineSegment (l, 2.0f);
        // g.strokePath (transferFunctionPath.createPathWithRoundedCorners (2.0f), juce::PathStrokeType (2.0f));
    }
private:
    // const juce::dsp::LookupTable<float>& transferFunction;
};
}