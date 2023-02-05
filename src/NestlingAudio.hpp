#include "plugin.hpp"
#include <jansson.h>


#define VOLTAGE_UNIPOLAR true


// interface for the harmonizer strategy - module may switch between strategies
struct Harmonizer {  // abstract
  std::vector<std::vector<int>> chordIntervals;

  virtual void init(std::vector<std::vector<int>> &chordIntervals);
  virtual void computeOffsets(size_t chordTypeIndex, int melody, int root, int* offsets) = 0;
  virtual void _setPassingNoteOffsets(size_t chordTypeIndex, int melody, int root, int* offsets) = 0;
  virtual std::string displayName() = 0;
};

struct BasicHarmonizer : Harmonizer {
  virtual void computeOffsets(size_t chordTypeIndex, int melody, int root, int* offsets) override;
  virtual void _setPassingNoteOffsets(size_t chordTypeIndex, int melody, int root, int* offsets) override;
  virtual std::string displayName() override;
};


// interface for the voicer strategy - module may switch between strategies
struct Voicer {  // abstract
  virtual void adjustOffsets(int* offsets) = 0;
  virtual std::string displayName() = 0;
};

struct CloseVoicer : Voicer {
  void adjustOffsets(int* offsets) override;
  virtual std::string displayName() override;
};


struct NestlingAudio : Module {

  std::vector<std::string> chordNames;
  std::vector<std::vector<int>> chordIntervals;

  // initialization
	NestlingAudio();
  void loadChords();

  // processing
	virtual void process(const ProcessArgs& args) override;

  // utility
  int cvToMidi(const float voltage);
  float midiToCV(const int midi);
  size_t cvToIndex(const float voltage, const size_t length);
};


