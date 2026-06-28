#include "SupersawProcessor.h"
#include "SupersawEditor.h"

juce::AudioProcessorValueTreeState::ParameterLayout SupersawProcessor::createParams() {
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> p;
    p.push_back(std::make_unique<juce::AudioParameterFloat>("SS_SPREAD", "Spread",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.4f));
    p.push_back(std::make_unique<juce::AudioParameterFloat>("SS_CUTOFF", "Cutoff",
        juce::NormalisableRange<float>(80.0f, 18000.0f, 1.0f, 0.3f), 7000.0f));
    p.push_back(std::make_unique<juce::AudioParameterFloat>("SS_RES", "Resonance",
        juce::NormalisableRange<float>(0.1f, 0.95f, 0.01f), 0.2f));
    p.push_back(std::make_unique<juce::AudioParameterFloat>("SS_ATK", "Attack",
        juce::NormalisableRange<float>(0.01f, 4.0f, 0.001f, 0.4f), 0.1f));
    p.push_back(std::make_unique<juce::AudioParameterFloat>("SS_SUS", "Sustain",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.9f));
    p.push_back(std::make_unique<juce::AudioParameterFloat>("SS_REL", "Release",
        juce::NormalisableRange<float>(0.05f, 4.0f, 0.001f, 0.4f), 1.0f));
    p.push_back(std::make_unique<juce::AudioParameterFloat>("SS_CHORUS", "Chorus",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.5f));
    p.push_back(std::make_unique<juce::AudioParameterFloat>("SS_REVERB", "Reverb",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.4f));
    p.push_back(std::make_unique<juce::AudioParameterFloat>("SS_MASTER", "Master",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.75f));
    return { p.begin(), p.end() };
}

SupersawProcessor::SupersawProcessor()
    : AudioProcessor(BusesProperties().withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "SS_PARAMS", createParams())
{
    for (int i = 0; i < 8; ++i) {
        synth.addVoice(new SupersawVoice());
        synth.addSound(new SupersawSound());
    }
}

void SupersawProcessor::prepareToPlay(double sr, int block) {
    synth.setCurrentPlaybackSampleRate(sr);
    for (int i = 0; i < synth.getNumVoices(); ++i)
        if (auto* v = dynamic_cast<SupersawVoice*>(synth.getVoice(i)))
            v->prepareToPlay(sr, block, 2);

    juce::dsp::ProcessSpec spec { sr, (juce::uint32)block, 2 };
    chorus.prepare(spec);
    chorus.setRate(0.4f);
    chorus.setDepth(0.03f);
    chorus.setCentreDelay(8.0f);
    chorus.setFeedback(0.1f);
    reverb.setSampleRate(sr);
    masterGain.prepare(spec);
}

void SupersawProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi) {
    juce::ScopedNoDenormals noDenormals;
    buffer.clear();

    for (int i = 0; i < synth.getNumVoices(); ++i)
        if (auto* v = dynamic_cast<SupersawVoice*>(synth.getVoice(i)))
            v->updateParams(apvts);

    synth.renderNextBlock(buffer, midi, 0, buffer.getNumSamples());

    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> ctx(block);

    chorus.setMix(*apvts.getRawParameterValue("SS_CHORUS"));
    chorus.process(ctx);

    float rv = *apvts.getRawParameterValue("SS_REVERB");
    juce::Reverb::Parameters rp;
    rp.roomSize = 0.6f + rv * 0.35f;
    rp.wetLevel = rv * 0.55f;
    rp.dryLevel = 1.0f - rv * 0.25f;
    rp.damping  = 0.4f;
    rp.width    = 1.0f;
    reverb.setParameters(rp);
    if (buffer.getNumChannels() >= 2)
        reverb.processStereo(buffer.getWritePointer(0), buffer.getWritePointer(1), buffer.getNumSamples());

    masterGain.setGainLinear(*apvts.getRawParameterValue("SS_MASTER"));
    masterGain.process(ctx);
}

void SupersawProcessor::getStateInformation(juce::MemoryBlock& dest) {
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, dest);
}
void SupersawProcessor::setStateInformation(const void* data, int size) {
    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, size));
    if (xml && xml->hasTagName(apvts.state.getType()))
        apvts.replaceState(juce::ValueTree::fromXml(*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new SupersawProcessor(); }
