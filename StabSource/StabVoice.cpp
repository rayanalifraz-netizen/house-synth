#include "StabVoice.h"

constexpr int StabVoice::CHORDS[5][4];

StabVoice::StabVoice() {}

void StabVoice::setWave(juce::dsp::Oscillator<float>& o, int wave) {
    switch (wave) {
        case 0: o.initialise([](float x) { return x / juce::MathConstants<float>::pi; }); break; // saw
        case 1: o.initialise([](float x) { return x < 0.f ? -1.f : 1.f; });              break; // square
        default: o.initialise([](float x) { return std::sin(x); });                        break; // sine
    }
}

void StabVoice::prepareToPlay(double sr, int block, int ch) {
    sampleRate = sr;
    juce::dsp::ProcessSpec spec { sr, (juce::uint32)block, (juce::uint32)ch };
    for (auto& o : osc) { setWave(o, 0); o.prepare(spec); }
    filter.prepare(spec);
    filter.setType(juce::dsp::StateVariableTPTFilterType::lowpass);
    gain.prepare(spec);
    gain.setGainLinear(0.3f);
    ampEnv.setSampleRate(sr);
    filterEnv.setSampleRate(sr);
    prepared = true;
}

void StabVoice::updateParams(juce::AudioProcessorValueTreeState& apvts) {
    chordType    = (int)*apvts.getRawParameterValue("STAB_CHORD");
    waveType     = (int)*apvts.getRawParameterValue("STAB_WAVE");
    cutoff       = *apvts.getRawParameterValue("STAB_CUTOFF");
    resonance    = *apvts.getRawParameterValue("STAB_RES");
    filterEnvAmt = *apvts.getRawParameterValue("STAB_FENV");
    driveAmt     = *apvts.getRawParameterValue("STAB_DRIVE");

    ampParams  = { 0.001f,
                   *apvts.getRawParameterValue("STAB_DECAY"),
                   0.0f,
                   *apvts.getRawParameterValue("STAB_REL") };
    filterParams = { 0.001f,
                     *apvts.getRawParameterValue("STAB_DECAY") * 0.7f,
                     0.0f,
                     *apvts.getRawParameterValue("STAB_REL") * 0.5f };

    ampEnv.setParameters(ampParams);
    filterEnv.setParameters(filterParams);
    for (auto& o : osc) setWave(o, waveType);
}

void StabVoice::startNote(int midiNote, float velocity, juce::SynthesiserSound*, int) {
    const int* intervals = CHORDS[juce::jlimit(0, 4, chordType)];
    int oscIdx = 0;
    for (int i = 0; i < NUM_OSC; ++i) {
        if (intervals[i] < 0) {
            osc[i].setFrequency(0.0f);
        } else {
            float freq = juce::MidiMessage::getMidiNoteInHertz(midiNote + intervals[i]);
            osc[i].setFrequency(juce::jlimit(20.0f, 20000.0f, freq));
            ++oscIdx;
        }
    }
    (void)oscIdx;
    (void)velocity;
    ampEnv.noteOn();
    filterEnv.noteOn();
}

void StabVoice::stopNote(float, bool allowTailOff) {
    ampEnv.noteOff();
    filterEnv.noteOff();
    if (!allowTailOff) clearCurrentNote();
}

void StabVoice::renderNextBlock(juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples) {
    if (!prepared || !isVoiceActive()) return;

    juce::AudioBuffer<float> buf(1, numSamples);
    buf.clear();
    auto* data = buf.getWritePointer(0);

    const int* intervals = CHORDS[juce::jlimit(0, 4, chordType)];
    int activeCount = 0;
    for (int i = 0; i < NUM_OSC; ++i)
        if (intervals[i] >= 0) ++activeCount;
    float scale = 1.0f / (float)juce::jmax(1, activeCount);

    for (int i = 0; i < numSamples; ++i) {
        float s = 0.0f;
        for (int j = 0; j < NUM_OSC; ++j)
            if (intervals[j] >= 0) s += osc[j].processSample(0.0f);
        data[i] = s * scale;
    }

    // Filter with envelope
    float envMod = filterEnvAmt * 10000.0f;
    for (int i = 0; i < numSamples; ++i) {
        float env = filterEnv.getNextSample();
        float c = juce::jlimit(80.0f, 18000.0f, cutoff + env * envMod);
        filter.setCutoffFrequency(c);
        filter.setResonance(resonance);
        data[i] = filter.processSample(0, data[i]);
    }

    // Drive
    if (driveAmt > 0.01f) {
        float pre = 1.0f + driveAmt * 5.0f;
        for (int i = 0; i < numSamples; ++i)
            data[i] = softClip(data[i] * pre);
    }

    // Amp envelope
    for (int i = 0; i < numSamples; ++i) {
        data[i] *= ampEnv.getNextSample();
        if (!ampEnv.isActive()) { clearCurrentNote(); break; }
    }

    juce::dsp::AudioBlock<float> block(buf);
    juce::dsp::ProcessContextReplacing<float> ctx(block);
    gain.process(ctx);

    for (int ch = 0; ch < outputBuffer.getNumChannels(); ++ch)
        outputBuffer.addFrom(ch, startSample, buf, 0, 0, numSamples);
}
