#pragma once
#include <JuceHeader.h>

struct StabSound : public juce::SynthesiserSound {
    bool appliesToNote(int) override { return true; }
    bool appliesToChannel(int) override { return true; }
};

class StabVoice : public juce::SynthesiserVoice {
public:
    StabVoice();
    void prepareToPlay(double sampleRate, int samplesPerBlock, int numChannels);
    void updateParams(juce::AudioProcessorValueTreeState& apvts);

    bool canPlaySound(juce::SynthesiserSound* s) override { return dynamic_cast<StabSound*>(s) != nullptr; }
    void startNote(int midiNote, float velocity, juce::SynthesiserSound*, int) override;
    void stopNote(float, bool allowTailOff) override;
    void pitchWheelMoved(int) override {}
    void controllerMoved(int, int) override {}
    void renderNextBlock(juce::AudioBuffer<float>&, int startSample, int numSamples) override;

private:
    static constexpr int NUM_OSC = 4;
    juce::dsp::Oscillator<float> osc[NUM_OSC];
    juce::dsp::StateVariableTPTFilter<float> filter;
    juce::dsp::Gain<float> gain;

    juce::ADSR ampEnv, filterEnv;
    juce::ADSR::Parameters ampParams, filterParams;

    int   chordType { 0 };
    int   waveType  { 0 };
    float cutoff    { 3000.0f };
    float resonance { 0.5f };
    float filterEnvAmt { 0.6f };
    float driveAmt  { 0.0f };

    bool prepared { false };
    double sampleRate { 44100.0 };

    // chord intervals in semitones; 4th entry -1 = unused
    static constexpr int CHORDS[5][4] = {
        { 0,  4,  7, -1 }, // Major
        { 0,  3,  7, -1 }, // Minor
        { 0,  4,  7, 10 }, // Dom 7
        { 0,  3,  7, 10 }, // Min 7
        { 0,  5,  7, -1 }, // Sus4
    };

    void setWave(juce::dsp::Oscillator<float>& o, int wave);
    static float softClip(float x) { return x / (1.0f + std::abs(x)); }
};
