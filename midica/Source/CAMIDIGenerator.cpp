/*
  ==============================================================================

    CAMIDIGenerator.cpp
    Created: 20 Dec 2021 9:21:09pm
    Author:  Rutio

  ==============================================================================
*/

#include "CAMIDIGenerator.h"

CAMIDIGenerator::CAMIDIGenerator() : buffer(BUFFER_WIDTH * BUFFER_WIDTH) {
  std::fill(notes_on.begin(), notes_on.end(), -1);
}

void CAMIDIGenerator::setCA(ElementaryCA* ptr) { ca = ptr; }

void CAMIDIGenerator::setEditor(MidiCAAudioProcessorEditor* ptr) {
  editor = ptr;
}

int CAMIDIGenerator::getPattern() { return pattern; }

void CAMIDIGenerator::setPattern(int new_pattern) { pattern = new_pattern; }

int CAMIDIGenerator::getPatternSpacing() { return pattern_spacing; }

void CAMIDIGenerator::setPatternSpacing(int new_pattern_spacing) {
  pattern_spacing = new_pattern_spacing;
}

int CAMIDIGenerator::getPatternOffset() { return pattern_offset; }

void CAMIDIGenerator::setPatternOffset(int new_pattern_offset) {
  pattern_offset = new_pattern_offset;
}

int CAMIDIGenerator::getWindowOffset() { return window_offset; }

void CAMIDIGenerator::setWindowOffset(int new_offset) {
  const juce::ScopedLock lock(mutex);
  window_offset = new_offset;
  window_begin = ((int)BUFFER_WIDTH - window_width) / 2;
  window_mid = window_begin + window_offset;
  window_end = window_begin + window_width;
}

int CAMIDIGenerator::getWindowWidth() { return window_width; }

void CAMIDIGenerator::setWindowWidth(int new_width) {
  const juce::ScopedLock lock(mutex);
  window_width = new_width;
  window_begin = ((int)BUFFER_WIDTH - window_width) / 2;
  window_mid = window_begin + window_offset;
  window_end = window_begin + window_width;
}

bool CAMIDIGenerator::getDoRepeatNotes() { return do_repeat_notes; }

void CAMIDIGenerator::setDoRepeatNotes(bool new_do_repeat_notes) {
  const juce::ScopedLock lock(mutex);
  do_repeat_notes = new_do_repeat_notes;
}

void CAMIDIGenerator::startNote(juce::MidiBuffer& midi, int sample_position,
                                int value, int velocity) {
  jassert(ca);
  ca->reset();
  note_value = value;
  note_velocity = velocity;
  buffer_start_row = 0;
  buffer_occupied_rows = 0;
  startGeneratedNotes(midi, sample_position);
}

void CAMIDIGenerator::step(juce::MidiBuffer& midi, int sample_position) {
  jassert(ca);
  // endGeneratedNotes(midi, sample_position);
  ca->step();
  startGeneratedNotes(midi, sample_position);
}

void CAMIDIGenerator::endNote(juce::MidiBuffer& midi, int sample_position) {
  endGeneratedNotes(midi, sample_position);
}

void CAMIDIGenerator::paint(juce::Graphics& g) {
  const int SPACING = 20;
  const int PADDING = 2;
  const int TILE_WIDTH = 16;
  const int X_BASE = 200;
  const int HEIGHT = 400;
  int y_offset = ((int)BUFFER_WIDTH - 1) * SPACING + PADDING;
  int row_index = buffer_start_row;
  const auto ACTIVE_OUT = juce::Colour(191, 191, 191);
  const auto ACTIVE_IN = juce::Colour(255, 255, 255);
  const auto BACKGROUND_OUT = juce::Colour(39, 39, 47);
  const auto BACKGROUND_IN = juce::Colour(59, 59, 69);
  const auto BACKGROUND_CENTER = juce::Colour(77, 77, 89);
  if (window_begin > 0) {
    g.setColour(BACKGROUND_OUT);
    g.fillRect(X_BASE, 0, window_begin * SPACING, HEIGHT);
  }
  g.setColour(BACKGROUND_IN);
  g.fillRect(X_BASE + window_begin * SPACING, 0,
             (window_end - window_begin) * SPACING, HEIGHT);
  if (window_end < (int)BUFFER_WIDTH) {
    g.setColour(BACKGROUND_OUT);
    g.fillRect(X_BASE + window_end * SPACING, 0,
               ((int)BUFFER_WIDTH - window_end) * SPACING, HEIGHT);
  }
  g.setColour(BACKGROUND_CENTER);
  g.fillRect(X_BASE + window_mid * SPACING, 0, SPACING, HEIGHT);
  for (int j = 0; j < buffer_occupied_rows; ++j) {
    int row_offset = row_index * BUFFER_WIDTH;
    row_index++;
    if (row_index >= (int)BUFFER_WIDTH) {
      row_index = 0;
    }
    for (int i = 0; i < (int)BUFFER_WIDTH; ++i) {
      int8_t value = buffer[row_offset + i];
      if (value > 0) {
        g.setColour(value == 2 ? ACTIVE_IN : ACTIVE_OUT);
        int x = X_BASE + i * SPACING + PADDING;
        int y = y_offset;
        g.fillRect(x, y, TILE_WIDTH, TILE_WIDTH);
      }
    }
    y_offset -= SPACING;
  }
}

