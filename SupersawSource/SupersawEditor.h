#pragma once
#include <JuceHeader.h>
#include "SupersawProcessor.h"

class SupersawEditor : public juce::AudioProcessorEditor {
public:
    explicit SupersawEditor(SupersawProcessor&);
    void paint(juce::Graphics&) override;
    void resized() override;

private:
    SupersawProcessor& proc;

    juce::Slider spreadKnob, cutoffKnob, resKnob, atkKnob, susKnob, relKnob, chorusKnob, reverbKnob, masterKnob;
    juce::Label  spreadL, cutoffL, resL, atkL, susL, relL, chorusL, reverbL, masterL;

    using SA = juce::AudioProcessorValueTreeState::SliderAttachment;
    std::unique_ptr<SA> spreadAtt, cutoffAtt, resAtt, atkAtt, susAtt, relAtt, chorusAtt, reverbAtt, masterAtt;

    void styleKnob(juce::Slider&, juce::Label&, const juce::String&);
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SupersawEditor)
};
