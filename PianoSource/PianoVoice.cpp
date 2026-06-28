#include "PianoVoice.h"

PianoVoice::PianoVoice() {}

void PianoVoice::prepareToPlay(double sr, int block, int ch) {
    sampleRate = sr;
    ampEnv.setSampleRate(sr);
    juce::dsp::ProcessSpec spec { sr, (juce::uint32)block, (juce::uint32)ch };
    filter.prepare(spec);
    filter.setType(juce::dsp::StateVariableTPTFilterType::lowpass);
    prepared = true;
}

void PianoVoice::updateParams(juce::AudioProcessorValueTreeState& apvts) {
    modIndexPeak  = *apvts.getRawParameterValue("EP_FM_DEPTH");
    modDecay      = *apvts.getRawParameterValue("EP_FM_DECAY");
    tremoloRate   = *apvts.getRawParameterValue("EP_TREM_RATE");
    tremoloDepth  = *apvts.getRawParameterValue("EP_TREM_DEPTH");
    brightness    = *apvts.getRawParameterValue("EP_BRIGHT");

    ampParams = {
        *apvts.getRawParameterValue("EP_ATK"),
        *apvts.getRawParameterValue("EP_DEC"),
        *apvts.getRawParameterValue("EP_SUS"),
        *apvts.getRawParameterValue("EP_REL")
    };
    ampEnv.setParameters(ampParams);

    lfoInc = juce::MathConstants<double>::twoPi * (double)tremoloRate / sampleRate;
}

void PianoVoice::startNote(int midiNote, float vel, juce::SynthesiserSound*, int) {
    velocity = vel;
    double freq = juce::MidiMessage::getMidiNoteInHertz(midiNote);
    carrierInc   = juce::MathConstants<double>::twoPi * freq / sampleRate;
    modulatorInc = carrierInc; // 1:1 ratio — classic Rhodes
    carrierPhase   = 0.0;
    modulatorPhase = 0.0;
    modIndex = modIndexPeak * (double)vel;
    lfoPhase = 0.0;
    ampEnv.noteOn();
}

void PianoVoice::stopNote(float, bool allowTailOff) {
    ampEnv.noteOff();
    if (!allowTailOff) clearCurrentNote();
}

void PianoVoice::renderNextBlock(juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples) {
    if (!prepared || !isVoiceActive()) return;

    // How fast modIndex decays per sample
    float decaySamples = juce::jmax(1.0f, modDecay * (float)sampleRate);
    float modDecayRate = modIndexPeak / decaySamples;

    // Filter cutoff from brightness (0-1 → 500Hz-18kHz)
    float cutoff = 500.0f + brightness * 17500.0f;
    filter.setCutoffFrequency(cutoff);
    filter.setResonance(0.15f);

    for (int i = 0; i < numSamples; ++i) {
        // 2-op FM: output = sin(carrier + modIndex * sin(modulator))
        float mod = (float)std::sin(modulatorPhase) * modIndex;
        float sample = (float)std::sin(carrierPhase + (double)mod);

        carrierPhase   += carrierInc;
        modulatorPhase += modulatorInc;
        if (carrierPhase   > juce::MathConstants<double>::twoPi) carrierPhase   -= juce::MathConstants<double>::twoPi;
        if (modulatorPhase > juce::MathConstants<double>::twoPi) modulatorPhase -= juce::MathConstants<double>::twoPi;

        // Decay modulation index
        modIndex = juce::jmax(0.0f, modIndex - modDecayRate);

        // Tremolo
        float tremolo = 1.0f - tremoloDepth * 0.5f * (1.0f + (float)std::sin(lfoPhase));
        lfoPhase += lfoInc;
        if (lfoPhase > juce::MathConstants<double>::twoPi) lfoPhase -= juce::MathConstants<double>::twoPi;

        // Filter
        sample = filter.processSample(0, sample);

        // Amp env + tremolo
        float env = ampEnv.getNextSample();
        sample *= env * tremolo * velocity * 0.6f;

        for (int ch = 0; ch < outputBuffer.getNumChannels(); ++ch)
            outputBuffer.addSample(ch, startSample + i, sample);

        if (!ampEnv.isActive()) { clearCurrentNote(); break; }
    }
}