void CAMIDIGenerator::startGeneratedNotes(juce::MidiBuffer& midi,
                                          int sample_position) {
  const juce::ScopedLock lock(mutex);
  std::array<int8_t, BUFFER_WIDTH> ignore_note_off;
  std::fill(ignore_note_off.begin(), ignore_note_off.end(), 0);
  std::array<int8_t, BUFFER_WIDTH> notes_on_copy;
  std::copy(notes_on.begin(), notes_on.end(), notes_on_copy.begin());
  if (do_repeat_notes) {
    endGeneratedNotes(midi, sample_position);
  }
  buffer_start_row =
      buffer_start_row == 0 ? (int)BUFFER_WIDTH - 1 : buffer_start_row - 1;
  buffer_occupied_rows = std::min((int)BUFFER_WIDTH, buffer_occupied_rows + 1);
  auto& ca_state = ca->getState();
  int ca_mid = ca->getMiddleIndex();
  int b_ca = ca_mid - window_mid;
  int row_offset = buffer_start_row * BUFFER_WIDTH;
  int note_count = 0;
  for (int i = 0; i < window_begin; ++i) {
    buffer[row_offset + i] = ca_state[b_ca + i];
  }
  const NotePattern& pattern_object = NOTE_PATTERNS_OBJECTS[pattern];
  for (int i = window_begin; i < window_end; ++i) {
    uint8_t active = ca_state[b_ca + i];
    buffer[row_offset + i] = active * 2;
    if (active) {
      int value = pattern_object.getNote(note_value, (i - window_begin + pattern_offset) * pattern_spacing);
      if (value >= 0 && value <= 127) {
        notes_on[note_count] = value;
        note_count++;
        bool found = false;
        if (!do_repeat_notes) {
          for (int j = 0; j < (int)BUFFER_WIDTH; ++j) {
            int8_t check_value = notes_on_copy[j];
            if (check_value < 0) {
              break;
            }
            if (value == check_value) {
              found = true;
              ignore_note_off[j] = true;
              break;
            }
          }
        }
        if (!found) {
          midi.addEvent(
              juce::MidiMessage::noteOn(1, value, (uint8_t)note_velocity),
              sample_position);
        }
      }
    }
  }
  if (!do_repeat_notes) {
    for (int i = 0; i < (int)BUFFER_WIDTH; ++i) {
      int8_t value = notes_on_copy[i];
      if (value < 0) {
        break;
      }
      if (!ignore_note_off[i]) {
        midi.addEvent(juce::MidiMessage::noteOff(1, value), sample_position);
      }
    }
  }
  if (note_count < (int)BUFFER_WIDTH) {
    notes_on[note_count] = -1;
  }
  for (int i = window_end; i < (int)BUFFER_WIDTH; ++i) {
    buffer[row_offset + i] = ca_state[b_ca + i];
  }
  sendChangeMessage();
}

void CAMIDIGenerator::endGeneratedNotes(juce::MidiBuffer& midi,
                                        int sample_position) {
  for (int i = 0; i < (int)BUFFER_WIDTH; ++i) {
    int8_t value = notes_on[i];
    if (value < 0) {
      break;
    }
    midi.addEvent(juce::MidiMessage::noteOff(1, value), sample_position);
  }
  notes_on[0] = -1;
  sendChangeMessage();
}
