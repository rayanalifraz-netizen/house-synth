#include "PluginEditor.h"

static const juce::Colour BG       { 0xff141414 };
static const juce::Colour PANEL    { 0xff1e1e1e };
static const juce::Colour ACCENT   { 0xff00d4ff };
static const juce::Colour TEXT     { 0xffdddddd };
static const juce::Colour TEXTDIM  { 0xff888888 };

HouseSynthEditor::HouseSynthEditor(HouseSynthProcessor& p)
    : AudioProcessorEditor(&p), processor(p)
{
    setSize(700, 440);
    setResizable(false, false);

    // Mode / Wave
    modeBox.addItemList({ "Chord", "Bass", "Lead" }, 1);
    waveBox.addItemList({ "Saw", "Square", "Sine" }, 1);
    for (auto* b : { &modeBox, &waveBox }) {
        b->setColour(juce::ComboBox::backgroundColourId, PANEL);
        b->setColour(juce::ComboBox::textColourId, TEXT);
        b->setColour(juce::ComboBox::outlineColourId, ACCENT);
        addAndMakeVisible(b);
    }
    modeAtt = std::make_unique<ComboAttachment>(processor.apvts, "MODE", modeBox);
    waveAtt = std::make_unique<ComboAttachment>(processor.apvts, "WAVE", waveBox);

    modeLabel.setText("MODE", juce::dontSendNotification);
    waveLabel.setText("WAVE", juce::dontSendNotification);
    for (auto* l : { &modeLabel, &waveLabel }) {
        l->setFont(juce::Font(11.0f, juce::Font::bold));
        l->setColour(juce::Label::textColourId, TEXTDIM);
        addAndMakeVisible(l);
    }

    // Preset buttons
    presetChord.setButtonText("House Chord");
    presetBass.setButtonText("House Bass");
    presetLead.setButtonText("House Lead");
    for (auto* btn : { &presetChord, &presetBass, &presetLead }) {
        btn->setColour(juce::TextButton::buttonColourId, PANEL);
        btn->setColour(juce::TextButton::textColourOffId, ACCENT);
        btn->setColour(juce::TextButton::buttonOnColourId, ACCENT);
        addAndMakeVisible(btn);
    }
    presetChord.onClick = [this] { processor.setCurrentProgram(0); };
    presetBass.onClick  = [this] { processor.setCurrentProgram(1); };
    presetLead.onClick  = [this] { processor.setCurrentProgram(2); };

    // OSC knobs
    styleKnob(detuneKnob, detuneLabel, "Detune");
    detuneAtt = std::make_unique<Attachment>(processor.apvts, "DETUNE", detuneKnob);

    // Filter knobs
    styleKnob(cutoffKnob,    cutoffLabel, "Cutoff");
    styleKnob(resonanceKnob, resLabel,    "Res");
    styleKnob(filterEnvKnob, fenvLabel,   "Env Amt");
    cutoffAtt = std::make_unique<Attachment>(processor.apvts, "CUTOFF",         cutoffKnob);
    resAtt    = std::make_unique<Attachment>(processor.apvts, "RESONANCE",      resonanceKnob);
    fenvAtt   = std::make_unique<Attachment>(processor.apvts, "FILTER_ENV_AMT", filterEnvKnob);

    // Amp env
    styleKnob(ampAtkKnob, aAtkL, "Atk");
    styleKnob(ampDecKnob, aDecL, "Dec");
    styleKnob(ampSusKnob, aSusL, "Sus");
    styleKnob(ampRelKnob, aRelL, "Rel");
    aAtkAtt = std::make_unique<Attachment>(processor.apvts, "AMP_ATK", ampAtkKnob);
    aDecAtt = std::make_unique<Attachment>(processor.apvts, "AMP_DEC", ampDecKnob);
    aSusAtt = std::make_unique<Attachment>(processor.apvts, "AMP_SUS", ampSusKnob);
    aRelAtt = std::make_unique<Attachment>(processor.apvts, "AMP_REL", ampRelKnob);

    // Filter env
    styleKnob(fltAtkKnob, fAtkL, "Atk");
    styleKnob(fltDecKnob, fDecL, "Dec");
    styleKnob(fltSusKnob, fSusL, "Sus");
    styleKnob(fltRelKnob, fRelL, "Rel");
    fAtkAtt = std::make_unique<Attachment>(processor.apvts, "FLT_ATK", fltAtkKnob);
    fDecAtt = std::make_unique<Attachment>(processor.apvts, "FLT_DEC", fltDecKnob);
    fSusAtt = std::make_unique<Attachment>(processor.apvts, "FLT_SUS", fltSusKnob);
    fRelAtt = std::make_unique<Attachment>(processor.apvts, "FLT_REL", fltRelKnob);

    // FX
    styleKnob(driveKnob,  driveL,  "Drive");
    styleKnob(chorusKnob, chorusL, "Chorus");
    styleKnob(reverbKnob, reverbL, "Reverb");
    styleKnob(masterKnob, masterL, "Master");
    driveAtt  = std::make_unique<Attachment>(processor.apvts, "DRIVE",  driveKnob);
    chorusAtt = std::make_unique<Attachment>(processor.apvts, "CHORUS", chorusKnob);
    reverbAtt = std::make_unique<Attachment>(processor.apvts, "REVERB", reverbKnob);
    masterAtt = std::make_unique<Attachment>(processor.apvts, "MASTER", masterKnob);

    // Section labels
    for (auto* l : { &ampLabel, &fltLabel, &fxLabel }) {
        l->setFont(juce::Font(11.0f, juce::Font::bold));
        l->setColour(juce::Label::textColourId, ACCENT);
        addAndMakeVisible(l);
    }
    ampLabel.setText("AMP ENV",    juce::dontSendNotification);
    fltLabel.setText("FILTER ENV", juce::dontSendNotification);
    fxLabel.setText("FX",         juce::dontSendNotification);
}

