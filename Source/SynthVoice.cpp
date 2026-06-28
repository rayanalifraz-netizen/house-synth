#include "SynthVoice.h"

SynthVoice::SynthVoice() {}

static float softClip(float x) {
    return x / (1.0f + std::abs(x));
}

void SynthVoice::setOscWave(juce::dsp::Oscillator<float>& osc, int wave) {
    switch (wave) {
        case 0: osc.initialise([](float x) { return x / juce::MathConstants<float>::pi; }); break; // saw
        case 1: osc.initialise([](float x) { return x < 0.0f ? -1.0f : 1.0f; });            break; // square
        case 2: osc.initialise([](float x) { return std::sin(x); });                          break; // sine
        default: osc.initialise([](float x) { return x / juce::MathConstants<float>::pi; }); break;
    }
}

void SynthVoice::prepareToPlay(double sampleRate, int samplesPerBlock, int outputChannels) {
    currentSampleRate = sampleRate;
    spec = { sampleRate, (juce::uint32)samplesPerBlock, (juce::uint32)outputChannels };

    setOscWave(osc1, 0);
    setOscWave(osc2, 0);
    subOsc.initialise([](float x) { return std::sin(x); });

    osc1.prepare(spec);
    osc2.prepare(spec);
    subOsc.prepare(spec);

    filter.prepare(spec);
    filter.setType(juce::dsp::StateVariableTPTFilterType::lowpass);

    gain.prepare(spec);
    gain.setGainLinear(0.4f);

    ampEnv.setSampleRate(sampleRate);
    filterEnv.setSampleRate(sampleRate);

    prepared = true;
}

void SynthVoice::updateParams(juce::AudioProcessorValueTreeState& apvts) {
    mode        = (int)*apvts.getRawParameterValue("MODE");
    waveType    = (int)*apvts.getRawParameterValue("WAVE");
    detuneAmount= *apvts.getRawParameterValue("DETUNE");
    filterCutoff= *apvts.getRawParameterValue("CUTOFF");
    filterRes   = *apvts.getRawParameterValue("RESONANCE");
    filterEnvAmt= *apvts.getRawParameterValue("FILTER_ENV_AMT");
    driveAmount = *apvts.getRawParameterValue("DRIVE");

    ampParams.attack  = *apvts.getRawParameterValue("AMP_ATK");
    ampParams.decay   = *apvts.getRawParameterValue("AMP_DEC");
    ampParams.sustain = *apvts.getRawParameterValue("AMP_SUS");
    ampParams.release = *apvts.getRawParameterValue("AMP_REL");

    filterParams.attack  = *apvts.getRawParameterValue("FLT_ATK");
    filterParams.decay   = *apvts.getRawParameterValue("FLT_DEC");
    filterParams.sustain = *apvts.getRawParameterValue("FLT_SUS");
    filterParams.release = *apvts.getRawParameterValue("FLT_REL");

    ampEnv.setParameters(ampParams);
    filterEnv.setParameters(filterParams);

    setOscWave(osc1, waveType);
    setOscWave(osc2, waveType);
}

void SynthVoice::startNote(int midiNote, float velocity, juce::SynthesiserSound*, int) {
    currentMidiNote = (float)midiNote;
    currentVelocity = velocity;

    float freq = juce::MidiMessage::getMidiNoteInHertz(midiNote);
    float detuneHz = freq * (std::pow(2.0f, detuneAmount / 1200.0f) - 1.0f);

    osc1.setFrequency(freq + detuneHz);
    osc2.setFrequency(freq - detuneHz);
    subOsc.setFrequency(freq * 0.5f); // one octave down

    ampEnv.noteOn();
    filterEnv.noteOn();
}

void SynthVoice::stopNote(float, bool allowTailOff) {
    ampEnv.noteOff();
    filterEnv.noteOff();
    if (!allowTailOff)
        clearCurrentNote();
}

void SynthVoice::renderNextBlock(juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples) {
    if (!prepared || !isVoiceActive()) return;

    juce::AudioBuffer<float> voiceBuffer(1, numSamples);
    voiceBuffer.clear();

    auto* data = voiceBuffer.getWritePointer(0);

    // Mix oscillators based on mode
    for (int i = 0; i < numSamples; ++i) {
        float o1  = osc1.processSample(0.0f);
        float o2  = osc2.processSample(0.0f);
        float sub = subOsc.processSample(0.0f);

        float sample = 0.0f;
        switch (mode) {
            case 0: // Chord — lush detuned saws, light sub body
                sample = (o1 * 0.45f) + (o2 * 0.45f) + (sub * 0.1f);
                break;
            case 1: // Bass — pure sine sub + just a touch of saw bite
                sample = (sub * 0.85f) + (o1 * 0.15f);
                break;
            case 2: // Lead — single bright saw, no sub at all
                sample = o1 * 0.9f;
                break;
        }

        data[i] = sample;
    }

    // Filter envelope modulation — bass gets 3x more snap
    float envCutoffMod = filterEnvAmt * (mode == 1 ? 24000.0f : 8000.0f);
    for (int i = 0; i < numSamples; ++i) {
        float fEnv = filterEnv.getNextSample();
        float cutoff = juce::jlimit(80.0f, 18000.0f, filterCutoff + fEnv * envCutoffMod);
        filter.setCutoffFrequency(cutoff);
        filter.setResonance(filterRes);
        data[i] = filter.processSample(0, data[i]);
    }

    // Drive
    if (driveAmount > 0.01f)
        applyDrive(data, numSamples, driveAmount);

    // Amp envelope
    for (int i = 0; i < numSamples; ++i) {
        float env = ampEnv.getNextSample();
        data[i] *= env * currentVelocity;
        if (!ampEnv.isActive())
            clearCurrentNote();
    }

    // Write to output (mono → stereo)
    juce::dsp::AudioBlock<float> block(voiceBuffer);
    juce::dsp::ProcessContextReplacing<float> ctx(block);
    gain.process(ctx);

    for (int ch = 0; ch < outputBuffer.getNumChannels(); ++ch)
        outputBuffer.addFrom(ch, startSample, voiceBuffer, 0, 0, numSamples);
}

void SynthVoice::applyDrive(float* data, int numSamples, float amount) {
    float preGain = 1.0f + amount * 4.0f;
    for (int i = 0; i < numSamples; ++i)
        data[i] = softClip(data[i] * preGain);
}
