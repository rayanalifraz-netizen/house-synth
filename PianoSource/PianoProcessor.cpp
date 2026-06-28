#include "PianoProcessor.h"
#include "PianoEditor.h"

juce::AudioProcessorValueTreeState::ParameterLayout PianoProcessor::createParams() {
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> p;
    p.push_back(std::make_unique<juce::AudioParameterFloat>("EP_FM_DEPTH", "FM Depth",
        juce::NormalisableRange<float>(0.5f, 6.0f, 0.01f), 3.0f));
    p.push_back(std::make_unique<juce::AudioParameterFloat>("EP_FM_DECAY", "FM Decay",
        juce::NormalisableRange<float>(0.05f, 2.0f, 0.001f, 0.4f), 0.3f));
    p.push_back(std::make_unique<juce::AudioParameterFloat>("EP_BRIGHT",   "Brightness",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.55f));
    p.push_back(std::make_unique<juce::AudioParameterFloat>("EP_TREM_RATE",  "Tremolo Rate",
        juce::NormalisableRange<float>(0.5f, 10.0f, 0.1f), 5.0f));
    p.push_back(std::make_unique<juce::AudioParameterFloat>("EP_TREM_DEPTH", "Tremolo Depth",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.0f));
    p.push_back(std::make_unique<juce::AudioParameterFloat>("EP_ATK", "Attack",
        juce::NormalisableRange<float>(0.001f, 0.5f, 0.001f, 0.4f), 0.005f));
    p.push_back(std::make_unique<juce::AudioParameterFloat>("EP_DEC", "Decay",
        juce::NormalisableRange<float>(0.05f, 3.0f, 0.001f, 0.4f), 0.8f));
    p.push_back(std::make_unique<juce::AudioParameterFloat>("EP_SUS", "Sustain",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.5f));
    p.push_back(std::make_unique<juce::AudioParameterFloat>("EP_REL", "Release",
        juce::NormalisableRange<float>(0.01f, 3.0f, 0.001f, 0.4f), 0.6f));
    p.push_back(std::make_unique<juce::AudioParameterFloat>("EP_REVERB", "Reverb",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.25f));
    p.push_back(std::make_unique<juce::AudioParameterFloat>("EP_MASTER", "Master",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.8f));
    return { p.begin(), p.end() };
}

PianoProcessor::PianoProcessor()
    : AudioProcessor(BusesProperties().withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "EP_PARAMS", createParams())
{
    for (int i = 0; i < 12; ++i) {
        synth.addVoice(new PianoVoice());
        synth.addSound(new PianoSound());
    }
}

void PianoProcessor::prepareToPlay(double sr, int block) {
    synth.setCurrentPlaybackSampleRate(sr);
    for (int i = 0; i < synth.getNumVoices(); ++i)
        if (auto* v = dynamic_cast<PianoVoice*>(synth.getVoice(i)))
            v->prepareToPlay(sr, block, 2);

    reverb.setSampleRate(sr);
    juce::dsp::ProcessSpec spec { sr, (juce::uint32)block, 2 };
    masterGain.prepare(spec);
}

void PianoProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi) {
    juce::ScopedNoDenormals noDenormals;
    buffer.clear();

    for (int i = 0; i < synth.getNumVoices(); ++i)
        if (auto* v = dynamic_cast<PianoVoice*>(synth.getVoice(i)))
            v->updateParams(apvts);

    synth.renderNextBlock(buffer, midi, 0, buffer.getNumSamples());

    float rv = *apvts.getRawParameterValue("EP_REVERB");
    juce::Reverb::Parameters rp;
    rp.roomSize = 0.5f + rv * 0.3f;
    rp.wetLevel = rv * 0.45f;
    rp.dryLevel = 1.0f - rv * 0.2f;
    rp.damping  = 0.5f;
    rp.width    = 0.9f;
    reverb.setParameters(rp);
    if (buffer.getNumChannels() >= 2)
        reverb.processStereo(buffer.getWritePointer(0), buffer.getWritePointer(1), buffer.getNumSamples());

    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> ctx(block);
    masterGain.setGainLinear(*apvts.getRawParameterValue("EP_MASTER"));
    masterGain.process(ctx);
}

void PianoProcessor::getStateInformation(juce::MemoryBlock& dest) {
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, dest);
}
void PianoProcessor::setStateInformation(const void* data, int size) {
    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, size));
    if (xml && xml->hasTagName(apvts.state.getType()))
        apvts.replaceState(juce::ValueTree::fromXml(*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new PianoProcessor(); }
