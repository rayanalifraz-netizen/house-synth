#pragma once
#include <JuceHeader.h>
#include "SynthVoice.h"

class HouseSynthProcessor : public juce::AudioProcessor {
public:
    HouseSynthProcessor();
    ~HouseSynthProcessor() override = default;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "House Synth"; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 1.0; }

    int getNumPrograms() override { return 3; }
    int getCurrentProgram() override { return currentProgram; }
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock&) override;
    void setStateInformation(const void*, int) override;

    juce::AudioProcessorValueTreeState apvts;
    static juce::AudioProcessorValueTreeState::ParameterLayout createParams();

    // Global FX
    juce::dsp::Chorus<float>   chorus;
    juce::Reverb               reverb;
    juce::dsp::Gain<float>     masterGain;

private:
    juce::Synthesiser synth;
    int currentProgram { 0 };

    void loadPreset(int index);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(HouseSynthProcessor)
};
