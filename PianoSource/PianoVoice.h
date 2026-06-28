#pragma once
#include <JuceHeader.h>

struct PianoSound : public juce::SynthesiserSound {
    bool appliesToNote(int) override { return true; }
    bool appliesToChannel(int) override { return true; }
};

class PianoVoice : public juce::SynthesiserVoice {
public:
    PianoVoice();
    void prepareToPlay(double sampleRate, int samplesPerBlock, int numChannels);
    void updateParams(juce::AudioProcessorValueTreeState& apvts);

    bool canPlaySound(juce::SynthesiserSound* s) override { return dynamic_cast<PianoSound*>(s) != nullptr; }
    void startNote(int midiNote, float velocity, juce::SynthesiserSound*, int) override;
    void stopNote(float, bool allowTailOff) override;
    void pitchWheelMoved(int) override {}
    void controllerMoved(int, int) override {}
    void renderNextBlock(juce::AudioBuffer<float>&, int startSample, int numSamples) override;

private:
    // 2-op FM: carrier + modulator (both sine)
    double carrierPhase  { 0.0 };
    double modulatorPhase { 0.0 };
    double carrierInc    { 0.0 };
    double modulatorInc  { 0.0 };

    // Modulation index envelope (bright attack → warm sustain)
    float  modIndex      { 0.0f };
    float  modIndexPeak  { 3.0f };
    float  modDecay      { 0.3f }; // seconds

    // Tremolo LFO
    double lfoPhase      { 0.0 };
    double lfoInc        { 0.0 };
    float  tremoloDepth  { 0.0f };
    float  tremoloRate   { 5.0f };

    juce::ADSR ampEnv;
    juce::ADSR::Parameters ampParams;

    float brightness { 0.5f };
    float velocity   { 1.0f };

    double sampleRate { 44100.0 };
    bool   prepared   { false };

    juce::dsp::StateVariableTPTFilter<float> filter;
};
