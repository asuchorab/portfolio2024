/*
  ==============================================================================

    Globals.h
    Created: 19 Dec 2021 3:41:35pm
    Author:  Rutio

  ==============================================================================
*/

#include "NotePattern.h"

const int DIV_OPTIONS_NUM = 12;
const int DIV_OPTIONS_INITIAL = 7;
const float DIV_OPTIONS_VALUES[DIV_OPTIONS_NUM] = {
    8.f / 1.f, 4.f / 1.f, 3.f / 1.f, 2.f / 1.f, 1.f / 1.f,  1.f / 2.f,
    1.f / 3.f, 1.f / 4.f, 1.f / 6.f, 1.f / 8.f, 1.f / 12.f, 1.f / 16.f};
const char* const DIV_OPTIONS_STRINGS[DIV_OPTIONS_NUM] = {
    "8/1", "4/1", "3/1", "2/1", "1/1",  "1/2",
    "1/3", "1/4", "1/6", "1/8", "1/12", "1/16"};

const int RULE_OPTIONS_INITIAL = 30;
const int WINDOW_WIDTH_INITIAL = 4;

enum NOTE_PATTERNS {
  OCTAVES,
  CHROMATIC_SCALE,
  WHOLE_TONE_SCALE,
  MAJOR_SCALE,
  MINOR_SCALE,
  MINOR_MELODIC_SCALE,
  MINOR_HARMONIC_SCALE,
  PHRYGIAN_MODE,
  DORIAN_MODE,
  LYDIAN_MODE,
  MIXOLYDIAN_MODE,
  ACOUSTIC_SCALE,
  PENTATONIC_MAJOR_SCALE,
  PENTATONIC_MINOR_SCALE,
  JAPANESE_MODE,
  MAJOR_CHORD,
  MINOR_CHORD,
  DIMINISHED_CHORD,
  AUGMENTED_CHORD,
  MAJOR_7_CHORD,
  DOMINANT_7_CHORD,
  MINOR_7_CHORD,
  MINOR_MAJOR_7_CHORD,
  DIMINISHED_7_CHORD,
  HALF_DIMINISHED_7_CHORD,
  DIMINISHED_MAJOR_7_CHORD,
  AUGMENTED_MAJOR_7_CHORD,
  MAJOR_6_CHORD,
  MAJOR_MINOR_6_CHORD,
  MINOR_6_CHORD,
  MINOR_MAJOR_6_CHORD,
  NOTE_PATTERNS_NUM
};

const char* const NOTE_PATTERNS_STRINGS[NOTE_PATTERNS_NUM] = {
    "Octaves",
    "Chromatic scale",
    "Whole tone scale",
    "Major scale",
    "Minor scale",
    "Minor melodic scale",
    "Minor harmonic scale",
    "Dorian mode",
    "Phrygian mode",
    "Lydian mode",
    "Mixolydian mode",
    "Acoustic scale",
    "Pentatonic major scale",
    "Pentatonic minor scale",
    "Japanese mode",
    "Major chord",
    "Minor chord",
    "Diminished chord",
    "Augmented chord",
    "Major 7 chord",
    "Dominant 7 chord",
    "Minor 7 chord",
    "Minor major 7 chord",
    "Diminished 7 chord",
    "Half-diminished 7 chord",
    "Diminished major 7 chord",
    "Augmented major 7 chord",
    "Major 6 chord",
    "Major minor 6 chord",
    "Minor 6 chord",
    "Minor major 6 chord"};

const NotePattern NOTE_PATTERNS_OBJECTS[NOTE_PATTERNS_NUM] = {
    // "Octaves",
    NotePattern(12, {0}),
    // "Chromatic scale",
    NotePattern(1, {0}),
    //"Whole tone scale",
    NotePattern(2, {0}),
    //"Major scale",
    NotePattern(12, {0, 2, 4, 5, 7, 9, 11}),
    //"Minor scale",
    NotePattern(12, {0, 2, 3, 5, 7, 8, 10}),
    //"Minor melodic scale",
    NotePattern(12, {0, 2, 3, 5, 7, 9, 11}),
    //"Minor harmonic scale",
    NotePattern(12, {0, 2, 3, 5, 7, 8, 11}),
    //"Dorian mode",
    NotePattern(12, {0, 2, 3, 5, 7, 9, 10}),
    //"Phrygian mode",
    NotePattern(12, {0, 1, 3, 5, 7, 8, 10}),
    //"Lydian mode",
    NotePattern(12, {0, 2, 4, 6, 7, 9, 11}),
    //"Mixolydian mode",
    NotePattern(12, {0, 2, 4, 5, 7, 9, 10}),
    //"Acoustic scale",
    NotePattern(12, {0, 2, 4, 6, 7, 9, 10}),
    //"Pentatonic major scale",
    NotePattern(12, {0, 2, 4, 7, 9}),
    //"Pentatonic minor scale",
    NotePattern(12, {0, 3, 5, 7, 10}),
    //"Japanese mode",
    NotePattern(12, {0, 1, 5, 7, 8}),
    //"Major chord",
    NotePattern(12, {0, 4, 7}),
    //"Minor chord",
    NotePattern(12, {0, 3, 7}),
    //"Diminished chord",
    NotePattern(12, {0, 3, 6}),
    //"Augmented chord",
    NotePattern(4, {0}),
    //"Major 7 chord",
    NotePattern(12, {0, 4, 7, 11}),
    //"Dominant 7 chord",
    NotePattern(12, {0, 4, 7, 10}),
    //"Minor 7 chord",
    NotePattern(12, {0, 3, 7, 10}),
    //"Minor major 7 chord",
    NotePattern(12, {0, 3, 7, 11}),
    //"Diminished 7 chord",
    NotePattern(3, {0}),
    //"Half-diminished 7 chord",
    NotePattern(12, {0, 3, 6, 10}),
    //"Diminished major 7 chord",
    NotePattern(12, {0, 3, 6, 11}),
    //"Augmented major 7 chord",
    NotePattern(12, {0, 4, 8, 11}),
    //"Major 6 chord",
    NotePattern(12, {0, 4, 7, 9}),
    //"Major minor 6 chord",
    NotePattern(12, {0, 4, 7, 8}),
    //"Minor 6 chord",
    NotePattern(12, {0, 3, 7, 8}),
    //"Minor major 6 chord"
    NotePattern(12, {0, 3, 7, 9}),
};

#pragma once
