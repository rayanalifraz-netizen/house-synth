#include "PluginProcessor.h"
#include "PluginEditor.h"

juce::AudioProcessorValueTreeState::ParameterLayout HouseSynthProcessor::createParams() {
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    // Mode: 0=Chord, 1=Bass, 2=Lead
    params.push_back(std::make_unique<juce::AudioParameterChoice>("MODE", "Mode",
        juce::StringArray { "Chord", "Bass", "Lead" }, 0));

    // Oscillator
    params.push_back(std::make_unique<juce::AudioParameterChoice>("WAVE", "Wave",
        juce::StringArray { "Saw", "Square", "Sine" }, 0));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("DETUNE", "Detune",
        juce::NormalisableRange<float>(0.0f, 50.0f, 0.1f), 8.0f));

    // Filter
    params.push_back(std::make_unique<juce::AudioParameterFloat>("CUTOFF", "Cutoff",
        juce::NormalisableRange<float>(80.0f, 18000.0f, 1.0f, 0.3f), 4000.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("RESONANCE", "Resonance",
        juce::NormalisableRange<float>(0.1f, 0.95f, 0.01f), 0.4f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("FILTER_ENV_AMT", "Filter Env",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.3f));

    // Amp Envelope
    params.push_back(std::make_unique<juce::AudioParameterFloat>("AMP_ATK", "Amp Attack",
        juce::NormalisableRange<float>(0.001f, 2.0f, 0.001f, 0.4f), 0.01f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("AMP_DEC", "Amp Decay",
        juce::NormalisableRange<float>(0.01f, 2.0f, 0.001f, 0.4f), 0.3f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("AMP_SUS", "Amp Sustain",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.7f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("AMP_REL", "Amp Release",
        juce::NormalisableRange<float>(0.01f, 4.0f, 0.001f, 0.4f), 0.5f));

    // Filter Envelope
    params.push_back(std::make_unique<juce::AudioParameterFloat>("FLT_ATK", "Filter Attack",
        juce::NormalisableRange<float>(0.001f, 2.0f, 0.001f, 0.4f), 0.01f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("FLT_DEC", "Filter Decay",
        juce::NormalisableRange<float>(0.01f, 2.0f, 0.001f, 0.4f), 0.4f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("FLT_SUS", "Filter Sustain",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.2f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("FLT_REL", "Filter Release",
        juce::NormalisableRange<float>(0.01f, 4.0f, 0.001f, 0.4f), 0.5f));

    // Drive
    params.push_back(std::make_unique<juce::AudioParameterFloat>("DRIVE", "Drive",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.0f));

    // Global FX
    params.push_back(std::make_unique<juce::AudioParameterFloat>("CHORUS", "Chorus",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("REVERB", "Reverb",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.15f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("MASTER", "Master",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.8f));

    return { params.begin(), params.end() };
}

HouseSynthProcessor::HouseSynthProcessor()
    : AudioProcessor(BusesProperties().withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "PARAMS", createParams())
{
    for (int i = 0; i < 8; ++i) {
        synth.addVoice(new SynthVoice());
        synth.addSound(new SynthSound());
    }
}

void HouseSynthProcessor::prepareToPlay(double sampleRate, int samplesPerBlock) {
    synth.setCurrentPlaybackSampleRate(sampleRate);

    juce::dsp::ProcessSpec spec { sampleRate, (juce::uint32)samplesPerBlock, 2 };

    for (int i = 0; i < synth.getNumVoices(); ++i)
        if (auto* v = dynamic_cast<SynthVoice*>(synth.getVoice(i)))
            v->prepareToPlay(sampleRate, samplesPerBlock, 2);

    chorus.prepare(spec);
    chorus.setRate(0.3f);
    chorus.setDepth(0.02f);
    chorus.setCentreDelay(7.0f);
    chorus.setFeedback(0.1f);
    chorus.setMix(0.0f);

    reverb.setSampleRate(sampleRate);

    masterGain.prepare(spec);
    masterGain.setGainLinear(0.8f);
}

void HouseSynthProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi) {
    juce::ScopedNoDenormals noDenormals;
    buffer.clear();

    for (int i = 0; i < synth.getNumVoices(); ++i)
        if (auto* v = dynamic_cast<SynthVoice*>(synth.getVoice(i)))
            v->updateParams(apvts);

    synth.renderNextBlock(buffer, midi, 0, buffer.getNumSamples());

    // Chorus
    float chorusMix = *apvts.getRawParameterValue("CHORUS");
    chorus.setMix(chorusMix);
    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> ctx(block);
    chorus.process(ctx);

    // Reverb
    float reverbMix = *apvts.getRawParameterValue("REVERB");
    juce::Reverb::Parameters rp;
    rp.roomSize   = 0.5f + reverbMix * 0.4f;
    rp.damping    = 0.5f;
    rp.wetLevel   = reverbMix * 0.5f;
    rp.dryLevel   = 1.0f - reverbMix * 0.3f;
    rp.width      = 1.0f;
    reverb.setParameters(rp);

    if (buffer.getNumChannels() >= 2)
        reverb.processStereo(buffer.getWritePointer(0), buffer.getWritePointer(1), buffer.getNumSamples());
    else
        reverb.processMono(buffer.getWritePointer(0), buffer.getNumSamples());

    // Master gain
    float master = *apvts.getRawParameterValue("MASTER");
    masterGain.setGainLinear(master);
    masterGain.process(ctx);
}

void HouseSynthProcessor::setCurrentProgram(int index) {
    currentProgram = index;
    loadPreset(index);
}

const juce::String HouseSynthProcessor::getProgramName(int index) {
    switch (index) {
        case 0: return "House Chord";
        case 1: return "House Bass";
        case 2: return "House Lead";
        default: return "Default";
    }
}

void HouseSynthProcessor::loadPreset(int index) {
    auto set = [&](const char* id, float v) {
        if (auto* p = apvts.getParameter(id)) p->setValueNotifyingHost(p->convertTo0to1(v));
    };

    switch (index) {
        case 0: // House Chord — wide detuned pad, slow attack, lots of width
            set("MODE", 0); set("WAVE", 0); set("DETUNE", 22.0f);
            set("CUTOFF", 2800.0f); set("RESONANCE", 0.3f); set("FILTER_ENV_AMT", 0.15f);
            set("AMP_ATK", 0.08f); set("AMP_DEC", 0.5f); set("AMP_SUS", 0.85f); set("AMP_REL", 1.2f);
            set("FLT_ATK", 0.15f); set("FLT_DEC", 0.6f); set("FLT_SUS", 0.4f); set("FLT_REL", 0.8f);
            set("DRIVE", 0.0f); set("CHORUS", 0.8f); set("REVERB", 0.5f); set("MASTER", 0.75f);
            break;
        case 1: // House Bass — deep sine sub, instant attack, hard filter snap, zero width
            set("MODE", 1); set("WAVE", 2); set("DETUNE", 0.0f);
            set("CUTOFF", 180.0f); set("RESONANCE", 0.65f); set("FILTER_ENV_AMT", 0.9f);
            set("AMP_ATK", 0.001f); set("AMP_DEC", 0.18f); set("AMP_SUS", 0.3f); set("AMP_REL", 0.15f);
            set("FLT_ATK", 0.001f); set("FLT_DEC", 0.18f); set("FLT_SUS", 0.0f); set("FLT_REL", 0.1f);
            set("DRIVE", 0.3f); set("CHORUS", 0.0f); set("REVERB", 0.0f); set("MASTER", 0.9f);
            break;
        case 2: // House Lead — single bright saw, wide open filter, punchy
            set("MODE", 2); set("WAVE", 0); set("DETUNE", 5.0f);
            set("CUTOFF", 9000.0f); set("RESONANCE", 0.6f); set("FILTER_ENV_AMT", 0.55f);
            set("AMP_ATK", 0.003f); set("AMP_DEC", 0.15f); set("AMP_SUS", 0.5f); set("AMP_REL", 0.3f);
            set("FLT_ATK", 0.003f); set("FLT_DEC", 0.25f); set("FLT_SUS", 0.1f); set("FLT_REL", 0.2f);
            set("DRIVE", 0.15f); set("CHORUS", 0.15f); set("REVERB", 0.12f); set("MASTER", 0.8f);
            break;
    }
}

void HouseSynthProcessor::getStateInformation(juce::MemoryBlock& dest) {
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, dest);
}

void HouseSynthProcessor::setStateInformation(const void* data, int size) {
    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, size));
    if (xml && xml->hasTagName(apvts.state.getType()))
        apvts.replaceState(juce::ValueTree::fromXml(*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
    return new HouseSynthProcessor();
}
