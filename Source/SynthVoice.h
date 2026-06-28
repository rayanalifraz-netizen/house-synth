#pragma once
#include <JuceHeader.h>

struct SynthSound : public juce::SynthesiserSound {
    bool appliesToNote(int) override { return true; }
    bool appliesToChannel(int) override { return true; }
};

class SynthVoice : public juce::SynthesiserVoice {
public:
    SynthVoice();

    void prepareToPlay(double sampleRate, int samplesPerBlock, int outputChannels);
    void updateParams(juce::AudioProcessorValueTreeState& apvts);

    bool canPlaySound(juce::SynthesiserSound* s) override { return dynamic_cast<SynthSound*>(s) != nullptr; }
    void startNote(int midiNote, float velocity, juce::SynthesiserSound*, int pitchWheel) override;
    void stopNote(float velocity, bool allowTailOff) override;
    void pitchWheelMoved(int) override {}
    void controllerMoved(int, int) override {}
    void renderNextBlock(juce::AudioBuffer<float>& buffer, int startSample, int numSamples) override;

private:
    // Oscillators
    juce::dsp::Oscillator<float> osc1, osc2, subOsc;

    // Filter
    juce::dsp::StateVariableTPTFilter<float> filter;

    // Envelopes
    juce::ADSR ampEnv, filterEnv;
    juce::ADSR::Parameters ampParams, filterParams;

    // Effects chain per voice
    juce::dsp::Gain<float> gain;

    // Detune / unison state
    float detuneAmount { 0.0f };
    float filterCutoff  { 2000.0f };
    float filterRes     { 0.5f };
    float filterEnvAmt  { 0.3f };
    float driveAmount   { 0.0f };
    int   waveType      { 0 };
    int   mode          { 0 }; // 0=chord, 1=bass, 2=lead

    double currentSampleRate { 44100.0 };
    float  currentMidiNote   { 60.0f };
    float  currentVelocity   { 1.0f };

    bool prepared { false };

    juce::dsp::ProcessSpec spec;

    void setOscWave(juce::dsp::Oscillator<float>& osc, int wave);
    void applyDrive(float* data, int numSamples, float amount);
};
