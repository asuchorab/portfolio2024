/*
  ==============================================================================

    CAMIDIGenerator.h
    Created: 20 Dec 2021 9:21:09pm
    Author:  Rutio

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

#include <array>
#include <vector>

#include "ElementaryCA.h"
#include "Globals.h"

class MidiCAAudioProcessorEditor;
class CAMIDIGenerator : public juce::ChangeBroadcaster {
 public:
  CAMIDIGenerator();
  void setCA(ElementaryCA* ptr);
  void setEditor(MidiCAAudioProcessorEditor* ptr);
  int getPattern();
  void setPattern(int new_pattern);
  int getPatternSpacing();
  void setPatternSpacing(int new_pattern_spacing);
  int getPatternOffset();
  void setPatternOffset(int new_pattern_offset);
  int getWindowOffset();
  void setWindowOffset(int new_offset);
  int getWindowWidth();
  void setWindowWidth(int new_width);
  bool getDoRepeatNotes();
  void setDoRepeatNotes(bool new_do_repeat_notes);
  void startNote(juce::MidiBuffer& midi, int sample_position, int value,
                 int velocity);
  void step(juce::MidiBuffer& midi, int sample_position);
  void endNote(juce::MidiBuffer& midi, int sample_position);
  void paint(juce::Graphics& g);

 private:
  void startGeneratedNotes(juce::MidiBuffer& midi, int sample_position);
  void endGeneratedNotes(juce::MidiBuffer& midi, int sample_position);

  constexpr static size_t BUFFER_WIDTH = 20;
  ElementaryCA* ca = nullptr;
  MidiCAAudioProcessorEditor* editor = nullptr;
  //const NotePattern* pattern = &NOTE_PATTERNS_OBJECTS[0];
  int pattern = 0;
  int pattern_spacing = 1;
  int pattern_offset = 0;
  std::vector<int8_t> buffer;
  std::array<int8_t, BUFFER_WIDTH> notes_on;
  int buffer_start_row = 0;
  int buffer_occupied_rows  = 0;
  int window_offset = 0;
  int window_width = 1;
  int window_begin = 0;
  int window_mid = 0;
  int window_end = 0;
  int note_value = -1;
  int note_velocity = 0;
  bool do_repeat_notes = true;
  juce::CriticalSection mutex;
};