#pragma once
#include <JuceHeader.h>
#include "StabProcessor.h"

class StabEditor : public juce::AudioProcessorEditor {
public:
    explicit StabEditor(StabProcessor&);
    void paint(juce::Graphics&) override;
    void resized() override;

private:
    StabProcessor& proc;

    juce::ComboBox chordBox, waveBox;
    juce::Label chordLabel, waveLabel;

    juce::Slider decayKnob, relKnob, cutoffKnob, resKnob, fenvKnob, driveKnob, reverbKnob, masterKnob;
    juce::Label  decayL, relL, cutoffL, resL, fenvL, driveL, reverbL, masterL;

    using SA = juce::AudioProcessorValueTreeState::SliderAttachment;
    using CA = juce::AudioProcessorValueTreeState::ComboBoxAttachment;

    std::unique_ptr<CA> chordAtt, waveAtt;
    std::unique_ptr<SA> decayAtt, relAtt, cutoffAtt, resAtt, fenvAtt, driveAtt, reverbAtt, masterAtt;

    void styleKnob(juce::Slider&, juce::Label&, const juce::String&);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(StabEditor)
};
