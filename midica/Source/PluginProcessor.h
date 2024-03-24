/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

#include "ElementaryCA.h"
#include "CAMIDIGenerator.h"

struct DisplayData {
  bool running;
  bool leftSquare;
};

//==============================================================================
/**
 */
class MidiCAAudioProcessor : public juce::AudioProcessor {
 public:
  //==============================================================================
  MidiCAAudioProcessor();
  ~MidiCAAudioProcessor() override;

  //==============================================================================
  void prepareToPlay(double sampleRate, int samplesPerBlock) override;
  void releaseResources() override;

#ifndef JucePlugin_PreferredChannelConfigurations
  bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
#endif

  void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

  //==============================================================================
  juce::AudioProcessorEditor* createEditor() override;
  bool hasEditor() const override;

  //==============================================================================
  const juce::String getName() const override;

  bool acceptsMidi() const override;
  bool producesMidi() const override;
  bool isMidiEffect() const override;
  double getTailLengthSeconds() const override;

  //==============================================================================
  int getNumPrograms() override;
  int getCurrentProgram() override;
  void setCurrentProgram(int index) override;
  const juce::String getProgramName(int index) override;
  void changeProgramName(int index, const juce::String& newName) override;

  //==============================================================================
  void getStateInformation(juce::MemoryBlock& destData) override;
  void setStateInformation(const void* data, int sizeInBytes) override;

  float getLastBPM(); 
  int getDivision();
  void setDivision(int index);
  void outputDisplayData(DisplayData& displayData);
  bool checkAndClearChangedFlag();
  void forceNoteOff();

  ElementaryCA* getCA();
  CAMIDIGenerator* getGenerator();
 private:
  //==============================================================================
  juce::AudioParameterChoice* division;
  juce::AudioPlayHead::CurrentPositionInfo currentPositionInfo;
  ElementaryCA ca;
  CAMIDIGenerator generator;
  float sampleRate;
  int baseNote;
  int lastNoteValue;
  bool running;
  int sequencePosition;
  float samplesAccumulator;
  juce::uint8 velocity;
  float lastBPM;
  bool changed = false;
  bool forcedNoteOff = false;
  //==============================================================================
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MidiCAAudioProcessor)
};
