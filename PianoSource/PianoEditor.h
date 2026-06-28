#pragma once
#include <JuceHeader.h>
#include "PianoProcessor.h"

class PianoEditor : public juce::AudioProcessorEditor {
public:
    explicit PianoEditor(PianoProcessor&);
    void paint(juce::Graphics&) override;
    void resized() override;

private:
    PianoProcessor& proc;

    juce::TextButton presetRhodes, presetBrightEP, presetWarm, presetFunky;

    juce::Slider fmDepthKnob, fmDecayKnob, brightKnob, tremRateKnob, tremDepthKnob;
    juce::Slider atkKnob, decKnob, susKnob, relKnob, reverbKnob, masterKnob;
    juce::Label  fmDepthL, fmDecayL, brightL, tremRateL, tremDepthL;
    juce::Label  atkL, decL, susL, relL, reverbL, masterL;

    using SA = juce::AudioProcessorValueTreeState::SliderAttachment;
    std::unique_ptr<SA> fmDepthAtt, fmDecayAtt, brightAtt, tremRateAtt, tremDepthAtt;
    std::unique_ptr<SA> atkAtt, decAtt, susAtt, relAtt, reverbAtt, masterAtt;

    void styleKnob(juce::Slider&, juce::Label&, const juce::String&);
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PianoEditor)
};
