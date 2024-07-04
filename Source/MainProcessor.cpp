#include "MainProcessor.h"
#include "MainEditor.h"
#include "Identifiers.h"
#include "DefaultTreeGenerator.h"
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
      valueTreeState (*this, &undoManager, id::ORIOTO, {}), 
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
void MainProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    juce::ignoreUnused (sampleRate, samplesPerBlock);
    juce::dsp::ProcessSpec spec;
    spec.maximumBlockSize = static_cast<juce::uint32> (samplesPerBlock);
    spec.numChannels = 2;
    spec.sampleRate = sampleRate;
    transferFunctionProcessor->prepare (spec);
    
    auto& lowShelf = inputChain.get<0>();
    lowShelf.state = juce::dsp::IIR::Coefficients<float>::makeLowShelf (spec.sampleRate, 200.0f, 10.0f, juce::Decibels::decibelsToGain (12.0f));

    inputChain.prepare (spec);

    overSampler.reset();
    overSampler.initProcessing (static_cast<unsigned long> (samplesPerBlock));

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

    auto inputBlock = juce::dsp::AudioBlock<float> (buffer);
    auto inputContext = juce::dsp::ProcessContextReplacing (inputBlock);
    inputChain.process (inputContext);

    auto upSampledBlock = overSampler.processSamplesUp (inputBlock);
    auto upSampledContext = juce::dsp::ProcessContextReplacing<float> (upSampledBlock);
    transferFunctionProcessor->process (upSampledContext);
    overSampler.processSamplesDown (inputBlock);
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
