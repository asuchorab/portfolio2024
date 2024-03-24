/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

#include "PluginProcessor.h"
#include <random>

//==============================================================================
/**
 */
class MidiCAAudioProcessorEditor : public juce::AudioProcessorEditor, juce::ChangeListener, juce::Timer {
 public:
  MidiCAAudioProcessorEditor(MidiCAAudioProcessor&);
  ~MidiCAAudioProcessorEditor() override;

  //==============================================================================
  void paint(juce::Graphics&) override;
  void resized() override;

  void timerCallback();

 private:
  void updateWindowOffsetRange();
  void updateGUIValues();
  // This reference is provided as a quick way for your editor to
  // access the processor object that created it.
  MidiCAAudioProcessor& audioProcessor;
  juce::ComboBox division;
  juce::Label divisionLabel;
  juce::ComboBox neighbourhood;
  juce::Label neighbourhoodLabel;
  juce::TextEditor rule;
  juce::Label ruleLabel;
  juce::TextButton ruleRandomise;
  juce::Slider windowWidth;
  juce::Label windowWidthLabel;
  juce::Slider windowOffset;
  juce::Label windowOffsetLabel;
  juce::ComboBox pattern;
  juce::Label patternLabel;
  juce::Slider patternOffset;
  juce::Label patternOffsetLabel;
  juce::Slider patternSpacing;
  juce::Label patternSpacingLabel;
  juce::ToggleButton repeatNotes;
  juce::Label repeatNotesLabel;
  juce::ToggleButton randomInitialisation;
  juce::Label randomInitialisationLabel;
  juce::TextEditor initialisationSeed;
  juce::Label initialisationSeedLabel;
  juce::TextButton initialisationSeedRandomise;
  juce::Slider bufferSpan;
  juce::Label bufferSpanLabel;
  DisplayData displayData;
  std::mt19937 random;

  // Inherited via ChangeListener
  virtual void changeListenerCallback(juce::ChangeBroadcaster* source) override;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MidiCAAudioProcessorEditor)
};
