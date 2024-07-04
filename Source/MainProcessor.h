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

    juce::ValueTree& getState() { return valueTreeState.state; }
    juce::UndoManager& getUndoManager() { return undoManager; }
private:
    juce::AudioProcessorValueTreeState valueTreeState;
    juce::UndoManager undoManager;
    
    std::unique_ptr<op::TransferFunctionProcessor<float>> transferFunctionProcessor;
    juce::dsp::Oversampling<float> overSampler;
    
    double phase = 0;
    double phaseIncrement = 0.001;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainProcessor)
};
