#include "plugin.hpp"
#include "NestlingAudio.hpp"
#include <cmath>


void Harmonizer::init(std::vector<std::vector<int>> &chordIntervals) {
  this->chordIntervals = chordIntervals;
}

void BasicHarmonizer::_setPassingNoteOffsets(size_t chordTypeIndex, int melody, int root, int* offsets) {
    // note not in chord --> use fully diminished chord as harmony
    offsets[0] = -3;
    offsets[1] = -6;
    offsets[2] = -9;
}

void BasicHarmonizer::computeOffsets(size_t chordTypeIndex, int melody, int root, int* offsets) {
  // todo: what's the right way to do this that avoids too much copying?
  std::vector<int> *currentInts = &(chordIntervals[chordTypeIndex]);
  int relativeMelNote = (melody - root) % 12;
  const size_t numInts = currentInts->size();
  if (relativeMelNote == 0) {
    // harmony tones are top three non-root notes of chord
    // TODO: fix bug where this won't include root if it should (if there are more voices than chord notes)
    offsets[0] = (*currentInts)[(numInts-1) % numInts] - 12;
    offsets[1] = (*currentInts)[(numInts-2) % numInts] - 12;
    offsets[2] = (*currentInts)[(numInts-3) % numInts] - 12;
  } else {
    // find index of relativeMelNote in currentInts
    int idx = -1;
    for (size_t m = 0; m < numInts; m++) {
      if ((*currentInts)[m] == relativeMelNote) {
        idx = m;
        break;
      }
    }
    if (idx == -1) {
      this->_setPassingNoteOffsets(chordTypeIndex, melody, root, offsets);
    } else {
      size_t poSize = numInts + 1;
      int possibleOffsets[poSize];
      possibleOffsets[0] = 0 - relativeMelNote;
      for (size_t i = 0; i < numInts; i++) {
        possibleOffsets[i+1] = (*currentInts)[i] - relativeMelNote;
      }
      int j = idx;
      offsets[0] = (possibleOffsets[j] % 12);
      if (offsets[0] >= 0) offsets[0] -= 12;
      j--;
      j = (j + poSize) % poSize;
      offsets[1] = (possibleOffsets[j] % 12);
      if (offsets[1] >= 0) offsets[1] -= 12;
      j--;
      j = (j + poSize) % poSize;
      offsets[2] = (possibleOffsets[j] % 12);
      if (offsets[2] >= 0) offsets[2] -= 12;
    }
  }
}
std::string BasicHarmonizer::displayName() {
  return "Diminished Passing";
}

void CloseVoicer::adjustOffsets(int* offsets) {
  // do nothing because the harmonizers already output close voicing
}
std::string CloseVoicer::displayName() {
  return "Close";
}

void DropTwoVoicer::adjustOffsets(int* offsets) {
  int voiceTwo = offsets[0];
  offsets[0] = offsets[1];
  offsets[1] = offsets[2];
  offsets[2] = voiceTwo - 12;
}
std::string DropTwoVoicer::displayName() {
  return "Drop 2";
}

void DropTwoFourVoicer::adjustOffsets(int* offsets) {
  int voiceTwo = offsets[0];
  int voiceFour = offsets[2];
  offsets[0] = offsets[1];
  offsets[1] = voiceTwo - 12;
  offsets[2] = voiceFour - 12;
}
std::string DropTwoFourVoicer::displayName() {
  return "Drop 2+4";
}


NestlingAudio::NestlingAudio() {
}

void NestlingAudio::loadChords() {
  INFO("Loading chords.");
  FILE* file = std::fopen(asset::plugin(pluginInstance, "res/chords.json").c_str(), "r");
  if (!file)
    return;
  DEFER({std::fclose(file);});

  json_error_t error;
  json_t* chordJ = json_loadf(file, 0, &error);
  if (!chordJ)
    throw Exception("Chords file has invalid JSON at %d:%d %s", error.line, error.column, error.text);

  /*
  char* js = json_dumps(chordJ, 0);
  INFO("%s\n", js);
  DEFER({free(js);});
  */

  assert(chordNames.size() == 0);
  assert(chordIntervals.size() == 0);
  size_t index;
  json_t* value;
  json_array_foreach(chordJ, index, value) {
    json_t* jname = json_object_get(value, "name");
    if (jname != NULL) {
      const char* str = json_string_value(jname);
      chordNames.push_back(str);
    } else {
      chordNames.push_back("error");
    }
    /*
    char* chord = json_dumps(currentChordJ, 0);
    INFO("Chord: %s\n", chord);
    DEFER({free(chord);});
    */
    json_t* intervalsJson = json_object_get(value, "intervals");
    if (intervalsJson != NULL) {
      /*
      char* intervals = json_dumps(intervalsJson, 0);
      INFO("Intervals: %s\n", intervals);
      */
      const size_t numInts = json_array_size(intervalsJson);
      std::vector<int> currentInts;
      for (size_t i = 0; i < numInts; i++) {
        json_t* intJ = json_array_get(intervalsJson, i);
        int intv = 0;
        if (intJ != NULL) {
          json_unpack(intJ, "i", &intv);
        }
        currentInts.push_back(intv);
      }
      chordIntervals.push_back(currentInts);
    }
  }
}


int NestlingAudio::cvToMidi(const float voltage) {
  return std::round(voltage * 12 + 60);
}

float NestlingAudio::midiToCV(const int midi) {
  return (midi - 60) / 12.0f;
}

std::string NestlingAudio::midiToString(const int midi) {
  std::string notes[] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
  return notes[midi % 12];
}

size_t NestlingAudio::cvToIndex(const float voltage, const size_t length) {
  size_t index = 0;
  // TODO: consider making this a switch in the UI instead of a hardcoded constant
  if (VOLTAGE_UNIPOLAR) {
    index = (size_t) (voltage / 10.0f * length);
  } else {
    index = (size_t) ((voltage + 5.0f) / 10.0f * length);
  }
  if (index < 0) return 0;
  if (index >= length) return length - 1;
  return index;
}

void NestlingAudio::process(const ProcessArgs& args) {
}
