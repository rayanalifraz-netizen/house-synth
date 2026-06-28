#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

class HouseSynthEditor : public juce::AudioProcessorEditor {
public:
    explicit HouseSynthEditor(HouseSynthProcessor&);
    ~HouseSynthEditor() override = default;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    HouseSynthProcessor& processor;

    // Mode selector
    juce::ComboBox modeBox;
    juce::ComboBox waveBox;
    juce::Label    modeLabel, waveLabel;

    // Knobs
    juce::Slider detuneKnob, cutoffKnob, resonanceKnob, filterEnvKnob;
    juce::Slider ampAtkKnob, ampDecKnob, ampSusKnob, ampRelKnob;
    juce::Slider fltAtkKnob, fltDecKnob, fltSusKnob, fltRelKnob;
    juce::Slider driveKnob, chorusKnob, reverbKnob, masterKnob;

    juce::Label detuneLabel, cutoffLabel, resLabel, fenvLabel;
    juce::Label ampLabel, fltLabel, fxLabel;
    juce::Label aAtkL, aDecL, aSusL, aRelL;
    juce::Label fAtkL, fDecL, fSusL, fRelL;
    juce::Label driveL, chorusL, reverbL, masterL;

    // Presets
    juce::TextButton presetChord, presetBass, presetLead;

    // Attachments
    using Attachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ComboAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;

    std::unique_ptr<ComboAttachment> modeAtt, waveAtt;
    std::unique_ptr<Attachment> detuneAtt, cutoffAtt, resAtt, fenvAtt;
    std::unique_ptr<Attachment> aAtkAtt, aDecAtt, aSusAtt, aRelAtt;
    std::unique_ptr<Attachment> fAtkAtt, fDecAtt, fSusAtt, fRelAtt;
    std::unique_ptr<Attachment> driveAtt, chorusAtt, reverbAtt, masterAtt;

    void styleKnob(juce::Slider& s, juce::Label& l, const juce::String& name);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(HouseSynthEditor)
};
