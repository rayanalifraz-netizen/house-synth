#pragma once
#include <JuceHeader.h>

struct SupersawSound : public juce::SynthesiserSound {
    bool appliesToNote(int) override { return true; }
    bool appliesToChannel(int) override { return true; }
};

class SupersawVoice : public juce::SynthesiserVoice {
public:
    SupersawVoice();
    void prepareToPlay(double sampleRate, int samplesPerBlock, int numChannels);
    void updateParams(juce::AudioProcessorValueTreeState& apvts);

    bool canPlaySound(juce::SynthesiserSound* s) override { return dynamic_cast<SupersawSound*>(s) != nullptr; }
    void startNote(int midiNote, float velocity, juce::SynthesiserSound*, int) override;
    void stopNote(float, bool allowTailOff) override;
    void pitchWheelMoved(int) override {}
    void controllerMoved(int, int) override {}
    void renderNextBlock(juce::AudioBuffer<float>&, int startSample, int numSamples) override;

private:
    static constexpr int NUM_SAW = 7;
    juce::dsp::Oscillator<float> saws[NUM_SAW];
    // panning: alternating L/R for stereo width
    static constexpr float PAN[NUM_SAW] = { 0.0f, -0.8f, 0.8f, -0.5f, 0.5f, -1.0f, 1.0f };
    // detune offsets in cents relative to centre
    static constexpr float DETUNE_OFFSETS[NUM_SAW] = { 0.f, -17.f, 17.f, -34.f, 34.f, -51.f, 51.f };

    juce::dsp::StateVariableTPTFilter<float> filter;
    juce::ADSR ampEnv;
    juce::ADSR::Parameters ampParams;

    float spread    { 0.3f };
    float cutoff    { 8000.0f };
    float resonance { 0.2f };

    bool prepared { false };
    int  midiNote { 60 };

    void updateFrequencies();
};
