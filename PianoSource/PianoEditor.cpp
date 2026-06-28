#include "PianoEditor.h"

static const juce::Colour BG     { 0xff141414 };
static const juce::Colour PANEL  { 0xff1e1e1e };
static const juce::Colour ACCENT { 0xffffcc44 }; // warm gold
static const juce::Colour DIM    { 0xff888888 };

PianoEditor::PianoEditor(PianoProcessor& p) : AudioProcessorEditor(&p), proc(p) {
    setSize(680, 240);

    styleKnob(fmDepthKnob,  fmDepthL,  "FM Depth");
    styleKnob(fmDecayKnob,  fmDecayL,  "FM Decay");
    styleKnob(brightKnob,   brightL,   "Brightness");
    styleKnob(tremRateKnob, tremRateL, "Trem Rate");
    styleKnob(tremDepthKnob,tremDepthL,"Trem Depth");
    styleKnob(atkKnob,  atkL,  "Attack");
    styleKnob(decKnob,  decL,  "Decay");
    styleKnob(susKnob,  susL,  "Sustain");
    styleKnob(relKnob,  relL,  "Release");
    styleKnob(reverbKnob, reverbL, "Reverb");
    styleKnob(masterKnob, masterL, "Master");

    fmDepthAtt  = std::make_unique<SA>(proc.apvts, "EP_FM_DEPTH",   fmDepthKnob);
    fmDecayAtt  = std::make_unique<SA>(proc.apvts, "EP_FM_DECAY",   fmDecayKnob);
    brightAtt   = std::make_unique<SA>(proc.apvts, "EP_BRIGHT",     brightKnob);
    tremRateAtt = std::make_unique<SA>(proc.apvts, "EP_TREM_RATE",  tremRateKnob);
    tremDepthAtt= std::make_unique<SA>(proc.apvts, "EP_TREM_DEPTH", tremDepthKnob);
    atkAtt      = std::make_unique<SA>(proc.apvts, "EP_ATK",        atkKnob);
    decAtt      = std::make_unique<SA>(proc.apvts, "EP_DEC",        decKnob);
    susAtt      = std::make_unique<SA>(proc.apvts, "EP_SUS",        susKnob);
    relAtt      = std::make_unique<SA>(proc.apvts, "EP_REL",        relKnob);
    reverbAtt   = std::make_unique<SA>(proc.apvts, "EP_REVERB",     reverbKnob);
    masterAtt   = std::make_unique<SA>(proc.apvts, "EP_MASTER",     masterKnob);
}

void PianoEditor::styleKnob(juce::Slider& s, juce::Label& l, const juce::String& name) {
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

void PianoEditor::paint(juce::Graphics& g) {
    g.fillAll(BG);
    g.setColour(ACCENT);
    g.setFont(juce::Font(20.f, juce::Font::bold));
    g.drawText("HOUSE PIANO", 20, 12, 300, 28, juce::Justification::left);
    g.setColour(ACCENT.withAlpha(0.3f));
    g.drawHorizontalLine(48, 16, 664);

    auto box = [&](juce::Rectangle<float> r, const juce::String& label) {
        g.setColour(PANEL);
        g.fillRoundedRectangle(r, 6.f);
        g.setColour(ACCENT.withAlpha(0.15f));
        g.drawRoundedRectangle(r, 6.f, 1.f);
        g.setColour(ACCENT);
        g.setFont(juce::Font(10.f, juce::Font::bold));
        g.drawText(label, (int)r.getX() + 6, (int)r.getY() - 14, 100, 14, juce::Justification::left);
    };
    box({ 16.f, 68.f, 320.f, 150.f }, "FM / TONE");
    box({ 348.f, 68.f, 320.f, 150.f }, "ENVELOPE / FX");
}

void PianoEditor::resized() {
    int y = 78, w = 54, x = 24;
    auto place = [&](juce::Slider& s, juce::Label& l) {
        s.setBounds(x, y, w, w); l.setBounds(x, y + w, w, 14); x += 62;
    };
    place(fmDepthKnob,  fmDepthL);
    place(fmDecayKnob,  fmDecayL);
    place(brightKnob,   brightL);
    place(tremRateKnob, tremRateL);
    place(tremDepthKnob,tremDepthL);

    x = 356;
    place(atkKnob,    atkL);
    place(decKnob,    decL);
    place(susKnob,    susL);
    place(relKnob,    relL);
    place(reverbKnob, reverbL);
    place(masterKnob, masterL);
}

juce::AudioProcessorEditor* PianoProcessor::createEditor() { return new PianoEditor(*this); }
