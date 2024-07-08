#include "MainProcessor.h"
#include "MainEditor.h"
#include "Identifiers.h"
#include "DefaultTreeGenerator.h"
#include "Parameters.h"

//==============================================================================
MainProcessor::MainProcessor()
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ), 
      valueTreeState (*this, &undoManager, id::ORIOTO, createParameterLayout()), 
      overSampler (2, 3, juce::dsp::Oversampling<float>::FilterType::filterHalfBandPolyphaseIIR)
{
    valueTreeState.state.addChild (CurveBranch::create(), -1, nullptr);
    transferFunctionProcessor = std::make_unique<op::TransferFunctionProcessor<float>> (getState().getChildWithName (id::CURVE));
}

MainProcessor::~MainProcessor()
{
}

//==============================================================================
const juce::String MainProcessor::getName() const
{
    return JucePlugin_Name;
}

bool MainProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool MainProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool MainProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double MainProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int MainProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int MainProcessor::getCurrentProgram()
{
    return 0;
}

void MainProcessor::setCurrentProgram (int index)
{
    juce::ignoreUnused (index);
}

const juce::String MainProcessor::getProgramName (int index)
{
    juce::ignoreUnused (index);
    return {};
}

void MainProcessor::changeProgramName (int index, const juce::String& newName)
{
    juce::ignoreUnused (index, newName);
}

//==============================================================================
void MainProcessor::prepareToPlay (double sr, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    juce::ignoreUnused (sampleRate, samplesPerBlock);
    juce::dsp::ProcessSpec spec;
    spec.maximumBlockSize = static_cast<juce::uint32> (samplesPerBlock);
    spec.numChannels = 2;
    spec.sampleRate = sr;
    sampleRate = sr;
    
    transferFunctionProcessor->prepare (spec);
    auto& inputGain = inputChain.get<0>();
    inputGain.setRampDurationSeconds (0.01);

    auto& lowShelf = inputChain.get<1>();
    *lowShelf.state = juce::dsp::IIR::ArrayCoefficients<float>::makeLowShelf (spec.sampleRate, 400.0f, 1.0f, juce::Decibels::decibelsToGain (0.0f));

    inputChain.prepare (spec);

    overSampler.reset();
    overSampler.initProcessing (static_cast<unsigned long> (samplesPerBlock));

    auto& dcFilter = outputChain.get<0>();
    *dcFilter.state = juce::dsp::IIR::ArrayCoefficients<float>::makeHighPass (sampleRate, 5.0f);

    auto& outputLevel = outputChain.get<4>();
    outputLevel.setRampDurationSeconds (0.01);
    
    outputChain.prepare (spec);
    phaseIncrement = juce::MathConstants<double>::twoPi * 440.0 / sampleRate;
}

void MainProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

bool MainProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}

void MainProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                  juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused (midiMessages);

    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // auto* channelData = buffer.getWritePointer (0);
    // for (int i = 0; i < buffer.getNumSamples(); i++)
    // {
    //     channelData[i] = (float)std::sin (phase) * 0.92f;
    //     phase = std::fmod (phase + phaseIncrement, juce::MathConstants<double>::twoPi);
    // }
    // buffer.copyFrom (1, 0, buffer.getReadPointer (0), buffer.getNumSamples());
    auto& inputGain = inputChain.get<0>();
    inputGain.setGainDecibels (*valueTreeState.getRawParameterValue ("InputGain"));
    
    smoothFilterSettings.setTarget ({valueTreeState.getRawParameterValue ("LowShelfFrequency")->load(),
                                     valueTreeState.getRawParameterValue ("LowShelfGain")->load(),
                                     valueTreeState.getRawParameterValue ("LowShelfQ")->load()});
    if (!juce::approximatelyEqual (sampleRate, 0.0))
    {
        auto settings = smoothFilterSettings.getSettings(); juce::ignoreUnused (settings);
        auto& lowShelf = inputChain.get<1>(); juce::ignoreUnused (lowShelf);
        *lowShelf.state = juce::dsp::IIR::ArrayCoefficients<float>::makeLowShelf 
            (sampleRate, 
            settings.frequency, 
            settings.q, 
            juce::Decibels::decibelsToGain (settings.gain));
    }
    auto& inputCompressor = inputChain.get<2>();
    inputCompressor.setAttack (*valueTreeState.getRawParameterValue ("InputCompressionAttack"));
    inputCompressor.setRelease (*valueTreeState.getRawParameterValue ("InputCompressionRelease"));
    inputCompressor.setRatio (*valueTreeState.getRawParameterValue ("InputCompressionRatio"));
    inputCompressor.setThreshold (*valueTreeState.getRawParameterValue ("InputCompressionThreshold"));
    

    auto inputBlock = juce::dsp::AudioBlock<float> (buffer);
    auto inputContext = juce::dsp::ProcessContextReplacing (inputBlock);
    inputChain.process (inputContext);

    auto upSampledBlock = overSampler.processSamplesUp (inputBlock);
    auto upSampledContext = juce::dsp::ProcessContextReplacing<float> (upSampledBlock);
    transferFunctionProcessor->setMix (*valueTreeState.getRawParameterValue ("Blend"));
    transferFunctionProcessor->process (upSampledContext);
    overSampler.processSamplesDown (inputBlock);

    auto& highShelf = outputChain.get<1>(); juce::ignoreUnused (highShelf);
        *highShelf.state = juce::dsp::IIR::ArrayCoefficients<float>::makeHighShelf 
        (sampleRate, 
        *valueTreeState.getRawParameterValue ("HighShelfFrequency"), 
        *valueTreeState.getRawParameterValue ("HighShelfQ"), 
        juce::Decibels::decibelsToGain (valueTreeState.getRawParameterValue ("HighShelfGain")->load()));
    
    auto& lowPass = outputChain.get<2>();
    *lowPass.state = juce::dsp::IIR::ArrayCoefficients<float>::makeLowPass 
        (sampleRate, *valueTreeState.getRawParameterValue ("LowPassFrequency"));
 
    auto& outputCompressor = outputChain.get<3>();
    outputCompressor.setAttack (*valueTreeState.getRawParameterValue ("OutputCompressionAttack"));
    outputCompressor.setRelease (*valueTreeState.getRawParameterValue ("OutputCompressionRelease"));
    outputCompressor.setRatio (*valueTreeState.getRawParameterValue ("OutputCompressionRatio"));
    outputCompressor.setThreshold (*valueTreeState.getRawParameterValue ("OutputCompressionThreshold"));

    auto& outputLevel = outputChain.get<4>();
    outputLevel.setGainDecibels (*valueTreeState.getRawParameterValue ("OutputLevel"));

    auto outputBlock = juce::dsp::AudioBlock<float> (buffer);
    auto outputContext = juce::dsp::ProcessContextReplacing<float> (outputBlock);

    outputChain.process (outputContext);
}

