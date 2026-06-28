#include "SupersawVoice.h"

constexpr float SupersawVoice::PAN[NUM_SAW];
constexpr float SupersawVoice::DETUNE_OFFSETS[NUM_SAW];

SupersawVoice::SupersawVoice() {}

void SupersawVoice::prepareToPlay(double sr, int block, int ch) {
    juce::dsp::ProcessSpec spec { sr, (juce::uint32)block, (juce::uint32)ch };
    for (auto& s : saws) {
        s.initialise([](float x) { return x / juce::MathConstants<float>::pi; });
        s.prepare(spec);
    }
    filter.prepare(spec);
    filter.setType(juce::dsp::StateVariableTPTFilterType::lowpass);
    ampEnv.setSampleRate(sr);
    prepared = true;
}

void SupersawVoice::updateParams(juce::AudioProcessorValueTreeState& apvts) {
    spread    = *apvts.getRawParameterValue("SS_SPREAD");
    cutoff    = *apvts.getRawParameterValue("SS_CUTOFF");
    resonance = *apvts.getRawParameterValue("SS_RES");

    ampParams = {
        *apvts.getRawParameterValue("SS_ATK"),
        0.3f,
        *apvts.getRawParameterValue("SS_SUS"),
        *apvts.getRawParameterValue("SS_REL")
    };
    ampEnv.setParameters(ampParams);
    updateFrequencies();
}

void SupersawVoice::updateFrequencies() {
    float base = juce::MidiMessage::getMidiNoteInHertz(midiNote);
    for (int i = 0; i < NUM_SAW; ++i) {
        float cents = DETUNE_OFFSETS[i] * spread;
        float freq  = base * std::pow(2.0f, cents / 1200.0f);
        saws[i].setFrequency(juce::jlimit(20.0f, 20000.0f, freq));
    }
}

void SupersawVoice::startNote(int note, float, juce::SynthesiserSound*, int) {
    midiNote = note;
    updateFrequencies();
    ampEnv.noteOn();
}

void SupersawVoice::stopNote(float, bool allowTailOff) {
    ampEnv.noteOff();
    if (!allowTailOff) clearCurrentNote();
}

void SupersawVoice::renderNextBlock(juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples) {
    if (!prepared || !isVoiceActive()) return;

    // Render into stereo buffer
    juce::AudioBuffer<float> buf(2, numSamples);
    buf.clear();

    filter.setCutoffFrequency(cutoff);
    filter.setResonance(resonance);

    for (int i = 0; i < numSamples; ++i) {
        float left = 0.f, right = 0.f;
        for (int j = 0; j < NUM_SAW; ++j) {
            float s = saws[j].processSample(0.0f) / (float)NUM_SAW;
            float p = PAN[j]; // -1 = full left, +1 = full right
            left  += s * (1.0f - juce::jmax(0.0f,  p));
            right += s * (1.0f - juce::jmax(0.0f, -p));
        }
        // Filter mono sum then redistribute
        float mono = (left + right) * 0.5f;
        mono = filter.processSample(0, mono);
        float env = ampEnv.getNextSample();
        buf.getWritePointer(0)[i] = mono * env;
        buf.getWritePointer(1)[i] = mono * env;
        if (!ampEnv.isActive()) { clearCurrentNote(); break; }
    }

    // Stereo spread: rebuild L/R from individual saws
    for (int ch = 0; ch < outputBuffer.getNumChannels(); ++ch)
        outputBuffer.addFrom(ch, startSample, buf, juce::jmin(ch, 1), 0, numSamples, 0.5f);
}
