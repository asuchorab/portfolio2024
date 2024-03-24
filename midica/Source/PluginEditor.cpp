/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginEditor.h"

#include <ctime>

#include "Globals.h"
#include "PluginProcessor.h"

//==============================================================================
MidiCAAudioProcessorEditor::MidiCAAudioProcessorEditor(MidiCAAudioProcessor& p)
    : AudioProcessorEditor(&p),
      audioProcessor(p),
      ruleRandomise("Random rule"),
      initialisationSeedRandomise("Random seed") {
  // Make sure that before the constructor has finished, you've set the
  // editor's size to whatever you need it to be.
  random.seed(std::time(nullptr));
  setSize(600, 400);
  addAndMakeVisible(division);
  addAndMakeVisible(rule);
  addAndMakeVisible(ruleRandomise);
  addAndMakeVisible(neighbourhood);
  addAndMakeVisible(windowWidth);
  addAndMakeVisible(windowOffset);
  addAndMakeVisible(pattern);
  addAndMakeVisible(patternOffset);
  addAndMakeVisible(patternSpacing);
  addAndMakeVisible(repeatNotes);
  addAndMakeVisible(randomInitialisation);
  addAndMakeVisible(initialisationSeed);
  addAndMakeVisible(initialisationSeedRandomise);
  addAndMakeVisible(bufferSpan);
  divisionLabel.setText("Division", juce::dontSendNotification);
  divisionLabel.attachToComponent(&division, true);
  for (int i = 0; i < DIV_OPTIONS_NUM; ++i) {
    division.addItem(DIV_OPTIONS_STRINGS[i], i + 1);
  }
  division.onChange = [this] {
    int selected = division.getSelectedItemIndex();
    audioProcessor.setDivision(selected);
  };
  neighbourhoodLabel.setText("Neighbourh.", juce::dontSendNotification);
  neighbourhoodLabel.attachToComponent(&neighbourhood, true);
  neighbourhood.addItem("3 (8-bit rule)", 1);
  neighbourhood.addItem("5 (32-bit rule)", 2);
  neighbourhood.onChange = [this] {
    int selected = neighbourhood.getSelectedItemIndex();
    audioProcessor.forceNoteOff();
    audioProcessor.getCA()->setNeighbourhood(selected == 0 ? 3 : 5);
    if (selected == 0) {
      int64_t value = std::atoll(rule.getText().toRawUTF8());
      if (value > 255) {
        rule.setText("255");
        audioProcessor.getCA()->setRule(255);
      }
    }
  };
  ruleLabel.setText("Rule", juce::dontSendNotification);
  ruleLabel.attachToComponent(&rule, true);
  rule.setInputRestrictions(0, "0123456789");
  rule.onTextChange = [this] {
    int64_t value = std::atoll(rule.getText().toRawUTF8());
    if (neighbourhood.getSelectedItemIndex() == 0) {
      if (value > 255) {
        value = 255;
        rule.setText("255");
      }
    } else {
      if (value > 4294967295ll) {
        value = 4294967295;
        rule.setText("4294967295");
      }
    }
    audioProcessor.getCA()->setRule(value);
  };
  ruleRandomise.onClick = [this] {
    int64_t value;
    if (neighbourhood.getSelectedItemIndex() == 0) {
      value = random() % 255;
    } else {
      value = random();
    }
    rule.setText(std::to_string(value));
  };
  windowWidthLabel.setText("Width", juce::dontSendNotification);
  windowWidthLabel.attachToComponent(&windowWidth, true);
  windowWidth.setRange(1, 20, 1);
  windowWidth.setTextBoxStyle(juce::Slider::TextBoxLeft, false, 40, 30);
  windowWidth.onValueChange = [this] {
    audioProcessor.getGenerator()->setWindowWidth(windowWidth.getValue());
    updateWindowOffsetRange();
    repaint();
  };
  windowOffsetLabel.setText("Offset", juce::dontSendNotification);
  windowOffsetLabel.attachToComponent(&windowOffset, true);
  updateWindowOffsetRange();
  windowOffset.setTextBoxStyle(juce::Slider::TextBoxLeft, false, 40, 30);
  windowOffset.onValueChange = [this] {
    audioProcessor.getGenerator()->setWindowOffset(windowOffset.getValue());
    repaint();
  };
  patternLabel.setText("Pattern", juce::dontSendNotification);
  patternLabel.attachToComponent(&pattern, true);
  for (int i = 0; i < NOTE_PATTERNS_NUM; ++i) {
    pattern.addItem(NOTE_PATTERNS_STRINGS[i], i + 1);
  }
  pattern.onChange = [this] {
    int selected = pattern.getSelectedItemIndex();
    audioProcessor.getGenerator()->setPattern(selected);
  };
  patternOffsetLabel.setText("Pat. offset", juce::dontSendNotification);
  patternOffsetLabel.attachToComponent(&patternOffset, true);
  patternOffset.setRange(-9, 9, 1);
  patternOffset.setTextBoxStyle(juce::Slider::TextBoxLeft, false, 40, 30);
  patternOffset.onValueChange = [this] {
    audioProcessor.getGenerator()->setPatternOffset(patternOffset.getValue());
  };
  patternSpacingLabel.setText("Pat. spac.", juce::dontSendNotification);
  patternSpacingLabel.attachToComponent(&patternSpacing, true);
  patternSpacing.setRange(1, 8, 1);
  patternSpacing.setTextBoxStyle(juce::Slider::TextBoxLeft, false, 40, 30);
  patternSpacing.onValueChange = [this] {
    audioProcessor.getGenerator()->setPatternSpacing(patternSpacing.getValue());
  };
  repeatNotesLabel.setText("Repeat", juce::dontSendNotification);
  repeatNotesLabel.attachToComponent(&repeatNotes, true);
  repeatNotes.onClick = [this] {
    auto enabled = repeatNotes.getToggleState();
    audioProcessor.getGenerator()->setDoRepeatNotes(enabled);
  };
  randomInitialisationLabel.setText("Random init", juce::dontSendNotification);
  randomInitialisationLabel.attachToComponent(&randomInitialisation, true);
  randomInitialisation.onClick = [this] {
    auto enabled = randomInitialisation.getToggleState();
    audioProcessor.getCA()->setRandomInitialisation(enabled);
  };
  initialisationSeedLabel.setText("Init seed", juce::dontSendNotification);
  initialisationSeedLabel.attachToComponent(&initialisationSeed, true);
  initialisationSeed.setInputRestrictions(0, "0123456789");
  initialisationSeed.onTextChange = [this] {
    int64_t value = std::atoll(initialisationSeed.getText().toRawUTF8());
    if (value > 4294967295ll) {
      value = 4294967295;
      rule.setText("4294967295");
    }
    audioProcessor.getCA()->setSeed(value);
  };
  initialisationSeedRandomise.onClick = [this] {
    int64_t value = random();
    initialisationSeed.setText(std::to_string(value));
  };
  bufferSpanLabel.setText("Buffer span", juce::dontSendNotification);
  bufferSpanLabel.attachToComponent(&bufferSpan, true);
  bufferSpan.setRange(30, 300, 1);
  bufferSpan.setTextBoxStyle(juce::Slider::TextBoxLeft, false, 40, 30);
  bufferSpan.onValueChange = [this] {
    audioProcessor.forceNoteOff();
    audioProcessor.getCA()->setBufferSpan(bufferSpan.getValue());
    repaint();
  };
  updateGUIValues();
  startTimer(100);
  audioProcessor.getGenerator()->addChangeListener(this);
};

