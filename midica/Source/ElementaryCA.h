/*
  ==============================================================================

    ElementaryCA.h
    Created: 20 Dec 2021 5:29:46pm
    Author:  Rutio

  ==============================================================================
*/

#pragma once

#include <vector>
#include <random>
#include <JuceHeader.h>

class ElementaryCA {
 public:
  uint32_t getBufferSpan();
  void setBufferSpan(uint32_t new_span);
  uint32_t getRule();
  void setRule(uint32_t new_rule);
  uint32_t getNeighbourhood();
  void setNeighbourhood(uint32_t new_neighbourhood);
  bool getDoRandomInitialisation();
  void setRandomInitialisation(bool new_do_random_intialisation);
  uint32_t getSeed();
  void setSeed(uint32_t new_seed);
  void reset();
  void step();
  const std::vector<uint8_t>& getState() const;
  uint32_t getMiddleIndex() const;

 private:
  void updateStateSize();
  std::vector<uint8_t> state;
  std::vector<uint8_t> state_old;
  uint32_t buffer_span = 50;
  uint32_t neighbourhood = 3;
  uint32_t rule = 30;
  uint32_t seed = 0;
  std::mt19937 random;
  bool do_random_initialisation = false;
  juce::CriticalSection mutex;
};