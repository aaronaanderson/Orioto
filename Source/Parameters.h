#pragma once

namespace op
{
class ChoiceParameter : public juce::AudioParameterChoice
{
public:
    ChoiceParameter(juce::String parameterName, 
                    juce::StringArray iChoices, 
                    juce::String unit,
                    int defaultChoice = 0)
      : juce::AudioParameterChoice ({parameterName.removeCharacters(" "), 1}, 
                                    parameterName, 
                                    iChoices, 
                                    defaultChoice,
                                    juce::AudioParameterChoiceAttributes().withLabel (unit)),
        options (iChoices)
    {
        valueChanged (defaultChoice);
    }

    void setIndex (int newIndex)
    {
        auto normalized = newIndex / static_cast<float> (options.size() - 1);
        setValueNotifyingHost (normalized);
    }
private: 
    juce::StringArray options;
};
class NormalizedFloatParameter : public juce::AudioParameterFloat
{
public:
    NormalizedFloatParameter (juce::String parameterName, 
                              float defaultValue = 0.0f,
                              juce::String unit = "")
    : juce::AudioParameterFloat ({parameterName.removeCharacters(" "), 1}, 
                                 parameterName,
                                 {0.0f, 1.0f},  
                                 defaultValue,
                                 juce::AudioParameterFloatAttributes().withLabel (unit)
                                                                       .withStringFromValueFunction ([&](float v, int n){ juce::ignoreUnused (n); return juce::String (v, 3); }))
    {
        valueChanged (defaultValue);
    }
};

class RangedFloatParameter : public juce::AudioParameterFloat
{
public:
    RangedFloatParameter (juce::String parameterName, 
                          juce::NormalisableRange<float> pRange,
                          float defaultValue = 0.0f,
                          juce::String unit = "")
    : juce::AudioParameterFloat ({parameterName.removeCharacters(" "), 1}, 
                                 parameterName,
                                 pRange,  
                                 defaultValue,
                                 juce::AudioParameterFloatAttributes().withLabel (unit)
                                                                      .withStringFromValueFunction ([&](float v, int n){ juce::ignoreUnused (n); return juce::String (v, 3); }))
    {
        valueChanged (defaultValue);
    }
};

}
