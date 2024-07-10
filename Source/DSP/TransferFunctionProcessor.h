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
        transferFunction.reset (new juce::dsp::LookupTable<float>());
        workingBuffer.reset (new juce::dsp::LookupTable<float>());
        updateTransferFunction();
    }
    float lookUp (const float value)
    {
        jassert (value <= 1.0f);
        jassert (value >= -1.0f);
        auto clampedValue = juce::jlimit (-1.0f, 1.0f, value);
        float index = normalizedToIndex (clampedValue);
        return transferFunction->get (index);
    }
    void reset() { updateTransferFunction(); }
private:
    juce::ValueTree state;
    static const size_t numPoints = 32 * 1;
    std::unique_ptr<juce::dsp::LookupTable<float>> transferFunction;
    std::unique_ptr<juce::dsp::LookupTable<float>> workingBuffer;
    CurvePositionCalculator cpc;

    void updateTransferFunction()
    {
        cpc.reset (state);
        auto f = [&](size_t i){ return cpc.getYatX (indexToNormalized (i)); };
        workingBuffer->initialise (f, numPoints);
        transferFunction.swap (workingBuffer);
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
        dryWetMix.reset (spec.sampleRate, 0.01);
    }
    void reset() noexcept 
    {
        transferFunction.reset();
    }
    
    void setMix (float newMix)
    {
        jassert (newMix >= 0.0f && newMix <= 1.0f);
        dryWetMix.setTargetValue (newMix);
    }

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

        for (size_t i = 0; i < numSamples; ++i)
        {
            auto mix = dryWetMix.getNextValue();
            for (size_t channel = 0; channel < numChannels; ++channel)
            {
                auto* inputSamples = inputBlock.getChannelPointer (channel);
                auto* outputSamples = outputBlock.getChannelPointer (channel);
                
                outputSamples[i] = (mix * processSample (inputSamples[i])) + 
                                   ((1.0f - mix) * inputSamples[i]);
            }
        }
    }
    FloatType processSample (FloatType inputValue)
    {
        return transferFunction.lookUp (inputValue);
    }
private:
    TransferFunction transferFunction;
    juce::SmoothedValue<float> dryWetMix;
};

}