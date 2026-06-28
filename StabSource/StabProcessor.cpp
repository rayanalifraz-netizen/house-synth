#include "StabProcessor.h"
#include "StabEditor.h"

juce::AudioProcessorValueTreeState::ParameterLayout StabProcessor::createParams() {
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> p;

    p.push_back(std::make_unique<juce::AudioParameterChoice>("STAB_CHORD", "Chord",
        juce::StringArray { "Major", "Minor", "Dom 7", "Min 7", "Sus4" }, 0));
    p.push_back(std::make_unique<juce::AudioParameterChoice>("STAB_WAVE", "Wave",
        juce::StringArray { "Saw", "Square", "Sine" }, 0));
    p.push_back(std::make_unique<juce::AudioParameterFloat>("STAB_DECAY",  "Decay",
        juce::NormalisableRange<float>(0.05f, 1.5f, 0.001f, 0.4f), 0.2f));
    p.push_back(std::make_unique<juce::AudioParameterFloat>("STAB_REL",    "Release",
        juce::NormalisableRange<float>(0.02f, 1.0f, 0.001f, 0.4f), 0.1f));
    p.push_back(std::make_unique<juce::AudioParameterFloat>("STAB_CUTOFF", "Cutoff",
        juce::NormalisableRange<float>(80.0f, 18000.0f, 1.0f, 0.3f), 4000.0f));
    p.push_back(std::make_unique<juce::AudioParameterFloat>("STAB_RES",    "Resonance",
        juce::NormalisableRange<float>(0.1f, 0.95f, 0.01f), 0.5f));
    p.push_back(std::make_unique<juce::AudioParameterFloat>("STAB_FENV",   "Filter Env",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.6f));
    p.push_back(std::make_unique<juce::AudioParameterFloat>("STAB_DRIVE",  "Drive",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.1f));
    p.push_back(std::make_unique<juce::AudioParameterFloat>("STAB_REVERB", "Reverb",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.1f));
    p.push_back(std::make_unique<juce::AudioParameterFloat>("STAB_MASTER", "Master",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.8f));

    return { p.begin(), p.end() };
}

StabProcessor::StabProcessor()
    : AudioProcessor(BusesProperties().withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "STAB_PARAMS", createParams())
{
    for (int i = 0; i < 8; ++i) {
        synth.addVoice(new StabVoice());
        synth.addSound(new StabSound());
    }
}

void StabProcessor::prepareToPlay(double sr, int block) {
    synth.setCurrentPlaybackSampleRate(sr);
    for (int i = 0; i < synth.getNumVoices(); ++i)
        if (auto* v = dynamic_cast<StabVoice*>(synth.getVoice(i)))
            v->prepareToPlay(sr, block, 2);

    reverb.setSampleRate(sr);
    juce::dsp::ProcessSpec spec { sr, (juce::uint32)block, 2 };
    masterGain.prepare(spec);
}

void StabProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi) {
    juce::ScopedNoDenormals noDenormals;
    buffer.clear();

    for (int i = 0; i < synth.getNumVoices(); ++i)
        if (auto* v = dynamic_cast<StabVoice*>(synth.getVoice(i)))
            v->updateParams(apvts);

    synth.renderNextBlock(buffer, midi, 0, buffer.getNumSamples());

    float rv = *apvts.getRawParameterValue("STAB_REVERB");
    juce::Reverb::Parameters rp;
    rp.roomSize = 0.4f + rv * 0.3f;
    rp.wetLevel = rv * 0.4f;
    rp.dryLevel = 1.0f - rv * 0.2f;
    rp.damping  = 0.7f;
    rp.width    = 0.8f;
    reverb.setParameters(rp);
    if (buffer.getNumChannels() >= 2)
        reverb.processStereo(buffer.getWritePointer(0), buffer.getWritePointer(1), buffer.getNumSamples());

    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> ctx(block);
    masterGain.setGainLinear(*apvts.getRawParameterValue("STAB_MASTER"));
    masterGain.process(ctx);
}

void StabProcessor::setCurrentProgram(int index) {
    currentProgram = index;
    loadPreset(index);
}

const juce::String StabProcessor::getProgramName(int index) {
    switch (index) {
        case 0: return "Classic";
        case 1: return "Soft";
        case 2: return "Bright";
        case 3: return "Deep";
        default: return "Default";
    }
}

void StabProcessor::loadPreset(int index) {
    auto set = [&](const char* id, float v) {
        if (auto* p = apvts.getParameter(id)) p->setValueNotifyingHost(p->convertTo0to1(v));
    };
    switch (index) {
        case 0: // Classic — punchy house stab, saw, fast decay
            set("STAB_CHORD", 2); set("STAB_WAVE", 0);
            set("STAB_DECAY", 0.18f); set("STAB_REL", 0.08f);
            set("STAB_CUTOFF", 3500.f); set("STAB_RES", 0.55f);
            set("STAB_FENV", 0.65f); set("STAB_DRIVE", 0.15f);
            set("STAB_REVERB", 0.08f); set("STAB_MASTER", 0.8f);
            break;
        case 1: // Soft — clean sine chord, gentle, no drive
            set("STAB_CHORD", 0); set("STAB_WAVE", 2);
            set("STAB_DECAY", 0.4f); set("STAB_REL", 0.3f);
            set("STAB_CUTOFF", 6000.f); set("STAB_RES", 0.2f);
            set("STAB_FENV", 0.1f); set("STAB_DRIVE", 0.0f);
            set("STAB_REVERB", 0.3f); set("STAB_MASTER", 0.75f);
            break;
        case 2: // Bright — upbeat, airy, minor 7, short with reverb tail
            set("STAB_CHORD", 3); set("STAB_WAVE", 2);
            set("STAB_DECAY", 0.22f); set("STAB_REL", 0.5f);
            set("STAB_CUTOFF", 9000.f); set("STAB_RES", 0.25f);
            set("STAB_FENV", 0.2f); set("STAB_DRIVE", 0.0f);
            set("STAB_REVERB", 0.45f); set("STAB_MASTER", 0.75f);
            break;
        case 3: // Deep — warm sus4, longer decay, low filter, full
            set("STAB_CHORD", 4); set("STAB_WAVE", 0);
            set("STAB_DECAY", 0.55f); set("STAB_REL", 0.4f);
            set("STAB_CUTOFF", 1800.f); set("STAB_RES", 0.4f);
            set("STAB_FENV", 0.5f); set("STAB_DRIVE", 0.05f);
            set("STAB_REVERB", 0.2f); set("STAB_MASTER", 0.8f);
            break;
    }
}

void StabProcessor::getStateInformation(juce::MemoryBlock& dest) {
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, dest);
}
void StabProcessor::setStateInformation(const void* data, int size) {
    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, size));
    if (xml && xml->hasTagName(apvts.state.getType()))
        apvts.replaceState(juce::ValueTree::fromXml(*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new StabProcessor(); }
