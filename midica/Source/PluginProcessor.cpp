/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"

#include "Globals.h"
#include "PluginEditor.h"

//==============================================================================
MidiCAAudioProcessor::MidiCAAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
    : AudioProcessor(
          BusesProperties()
#if !JucePlugin_IsMidiEffect
#if !JucePlugin_IsSynth
              .withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
              .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
      )
#endif
{
  juce::StringArray rate_options;
  for (int i = 0; i < DIV_OPTIONS_NUM; ++i) {
    rate_options.add(DIV_OPTIONS_STRINGS[i]);
  }

  addParameter(division = new juce::AudioParameterChoice(
                   "rate", "rate", rate_options, DIV_OPTIONS_INITIAL));
}

MidiCAAudioProcessor::~MidiCAAudioProcessor() {}

//==============================================================================
const juce::String MidiCAAudioProcessor::getName() const {
  return JucePlugin_Name;
}

bool MidiCAAudioProcessor::acceptsMidi() const {
#if JucePlugin_WantsMidiInput
  return true;
#else
  return false;
#endif
}

bool MidiCAAudioProcessor::producesMidi() const {
#if JucePlugin_ProducesMidiOutput
  return true;
#else
  return false;
#endif
}

bool MidiCAAudioProcessor::isMidiEffect() const {
#if JucePlugin_IsMidiEffect
  return true;
#else
  return false;
#endif
}

double MidiCAAudioProcessor::getTailLengthSeconds() const { return 0.0; }

int MidiCAAudioProcessor::getNumPrograms() {
  return 1;  // NB: some hosts don't cope very well if you tell them there are 0
             // programs, so this should be at least 1, even if you're not
             // really implementing programs.
}

int MidiCAAudioProcessor::getCurrentProgram() { return 0; }

void MidiCAAudioProcessor::setCurrentProgram(int index) {}

const juce::String MidiCAAudioProcessor::getProgramName(int index) {
  return {};
}

void MidiCAAudioProcessor::changeProgramName(int index,
                                             const juce::String& newName) {}

//==============================================================================
void MidiCAAudioProcessor::prepareToPlay(double sampleRate,
                                         int samplesPerBlock) {
  // Use this method as the place to do any pre-playback
  // initialisation that you need..
  baseNote = 0;
  lastNoteValue = 0;
  sequencePosition = 0;
  samplesAccumulator = 0;
  running = false;
  lastBPM = 120.f;
  this->sampleRate = static_cast<float>(sampleRate);
  generator.setCA(&ca);
}

void MidiCAAudioProcessor::releaseResources() {
  // When playback stops, you can use this as an opportunity to free up any
  // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool MidiCAAudioProcessor::isBusesLayoutSupported(
    const BusesLayout& layouts) const {
#if JucePlugin_IsMidiEffect
  juce::ignoreUnused(layouts);
  return true;
#else
  // This is the place where you check if the layout is supported.
  // In this template code we only support mono or stereo.
  // Some plugin hosts, such as certain GarageBand versions, will only
  // load plugins that support stereo bus layouts.
  if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono() &&
      layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
    return false;

    // This checks if the input layout matches the output layout
#if !JucePlugin_IsSynth
  if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
    return false;
#endif

  return true;
#endif
}
#endif

void MidiCAAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer,
                                        juce::MidiBuffer& midiMessages) {
  juce::ScopedNoDenormals noDenormals;
  // jassert(buffer.getNumChannels() == 0);
  float bpm = 120.f;
  if (auto playHead = this->getPlayHead()) {
    playHead->getCurrentPosition(currentPositionInfo);
    bpm = static_cast<float>(currentPositionInfo.bpm);
    lastBPM = bpm;
  }
  auto numSamples = static_cast<float>(buffer.getNumSamples());
  auto divisionFactor = DIV_OPTIONS_VALUES[division->getIndex()];
  auto noteDuration = divisionFactor * sampleRate * 60.f / bpm;
  float tempSamplePosition = 0;
  bool wasRunning = running;
  if (forcedNoteOff) {
    running = false;
  }
  bool doNoteOff = forcedNoteOff;
  forcedNoteOff = false;
  int noteOffTime = 0;
  for (const auto metadata : midiMessages) {
    const auto msg = metadata.getMessage();
    if (msg.isNoteOn()) {
      tempSamplePosition = static_cast<float>(msg.getTimeStamp());
      baseNote = msg.getNoteNumber();
      velocity = msg.getVelocity();
      samplesAccumulator = 0;
      sequencePosition = 0;
      running = true;
      if (!doNoteOff) {
        doNoteOff = true;
        noteOffTime = msg.getTimeStamp();
      }
    } else if (msg.isNoteOff()) {
      if (msg.getNoteNumber() == baseNote) {
        running = false;
        if (!doNoteOff) {
          doNoteOff = true;
          noteOffTime = msg.getTimeStamp();
        }
      }
    }
  }
  midiMessages.clear();
  if (doNoteOff && wasRunning) {
    generator.endNote(midiMessages, noteOffTime);
  }
  if (running) {
    tempSamplePosition += samplesAccumulator;
    while (tempSamplePosition < numSamples) {
      if (sequencePosition > 0) {
        generator.step(midiMessages, tempSamplePosition);
      } else {
        generator.startNote(midiMessages, tempSamplePosition, baseNote,
                            velocity);
      }
      sequencePosition++;
      tempSamplePosition += noteDuration;
    }
    samplesAccumulator = tempSamplePosition - numSamples;
  }

  // auto totalNumInputChannels = getTotalNumInputChannels();
  // auto totalNumOutputChannels = getTotalNumOutputChannels();

  // In case we have more outputs than inputs, this code clears any output
  // channels that didn't contain input data, (because these aren't
  // guaranteed to be empty - they may contain garbage).
  // This is here to avoid people getting screaming feedback
  // when they first compile a plugin, but obviously you don't need to keep
  // this code if your algorithm always overwrites all the output channels.
  // for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
  //  buffer.clear(i, 0, buffer.getNumSamples());

  // This is the place where you'd normally do the guts of your plugin's
  // audio processing...
  // Make sure to reset the state if your inner loop is processing
  // the samples and the outer loop is handling the channels.
  // Alternatively, you can process the samples with the channels
  // interleaved by keeping the same state.
  // for (int channel = 0; channel < totalNumInputChannels; ++channel) {
  //  auto* channelData = buffer.getWritePointer(channel);
  // ..do something to the data...
  //}
}

//==============================================================================
bool MidiCAAudioProcessor::hasEditor() const {
  return true;  // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* MidiCAAudioProcessor::createEditor() {
  return new MidiCAAudioProcessorEditor(*this);
}

//==============================================================================
void MidiCAAudioProcessor::getStateInformation(juce::MemoryBlock& destData) {
  // You should use this method to store your parameters in the memory block.
  // You could do that either as raw data, or use the XML or ValueTree classes
  // as intermediaries to make it easy to save and load complex data.
  juce::ValueTree state("state");
  state.setProperty("division", getDivision(), nullptr);
  DBG((int)getCA()->getRule());
  state.setProperty("neighbourhood", (int)getCA()->getNeighbourhood(), nullptr);
  state.setProperty("rule", (int)getCA()->getRule(), nullptr);
  state.setProperty("windowWidth", getGenerator()->getWindowWidth(), nullptr);
  state.setProperty("windowOffset", getGenerator()->getWindowOffset(), nullptr);
  state.setProperty("pattern", getGenerator()->getPattern(), nullptr);
  state.setProperty("patternOffset", getGenerator()->getPatternOffset(), nullptr);
  state.setProperty("patternSpacing", getGenerator()->getPatternSpacing(),
                    nullptr);
  state.setProperty("repeatNotes",
                    getGenerator()->getDoRepeatNotes() ? 1 : 0, nullptr);
  state.setProperty("randomInitialisation",
                    getCA()->getDoRandomInitialisation() ? 1 : 0, nullptr);
  state.setProperty("initialisationSeed", (int)getCA()->getSeed(), nullptr);
  state.setProperty("bufferSpan", (int)getCA()->getBufferSpan(), nullptr);
  std::unique_ptr<juce::XmlElement> xml(state.createXml());
  copyXmlToBinary(*xml, destData);
}

