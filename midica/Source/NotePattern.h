/*
  ==============================================================================

    NotePattern.h
    Created: 2 Jan 2022 3:54:50pm
    Author:  Rutio

  ==============================================================================
*/

#pragma once

#include <vector>

class NotePattern {
 public:
  NotePattern(int transpose, std::vector<int> elements) noexcept;
  int getNote(int base, int pattern_position) const;

 private:
  int transpose;
  std::vector<int> elements;
};