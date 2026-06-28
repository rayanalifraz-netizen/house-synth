#include "StabEditor.h"

static const juce::Colour BG     { 0xff141414 };
static const juce::Colour PANEL  { 0xff1e1e1e };
static const juce::Colour ACCENT { 0xffff6b35 }; // orange — distinct from House Synth cyan
static const juce::Colour TEXT   { 0xffdddddd };
static const juce::Colour DIM    { 0xff888888 };

StabEditor::StabEditor(StabProcessor& p) : AudioProcessorEditor(&p), proc(p) {
    setSize(560, 280);

    chordBox.addItemList({ "Major", "Minor", "Dom 7", "Min 7", "Sus4" }, 1);
    waveBox.addItemList({ "Saw", "Square", "Sine" }, 1);
    for (auto* b : { &chordBox, &waveBox }) {
        b->setColour(juce::ComboBox::backgroundColourId, PANEL);
        b->setColour(juce::ComboBox::textColourId, TEXT);
        b->setColour(juce::ComboBox::outlineColourId, ACCENT);
        addAndMakeVisible(b);
    }
    chordAtt = std::make_unique<CA>(proc.apvts, "STAB_CHORD", chordBox);
    waveAtt  = std::make_unique<CA>(proc.apvts, "STAB_WAVE",  waveBox);

    chordLabel.setText("CHORD", juce::dontSendNotification);
    waveLabel.setText("WAVE",   juce::dontSendNotification);
    for (auto* l : { &chordLabel, &waveLabel }) {
        l->setFont(juce::Font(11.f, juce::Font::bold));
        l->setColour(juce::Label::textColourId, DIM);
        addAndMakeVisible(l);
    }

    styleKnob(decayKnob,  decayL,  "Decay");
    styleKnob(relKnob,    relL,    "Release");
    styleKnob(cutoffKnob, cutoffL, "Cutoff");
    styleKnob(resKnob,    resL,    "Res");
    styleKnob(fenvKnob,   fenvL,   "Filter Env");
    styleKnob(driveKnob,  driveL,  "Drive");
    styleKnob(reverbKnob, reverbL, "Reverb");
    styleKnob(masterKnob, masterL, "Master");

    decayAtt  = std::make_unique<SA>(proc.apvts, "STAB_DECAY",  decayKnob);
    relAtt    = std::make_unique<SA>(proc.apvts, "STAB_REL",    relKnob);
    cutoffAtt = std::make_unique<SA>(proc.apvts, "STAB_CUTOFF", cutoffKnob);
    resAtt    = std::make_unique<SA>(proc.apvts, "STAB_RES",    resKnob);
    fenvAtt   = std::make_unique<SA>(proc.apvts, "STAB_FENV",   fenvKnob);
    driveAtt  = std::make_unique<SA>(proc.apvts, "STAB_DRIVE",  driveKnob);
    reverbAtt = std::make_unique<SA>(proc.apvts, "STAB_REVERB", reverbKnob);
    masterAtt = std::make_unique<SA>(proc.apvts, "STAB_MASTER", masterKnob);
}

void StabEditor::styleKnob(juce::Slider& s, juce::Label& l, const juce::String& name) {
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

void StabEditor::paint(juce::Graphics& g) {
    g.fillAll(BG);
    g.setColour(ACCENT);
    g.setFont(juce::Font(20.f, juce::Font::bold));
    g.drawText("HOUSE STAB", 20, 12, 300, 28, juce::Justification::left);
    g.setColour(ACCENT.withAlpha(0.3f));
    g.drawHorizontalLine(48, 16, 544);
    g.setColour(PANEL);
    g.fillRoundedRectangle({ 16.f, 54.f, 250.f, 80.f }, 6.f);
    g.fillRoundedRectangle({ 16.f, 150.f, 530.f, 114.f }, 6.f);
    g.setColour(ACCENT.withAlpha(0.15f));
    g.drawRoundedRectangle({ 16.f, 54.f, 250.f, 80.f }, 6.f, 1.f);
    g.drawRoundedRectangle({ 16.f, 150.f, 530.f, 114.f }, 6.f, 1.f);
}

void StabEditor::resized() {
    chordLabel.setBounds(24,  58, 60, 14);
    chordBox.setBounds(24,    74, 110, 26);
    waveLabel.setBounds(148,  58, 60, 14);
    waveBox.setBounds(148,    74, 110, 26);

    int y = 158, w = 54;
    int x = 24;
    auto place = [&](juce::Slider& s, juce::Label& l) {
        s.setBounds(x, y, w, w); l.setBounds(x, y + w, w, 14); x += 66;
    };
    place(decayKnob,  decayL);
    place(relKnob,    relL);
    place(cutoffKnob, cutoffL);
    place(resKnob,    resL);
    place(fenvKnob,   fenvL);
    place(driveKnob,  driveL);
    place(reverbKnob, reverbL);
    place(masterKnob, masterL);
}

juce::AudioProcessorEditor* StabProcessor::createEditor() { return new StabEditor(*this); }