void MidiCAAudioProcessor::setStateInformation(const void* data,
                                               int sizeInBytes) {
  // You should use this method to restore your parameters from this memory
  // block, whose contents will have been created by the getStateInformation()
  // call.
  std::unique_ptr<juce::XmlElement> xmlState(
      getXmlFromBinary(data, sizeInBytes));

  if (xmlState.get() != nullptr) {
    auto state = juce::ValueTree::fromXml(*xmlState);
    auto divisionValue = state.getProperty("division");
    if (divisionValue.isString()) {
      setDivision(divisionValue);
    }
    auto neighbourhoodValue = state.getProperty("neighbourhood");
    if (neighbourhoodValue.isString()) {
      getCA()->setNeighbourhood((int)neighbourhoodValue);
    }
    auto ruleValue = state.getProperty("rule");
    if (ruleValue.isString()) {
      getCA()->setRule((int)ruleValue);
    }
    auto windowWidthValue = state.getProperty("windowWidth");
    if (windowWidthValue.isString()) {
      getGenerator()->setWindowWidth(windowWidthValue);
    }
    auto windowOffsetValue = state.getProperty("windowOffset");
    if (windowOffsetValue.isString()) {
      getGenerator()->setWindowOffset(windowOffsetValue);
    }
    auto patternValue = state.getProperty("pattern");
    if (patternValue.isString()) {
      getGenerator()->setPattern(patternValue);
    }
    auto patternOffsetValue = state.getProperty("patternOffset");
    if (patternOffsetValue.isString()) {
      getGenerator()->setPatternOffset(patternOffsetValue);
    }
    auto patternSpacingValue = state.getProperty("patternSpacing");
    if (patternSpacingValue.isString()) {
      getGenerator()->setPatternSpacing(patternSpacingValue);
    }
    auto repeatNotesValue = state.getProperty("repeatNotes");
    if (repeatNotesValue.isString()) {
      getGenerator()->setDoRepeatNotes(repeatNotesValue);
    }
    auto randomInitialisationValue = state.getProperty("randomInitialisation");
    if (randomInitialisationValue.isString()) {
      getCA()->setRandomInitialisation(randomInitialisationValue);
    }
    auto initialisationSeedValue = state.getProperty("initialisationSeed");
    if (initialisationSeedValue.isString()) {
      getCA()->setSeed((int)initialisationSeedValue);
    }
    auto bufferSpanValue = state.getProperty("bufferSpan");
    if (bufferSpanValue.isString()) {
      getCA()->setBufferSpan((int)bufferSpanValue);
    }
  }
  changed = true;
}

float MidiCAAudioProcessor::getLastBPM() { return lastBPM; }

int MidiCAAudioProcessor::getDivision() { return (*division); }

void MidiCAAudioProcessor::setDivision(int index) { (*division) = index; }

void MidiCAAudioProcessor::outputDisplayData(DisplayData& displayData) {
  displayData.running = running;
  displayData.leftSquare = sequencePosition % 2;
}

bool MidiCAAudioProcessor::checkAndClearChangedFlag() {
  if (changed) {
    changed = false;
    return true;
  }
  return false;
}

void MidiCAAudioProcessor::forceNoteOff() { forcedNoteOff = true; }

ElementaryCA* MidiCAAudioProcessor::getCA() { return &ca; }

CAMIDIGenerator* MidiCAAudioProcessor::getGenerator() { return &generator; }

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
  return new MidiCAAudioProcessor();
}
