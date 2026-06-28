#include "SupersawEditor.h"

static const juce::Colour BG     { 0xff141414 };
static const juce::Colour PANEL  { 0xff1e1e1e };
static const juce::Colour ACCENT { 0xffb44fff }; // purple
static const juce::Colour DIM    { 0xff888888 };

SupersawEditor::SupersawEditor(SupersawProcessor& p) : AudioProcessorEditor(&p), proc(p) {
    setSize(600, 220);

    styleKnob(spreadKnob, spreadL, "Spread");
    styleKnob(cutoffKnob, cutoffL, "Cutoff");
    styleKnob(resKnob,    resL,    "Res");
    styleKnob(atkKnob,    atkL,    "Attack");
    styleKnob(susKnob,    susL,    "Sustain");
    styleKnob(relKnob,    relL,    "Release");
    styleKnob(chorusKnob, chorusL, "Chorus");
    styleKnob(reverbKnob, reverbL, "Reverb");
    styleKnob(masterKnob, masterL, "Master");

    spreadAtt = std::make_unique<SA>(proc.apvts, "SS_SPREAD", spreadKnob);
    cutoffAtt = std::make_unique<SA>(proc.apvts, "SS_CUTOFF", cutoffKnob);
    resAtt    = std::make_unique<SA>(proc.apvts, "SS_RES",    resKnob);
    atkAtt    = std::make_unique<SA>(proc.apvts, "SS_ATK",    atkKnob);
    susAtt    = std::make_unique<SA>(proc.apvts, "SS_SUS",    susKnob);
    relAtt    = std::make_unique<SA>(proc.apvts, "SS_REL",    relKnob);
    chorusAtt = std::make_unique<SA>(proc.apvts, "SS_CHORUS", chorusKnob);
    reverbAtt = std::make_unique<SA>(proc.apvts, "SS_REVERB", reverbKnob);
    masterAtt = std::make_unique<SA>(proc.apvts, "SS_MASTER", masterKnob);
}

void SupersawEditor::styleKnob(juce::Slider& s, juce::Label& l, const juce::String& name) {
    s.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    s.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    s.setColour(juce::Slider::rotarySliderFillColourId,    ACCENT);
    s.setColour(juce::Slider::rotarySliderOutlineColourId, PANEL);
    s.setColour(juce::Slider::thumbColourId,               ACCENT);
    addAndMakeVisible(s);
    l.setText(name, juce::dontSendNotification);
    l.setJustificationType(juce::Justification::centred);
    l.setFont(juce::Font(10.f));
    l.setColour(juce::Label::textColourId, DIM);
    addAndMakeVisible(l);
}

void SupersawEditor::paint(juce::Graphics& g) {
    g.fillAll(BG);
    g.setColour(ACCENT);
    g.setFont(juce::Font(20.f, juce::Font::bold));
    g.drawText("SUPERSAW PAD", 20, 12, 300, 28, juce::Justification::left);
    g.setColour(ACCENT.withAlpha(0.3f));
    g.drawHorizontalLine(48, 16, 584);
    g.setColour(PANEL);
    g.fillRoundedRectangle({ 16.f, 54.f, 570.f, 150.f }, 6.f);
    g.setColour(ACCENT.withAlpha(0.15f));
    g.drawRoundedRectangle({ 16.f, 54.f, 570.f, 150.f }, 6.f, 1.f);
}

void SupersawEditor::resized() {
    int x = 24, y = 62, w = 56;
    auto place = [&](juce::Slider& s, juce::Label& l) {
        s.setBounds(x, y, w, w); l.setBounds(x, y + w, w, 14); x += 62;
    };
    place(spreadKnob, spreadL);
    place(cutoffKnob, cutoffL);
    place(resKnob,    resL);
    x += 10;
    place(atkKnob,    atkL);
    place(susKnob,    susL);
    place(relKnob,    relL);
    x += 10;
    place(chorusKnob, chorusL);
    place(reverbKnob, reverbL);
    place(masterKnob, masterL);
}

juce::AudioProcessorEditor* SupersawProcessor::createEditor() { return new SupersawEditor(*this); }
