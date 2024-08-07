#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include "DSP/TransferFunctionProcessor.h"
//==============================================================================
class MainProcessor final : public juce::AudioProcessor
{
public:
    //==============================================================================
    MainProcessor();
    ~MainProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    using AudioProcessor::processBlock;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState& getValueTreeState() { return valueTreeState; }
    juce::ValueTree& getState() { return valueTreeState.state; }
    juce::UndoManager& getUndoManager() { return undoManager; }
private:
    juce::AudioProcessorValueTreeState valueTreeState;
    juce::UndoManager undoManager;
    double sampleRate;
    std::unique_ptr<op::TransferFunctionProcessor<float>> transferFunctionProcessor;
    juce::dsp::Oversampling<float> overSampler;

    juce::dsp::ProcessorChain<juce::dsp::Gain<float>,
                              juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, 
                                                             juce::dsp::IIR::Coefficients<float>>,
                              juce::dsp::Compressor<float>> inputChain;
    
    juce::dsp::ProcessorChain<juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>,
                                                             juce::dsp::IIR::Coefficients<float>>,
                              juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, 
                                                             juce::dsp::IIR::Coefficients<float>>,
                              juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, 
                                                             juce::dsp::IIR::Coefficients<float>>,
                              juce::dsp::Compressor<float>,
                              juce::dsp::Gain<float>> outputChain;
    
    struct FilterSettings
    {
        float frequency = 200.0f;
        float gain = 0.0f;
        float q = 1.0f;
    };
    struct SmoothFilterSettings
    {
        SmoothFilterSettings()
        {
            int numBuffers = 8;
            frequency.reset (numBuffers);
            gain.reset (numBuffers);
            q.reset (numBuffers);
        }
        const FilterSettings getSettings()
        {
            return {frequency.getNextValue(), gain.getNextValue(), q.getNextValue()};
        }
        void setTarget (const FilterSettings settings)
        {
            frequency.setTargetValue (settings.frequency);
            gain.setTargetValue (settings.gain);
            q.setTargetValue (settings.q);
        }
    private:
        juce::SmoothedValue<float> frequency;
        juce::SmoothedValue<float> gain;
        juce::SmoothedValue<float> q;
    };
    SmoothFilterSettings smoothFilterSettings;
    double phase = 0;
    double phaseIncrement = 0.001;

    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    void syncValueTreeNotifyListeners (const juce::ValueTree& source, juce::ValueTree& destination)
    {
        const int numProperties = source.getNumProperties();
        for (int i = 0; i < numProperties; ++i)
        {
            auto propertyName = source.getPropertyName (i);
    
            if (destination.hasProperty (propertyName))
                destination.setProperty (propertyName, source.getProperty (propertyName), nullptr);
        }
    
        // for (const auto& child : source)
        for (int i = 0; i < source.getNumChildren(); i++)
        {
            auto child = source.getChild (i);
            // auto childType = source.getChild (i).getType();
            auto childInDestination = destination.getChild (i);
    
            if (childInDestination.isValid())
                syncValueTreeNotifyListeners (child, childInDestination);
        }       
    }
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainProcessor)
};
