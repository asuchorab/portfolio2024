/*
  ==============================================================================

    NotePattern.cpp
    Created: 2 Jan 2022 3:54:50pm
    Author:  Rutio

  ==============================================================================
*/

#include "NotePattern.h"

#include <cmath>

NotePattern::NotePattern(int transpose, std::vector<int> elements) noexcept
    : transpose(transpose), elements(std::move(elements)) {}

int NotePattern::getNote(int base, int pattern_position) const {
  int octave = std::floor(pattern_position / (float)elements.size());
  int element = pattern_position - octave * (int)elements.size();
  return base + octave * transpose + elements[element];
}
