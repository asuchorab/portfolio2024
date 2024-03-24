/*
  ==============================================================================

    ElementaryCA.cpp
    Created: 20 Dec 2021 5:29:46pm
    Author:  Rutio

  ==============================================================================
*/

#include "ElementaryCA.h"

#include <string>

uint32_t ElementaryCA::getBufferSpan() { return buffer_span; }

void ElementaryCA::setBufferSpan(uint32_t new_span) {
  const juce::ScopedLock lock(mutex);
  buffer_span = new_span;
  updateStateSize();
}

uint32_t ElementaryCA::getRule() { return rule; }

uint32_t ElementaryCA::getNeighbourhood() { return neighbourhood; }

void ElementaryCA::setRule(uint32_t new_rule) { rule = new_rule; }

void ElementaryCA::setNeighbourhood(uint32_t new_neighbourhood) {
  const juce::ScopedLock lock(mutex);
  if (new_neighbourhood != neighbourhood) {
    neighbourhood = new_neighbourhood;
    updateStateSize();
  }
}

bool ElementaryCA::getDoRandomInitialisation() {
  return do_random_initialisation;
}

void ElementaryCA::setRandomInitialisation(bool new_do_random_intialisation) {
  const juce::ScopedLock lock(mutex);
  do_random_initialisation = new_do_random_intialisation;
}

uint32_t ElementaryCA::getSeed() { return seed; }

void ElementaryCA::setSeed(uint32_t new_seed) {
  const juce::ScopedLock lock(mutex);
  seed = new_seed;
}

void ElementaryCA::reset() {
  const juce::ScopedLock lock(mutex);
  std::fill(state.begin(), state.end(), 0);
  if (do_random_initialisation) {
    random.seed(seed);
    for (int end_index = state.size() - 1, i = 1; i < end_index; ++i) {
      state[i] = random() & 1;
    }
  } else {
    state[getMiddleIndex()] = 1;
  }
}

void ElementaryCA::step() {
  state.swap(state_old);
  uint32_t offset = neighbourhood / 2;
  // std::string dbg;
  for (uint32_t last = state.size() - offset, i = offset; i < last; ++i) {
    uint32_t inner_first = i - offset;
    uint32_t inner_last = inner_first + neighbourhood;
    uint32_t rule_bit = 0;
    for (uint32_t j = inner_first; j < inner_last; ++j) {
      rule_bit <<= 1;
      rule_bit |= state_old[j];
    }
    // dbg += std::to_string((int) rule_bit);
    state[i] = (rule >> rule_bit) & 1;
    // dbg += state[i] ? 'X' : '-';
  }
  // DBG(dbg);
}

const std::vector<uint8_t>& ElementaryCA::getState() const { return state; }

uint32_t ElementaryCA::getMiddleIndex() const {
  return buffer_span + neighbourhood / 2;
}

void ElementaryCA::updateStateSize() {
  int size = 2 * buffer_span + neighbourhood;
  std::vector<uint8_t> new_state(size);
  state.swap(new_state);
  std::vector<uint8_t> new_state_old(size);
  state_old.swap(new_state_old);
}
