#pragma once 

#include <juce_data_structures/juce_data_structures.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_dsp/juce_dsp.h>
#include "../Identifiers.h"
#include "../CurvePositionCalculator.h"

namespace op
{
class TransferFunction : private juce::ValueTree::Listener
{
public:
    TransferFunction (juce::ValueTree curveBranch)
      : state (curveBranch), 
        cpc (curveBranch)
    {
        jassert (state.getType() == id::CURVE);
        state.addListener (this);
        updateTransferFunction();
    }
    float lookUp (const float value)
    {
        auto clampedValue = juce::jlimit (-1.0f, 1.0f, value);
        float index = normalizedToIndex (clampedValue);
        return transferFunction.get (index);
    }
private:
    juce::ValueTree state;
    juce::dsp::LookupTable<float> transferFunction;
    static const size_t numPoints = 1024 * 1;
    CurvePositionCalculator cpc;

    void updateTransferFunction()
    {
        cpc.reset (state);
        auto f = [&](size_t i){ return cpc.getYatX (indexToNormalized (i)); };
        transferFunction.initialise (f, numPoints);

    }
    float indexToNormalized (size_t index)
    {
        return juce::jmap (static_cast<float> (index),
                           0.0f, static_cast<float> (numPoints - 1),
                           -1.0f, 1.0f);
    }
    float normalizedToIndex (float normalized)
    {
        return juce::jmap (normalized,
                           -1.0f, 1.0f, 
                           0.0f, static_cast<float> (numPoints - 1));
    }
    void valueTreePropertyChanged (juce::ValueTree& tree,
                                   const juce::Identifier& property) override 
    {
        juce::ignoreUnused (property);
        if (tree.getType() == id::endPoint ||
            tree.getType() == id::controlPoint1 ||
            tree.getType() == id::controlPoint2)
        {
            updateTransferFunction();
        }
    }
};

template <typename FloatType>
class TransferFunctionProcessor
{
public:
    TransferFunctionProcessor (juce::ValueTree curveBranch)
      : transferFunction (curveBranch)
    {}

    void prepare (const juce::dsp::ProcessSpec& spec) 
    {
        juce::ignoreUnused (spec);
    }
    void reset() noexcept {}
    
    template<typename ProcessContext>
    void process (const ProcessContext& context) noexcept
    {
        const auto& inputBlock = context.getInputBlock();
        auto& outputBlock      = context.getOutputBlock();
        const auto numChannels = outputBlock.getNumChannels();
        const auto numSamples  = outputBlock.getNumSamples();

        if (context.isBypassed)
        {
            outputBlock.copyFrom (inputBlock);
            return;
        }
        for (size_t channel = 0; channel < numChannels; ++channel)
        {
            auto* inputSamples = inputBlock.getChannelPointer (channel);
            auto* outputSamples = outputBlock.getChannelPointer (channel);

            for (size_t i = 0; i < numSamples; ++i)
                outputSamples[i] = processSample (inputSamples[i]);
        }
    }
    FloatType processSample (FloatType inputValue)
    {
        return transferFunction.lookUp (inputValue);
    }
private:
    TransferFunction transferFunction;
};

}