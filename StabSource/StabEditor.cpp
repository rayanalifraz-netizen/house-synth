#include "StabEditor.h"

static const juce::Colour BG     { 0xff141414 };
static const juce::Colour PANEL  { 0xff1e1e1e };
static const juce::Colour ACCENT { 0xffff6b35 }; // orange — distinct from House Synth cyan
static const juce::Colour TEXT   { 0xffdddddd };
static const juce::Colour DIM    { 0xff888888 };

StabEditor::StabEditor(StabProcessor& p) : AudioProcessorEditor(&p), proc(p) {
    setSize(560, 310);

    // Preset buttons
    presetClassic.setButtonText("Classic");
    presetSoft.setButtonText("Soft");
    presetBright.setButtonText("Bright");
    presetDeep.setButtonText("Deep");
    for (auto* btn : { &presetClassic, &presetSoft, &presetBright, &presetDeep }) {
        btn->setColour(juce::TextButton::buttonColourId,  PANEL);
        btn->setColour(juce::TextButton::textColourOffId, ACCENT);
        btn->setColour(juce::TextButton::buttonOnColourId, ACCENT);
        addAndMakeVisible(btn);
    }
    presetClassic.onClick = [this] { proc.loadPreset(0); };
    presetSoft.onClick    = [this] { proc.loadPreset(1); };
    presetBright.onClick  = [this] { proc.loadPreset(2); };
    presetDeep.onClick    = [this] { proc.loadPreset(3); };

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
    g.fillRoundedRectangle({ 16.f,  54.f, 530.f, 40.f  }, 6.f); // presets
    g.fillRoundedRectangle({ 16.f, 106.f, 250.f, 80.f  }, 6.f); // osc
    g.fillRoundedRectangle({ 16.f, 200.f, 530.f, 100.f }, 6.f); // knobs
    g.setColour(ACCENT.withAlpha(0.15f));
    g.drawRoundedRectangle({ 16.f,  54.f, 530.f, 40.f  }, 6.f, 1.f);
    g.drawRoundedRectangle({ 16.f, 106.f, 250.f, 80.f  }, 6.f, 1.f);
    g.drawRoundedRectangle({ 16.f, 200.f, 530.f, 100.f }, 6.f, 1.f);
}

void StabEditor::resized() {
    // Preset buttons
    presetClassic.setBounds(24,  62, 118, 24);
    presetSoft.setBounds(148,    62, 118, 24);
    presetBright.setBounds(272,  62, 118, 24);
    presetDeep.setBounds(396,    62, 118, 24);

    // OSC selectors
    chordLabel.setBounds(24, 110, 60, 14);
    chordBox.setBounds(24,   126, 110, 26);
    waveLabel.setBounds(148, 110, 60, 14);
    waveBox.setBounds(148,   126, 110, 26);

    // Knobs
    int y = 208, w = 54, x = 24;
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