//==============================================================================
bool MainProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* MainProcessor::createEditor()
{
    return new MainEditor (*this);
}

//==============================================================================
void MainProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = valueTreeState.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void MainProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    juce::ignoreUnused (data, sizeInBytes);
    // std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
 
    // if (xmlState.get() != nullptr)
    //     if (xmlState->hasTagName (valueTreeState.state.getType()))
    //         valueTreeState.replaceState (juce::ValueTree::fromXml (*xmlState));
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new MainProcessor();
}

juce::AudioProcessorValueTreeState::ParameterLayout MainProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    juce::NormalisableRange<float> range;

    range = {-12.0f, 12.0f};
    layout.add (std::make_unique<op::RangedFloatParameter> ("Input Gain", range, 0.0f));
    
    range = {20.0f, 320.0f};
    range.setSkewForCentre (80.0f);
    layout.add (std::make_unique<op::RangedFloatParameter> ("Low Shelf Frequency", range, 80.0f));
    range = {-8.0f, 8.0f};
    layout.add (std::make_unique<op::RangedFloatParameter> ("Low Shelf Gain", range, 0.0f));
    range = {0.25, 4.0f}; range.setSkewForCentre (1.0f);
    layout.add (std::make_unique<op::RangedFloatParameter> ("Low Shelf Q", range, 1.0f));
    
    range = {-30.0f, 0.0f};
    layout.add (std::make_unique<op::RangedFloatParameter> ("Input Compression Threshold", range, 0.0f));
    range = {1.0f, 32.0f}; range.setSkewForCentre (4.0f);
    layout.add (std::make_unique<op::RangedFloatParameter> ("Input Compression Ratio", range, 4.0f));
    range = {1.0f, 256.0f}; range.setSkewForCentre (16.0f);
    layout.add (std::make_unique<op::RangedFloatParameter> ("Input Compression Attack", range, 16.0f));
    range = {20.0f, 1280.0f}; range.setSkewForCentre (640.0f);
    layout.add (std::make_unique<op::RangedFloatParameter> ("Input Compression Release", range, 640.0f));

    layout.add (std::make_unique<op::NormalizedFloatParameter> ("Blend", 1.0f));

    range = {1000.0f, 10000.0f}; range.setSkewForCentre (4000.0f);
    layout.add (std::make_unique<op::RangedFloatParameter> ("High Shelf Frequency", range, 4000.0f));
    range = {-8.0f, 8.0f};
    layout.add (std::make_unique<op::RangedFloatParameter> ("High Shelf Gain", range, 0.0f));
    range = {0.25, 4.0f}; range.setSkewForCentre (1.0f);
    layout.add (std::make_unique<op::RangedFloatParameter> ("High Shelf Q", range, 1.0f));

    range = {5000.0f, 20000.0f}; range.setSkewForCentre (10000.0f);
    layout.add (std::make_unique<op::RangedFloatParameter> ("Low Pass Frequency", range, 18500.0f));

    range = {-30.0f, 0.0f};
    layout.add (std::make_unique<op::RangedFloatParameter> ("Output Compression Threshold", range, 0.0f));
    range = {1.0f, 32.0f}; range.setSkewForCentre (4.0f);
    layout.add (std::make_unique<op::RangedFloatParameter> ("Output Compression Ratio", range, 4.0f));
    range = {1.0f, 256.0f}; range.setSkewForCentre (16.0f);
    layout.add (std::make_unique<op::RangedFloatParameter> ("Output Compression Attack", range, 16.0f));
    range = {20.0f, 1280.0f}; range.setSkewForCentre (640.0f);
    layout.add (std::make_unique<op::RangedFloatParameter> ("Output Compression Release", range, 640.0f));

    range = { -60.0f, 6.0f };
    layout.add (std::make_unique<op::RangedFloatParameter> ("Output Level", range, 0.0f));

    return layout;
}