MidiCAAudioProcessorEditor::~MidiCAAudioProcessorEditor() {
  audioProcessor.getGenerator()->removeChangeListener(this);
}

//==============================================================================
void MidiCAAudioProcessorEditor::paint(juce::Graphics& g) {
  // (Our component is opaque, so we must completely fill the background with
  // a solid colour)
  g.fillAll(
      getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
  audioProcessor.outputDisplayData(displayData);
  g.setColour(juce::Colours::white);

  g.setFont(15.0f);
  // g.drawFittedText(
  //    "BPM: " + std::to_string((int) audioProcessor.getLastBPM()),
  //    getLocalBounds(), juce::Justification::centred, 1);

  g.drawFittedText("BPM: " + std::to_string((int)audioProcessor.getLastBPM()),
                   {0, 0, 200, 20}, juce::Justification::centred, 1);

  // g.drawFittedText("Division", {10, 20, 90, 20}, juce::Justification::left,
  // 1);

  audioProcessor.getGenerator()->paint(g);
}

void MidiCAAudioProcessorEditor::resized() {
  // This is generally where you'll want to lay out the positions of any
  // subcomponents in your editor..
  const int x = 70;
  const int width = 120;
  const int width2 = 130;
  const int width_button = 140;
  const int x_button = x - 40;
  const int height = 25;
  const int row_spacing = 28;
  int y = 0;
  division.setBounds(x, y += row_spacing, width, height);
  neighbourhood.setBounds(x, y += row_spacing, width, height);
  rule.setBounds(x, y += row_spacing, width, height);
  ruleRandomise.setBounds(x_button, y += row_spacing, width_button, height);
  windowWidth.setBounds(x, y += row_spacing, width2, height);
  windowOffset.setBounds(x, y += row_spacing, width2, height);
  pattern.setBounds(x, y += row_spacing, width, height);
  patternOffset.setBounds(x, y += row_spacing, width2, height);
  patternSpacing.setBounds(x, y += row_spacing, width2, height);
  repeatNotes.setBounds(170, y += row_spacing, 30, height);
  randomInitialisation.setBounds(x, y, 30, height);
  initialisationSeed.setBounds(x, y += row_spacing, width, height);
  initialisationSeedRandomise.setBounds(x_button, y += row_spacing,
                                        width_button, height);
  bufferSpan.setBounds(x, y += row_spacing, width2, height);
}

void MidiCAAudioProcessorEditor::timerCallback() {
  if (audioProcessor.checkAndClearChangedFlag()) {
    updateGUIValues();
  }
}

void MidiCAAudioProcessorEditor::updateWindowOffsetRange() {
  int width = windowWidth.getValue();
  int window_begin = (20 - width) / 2;
  int range_begin = -window_begin;
  int range_end = range_begin + 20;
  int current_value = windowOffset.getValue();
  windowOffset.setRange(range_begin, range_end, 1);
  if (current_value < range_begin) {
    windowOffset.setValue(range_begin);
    audioProcessor.getGenerator()->setWindowOffset(range_begin);
  } else if (current_value >= range_end) {
    windowOffset.setValue(range_end - 1);
    audioProcessor.getGenerator()->setWindowOffset(range_end - 1);
  }
}

void MidiCAAudioProcessorEditor::updateGUIValues() {
  division.setSelectedId(audioProcessor.getDivision() + 1,
                         juce::dontSendNotification);
  neighbourhood.setSelectedId(
      audioProcessor.getCA()->getNeighbourhood() == 3 ? 1 : 2,
      juce::dontSendNotification);
  rule.setText(std::to_string(audioProcessor.getCA()->getRule()),
               juce::dontSendNotification);
  windowWidth.setValue(audioProcessor.getGenerator()->getWindowWidth(),
                       juce::dontSendNotification);
  windowOffset.setValue(audioProcessor.getGenerator()->getWindowOffset(),
                        juce::dontSendNotification);
  pattern.setSelectedId(audioProcessor.getGenerator()->getPattern() + 1,
                        juce::dontSendNotification);
  patternOffset.setValue(audioProcessor.getGenerator()->getPatternOffset(),
                        juce::dontSendNotification);
  patternSpacing.setValue(audioProcessor.getGenerator()->getPatternSpacing(),
                          juce::dontSendNotification);
  repeatNotes.setToggleState(
      audioProcessor.getGenerator()->getDoRepeatNotes(),
      juce::dontSendNotification);
  randomInitialisation.setToggleState(
      audioProcessor.getCA()->getDoRandomInitialisation(),
      juce::dontSendNotification);
  initialisationSeed.setText(std::to_string(audioProcessor.getCA()->getSeed()),
                             juce::dontSendNotification);
  bufferSpan.setValue(audioProcessor.getCA()->getBufferSpan(),
                      juce::dontSendNotification);
}

// void MidiCAAudioProcessorEditor::timerCallback() { repaint(); }

void MidiCAAudioProcessorEditor::changeListenerCallback(
    juce::ChangeBroadcaster* source) {
  repaint();
}