void HouseSynthEditor::styleKnob(juce::Slider& s, juce::Label& l, const juce::String& name) {
    s.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    s.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    s.setColour(juce::Slider::rotarySliderFillColourId, ACCENT);
    s.setColour(juce::Slider::rotarySliderOutlineColourId, PANEL);
    s.setColour(juce::Slider::thumbColourId, ACCENT);
    addAndMakeVisible(s);

    l.setText(name, juce::dontSendNotification);
    l.setJustificationType(juce::Justification::centred);
    l.setFont(juce::Font(10.0f));
    l.setColour(juce::Label::textColourId, TEXTDIM);
    addAndMakeVisible(l);
}

void HouseSynthEditor::paint(juce::Graphics& g) {
    g.fillAll(BG);

    // Title
    g.setColour(ACCENT);
    g.setFont(juce::Font(20.0f, juce::Font::bold));
    g.drawText("HOUSE SYNTH", 20, 12, 200, 28, juce::Justification::left);

    g.setColour(ACCENT.withAlpha(0.3f));
    g.drawHorizontalLine(50, 16, 684);

    // Section boxes
    auto drawBox = [&](juce::Rectangle<int> r, const juce::String&) {
        g.setColour(PANEL);
        g.fillRoundedRectangle(r.toFloat(), 6.0f);
        g.setColour(ACCENT.withAlpha(0.15f));
        g.drawRoundedRectangle(r.toFloat(), 6.0f, 1.0f);
    };
    drawBox({ 16, 54, 200, 90 }, "osc");
    drawBox({ 16, 158, 290, 100 }, "filter");
    drawBox({ 16, 272, 290, 90 }, "amp env");
    drawBox({ 316, 158, 290, 100 }, "flt env");
    drawBox({ 316, 272, 290, 90 }, "fx");
}

void HouseSynthEditor::resized() {
    // Presets
    presetChord.setBounds(300, 14, 110, 26);
    presetBass.setBounds(418, 14, 110, 26);
    presetLead.setBounds(536, 14, 110, 26);

    // OSC section
    modeLabel.setBounds(24, 58, 50, 16);
    modeBox.setBounds(24, 74, 88, 26);
    waveLabel.setBounds(120, 58, 50, 16);
    waveBox.setBounds(120, 74, 88, 26);

    // Detune (under osc)
    detuneKnob.setBounds(70, 100, 54, 54);
    detuneLabel.setBounds(62, 150, 70, 14);

    // Filter section
    int fx = 24, fy = 162, fkw = 58;
    cutoffKnob.setBounds(fx, fy, fkw, fkw);         cutoffLabel.setBounds(fx, fy + fkw, fkw, 14);
    resonanceKnob.setBounds(fx + 76, fy, fkw, fkw); resLabel.setBounds(fx + 76, fy + fkw, fkw, 14);
    filterEnvKnob.setBounds(fx + 152, fy, fkw, fkw); fenvLabel.setBounds(fx + 152, fy + fkw, fkw, 14);

    // Amp Env
    int ae = 24, aey = 276, aw = 54;
    ampLabel.setBounds(ae, aey - 14, 120, 14);
    ampAtkKnob.setBounds(ae,        aey, aw, aw); aAtkL.setBounds(ae,        aey + aw, aw, 14);
    ampDecKnob.setBounds(ae + 60,   aey, aw, aw); aDecL.setBounds(ae + 60,   aey + aw, aw, 14);
    ampSusKnob.setBounds(ae + 120,  aey, aw, aw); aSusL.setBounds(ae + 120,  aey + aw, aw, 14);
    ampRelKnob.setBounds(ae + 180,  aey, aw, aw); aRelL.setBounds(ae + 180,  aey + aw, aw, 14);

    // Filter Env
    int fe = 324, fey = 162;
    fltLabel.setBounds(fe, fey - 14, 120, 14);
    fltAtkKnob.setBounds(fe,        fey, aw, aw); fAtkL.setBounds(fe,        fey + aw, aw, 14);
    fltDecKnob.setBounds(fe + 60,   fey, aw, aw); fDecL.setBounds(fe + 60,   fey + aw, aw, 14);
    fltSusKnob.setBounds(fe + 120,  fey, aw, aw); fSusL.setBounds(fe + 120,  fey + aw, aw, 14);
    fltRelKnob.setBounds(fe + 180,  fey, aw, aw); fRelL.setBounds(fe + 180,  fey + aw, aw, 14);

    // FX
    int xx = 324, xy = 276;
    fxLabel.setBounds(xx, xy - 14, 60, 14);
    driveKnob.setBounds(xx,       xy, aw, aw); driveL.setBounds(xx,       xy + aw, aw, 14);
    chorusKnob.setBounds(xx + 60,  xy, aw, aw); chorusL.setBounds(xx + 60,  xy + aw, aw, 14);
    reverbKnob.setBounds(xx + 120, xy, aw, aw); reverbL.setBounds(xx + 120, xy + aw, aw, 14);
    masterKnob.setBounds(xx + 200, xy, aw, aw); masterL.setBounds(xx + 200, xy + aw, aw, 14);
}

juce::AudioProcessorEditor* HouseSynthProcessor::createEditor() {
    return new HouseSynthEditor(*this);
}
