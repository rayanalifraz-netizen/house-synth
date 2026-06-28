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
