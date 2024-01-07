#include "plugin.hpp"
#include "NestlingAudio.hpp"


struct LabelQuantity : ParamQuantity {
  std::vector<std::string> labels;
  void setLabels(std::vector<std::string> labels) {
    this->labels = labels;
  }
	std::string getDisplayValueString() override {
		size_t val = (size_t) getValue();
    if (val >= 0 && val < labels.size()) {
			return labels[val];
		} else {
			return "error";
		}
	}
};

struct ParallelHarmonizer : BasicHarmonizer {
  int* lastOffsets;
  virtual void init(std::vector<std::vector<int>> &chordIntervals) override {
    BasicHarmonizer::init(chordIntervals);
    lastOffsets = new int[3];
    // if the first note after initialization is a passing tone, we'll use a diminished chord like BasicHarmonizer
    lastOffsets[0] = -3;
    lastOffsets[1] = -6;
    lastOffsets[2] = -9;
  }
  virtual void _setPassingNoteOffsets(size_t chordTypeIndex, int melody, int root, int* offsets) override {
    offsets[0] = lastOffsets[0];
    offsets[1] = lastOffsets[1];
    offsets[2] = lastOffsets[2];
  }
  virtual void computeOffsets(size_t chordTypeIndex, int melody, int root, int* offsets) override {
    BasicHarmonizer::computeOffsets(chordTypeIndex, melody, root, offsets);
    lastOffsets[0] = offsets[0];
    lastOffsets[1] = offsets[1];
    lastOffsets[2] = offsets[2];
  }
  virtual std::string displayName() override {
    return "Parallel Passing";
  }
};

struct RandomPassingHarmonizer : BasicHarmonizer {
  int lastNote = 0;
  int* lastOffsets;

  virtual void init(std::vector<std::vector<int>> &chordIntervals) override {
    BasicHarmonizer::init(chordIntervals);
    lastOffsets = new int[3];
  }
 
  virtual void computeOffsets(size_t chordTypeIndex, int melody, int root, int* offsets) override {
    BasicHarmonizer::computeOffsets(chordTypeIndex, melody, root, offsets);
    lastNote = melody;
  }
  virtual void _setPassingNoteOffsets(size_t chordTypeIndex, int melody, int root, int* offsets) override {
    if (melody == lastNote) {
      offsets[0] = lastOffsets[0];
      offsets[1] = lastOffsets[1];
      offsets[2] = lastOffsets[2];
      return;
    }
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> third_distr(2, 5);  // M2 through P4
    int third = third_distr(gen);

    std::uniform_int_distribution<> fifth_distr(third == 5 ? 7 : 6, 8);  // D5, P5 or A5
    int fifth = fifth_distr(gen);

    std::uniform_int_distribution<> seventh_distr(fifth == 8 ? 10 : 9, 11);  // M6 thru M7
    int seventh = seventh_distr(gen);

    int possibleOffsets[4] = {0, third, fifth, seventh};

    INFO("possibleOffsets initial: %d, %d, %d, %d\n", 0, third, fifth, seventh);
    std::uniform_int_distribution<> idx_distr(0, 3);
    int melIndex = idx_distr(gen);
    int delta = possibleOffsets[melIndex];
    possibleOffsets[0] -= delta;
    possibleOffsets[1] -= delta;
    possibleOffsets[2] -= delta;
    possibleOffsets[3] -= delta;
    INFO("possibleOffsets after delta: %d, %d, %d, %d\n", 
        possibleOffsets[0],
        possibleOffsets[1],
        possibleOffsets[2],
        possibleOffsets[3]);
    // TODO: there's some common-ish code between here and BasicHarmonizer::computeOffsets, write unit tests and refactor
    int j = melIndex - 1;
    j = (j + 4) % 4;
    INFO("0: j, j%%4, po[j%%4], po[j%%4]%%12: %d, %d, %d, %d \n", j, j%4, possibleOffsets[j%4], possibleOffsets[j % 4] % 12);
    offsets[0] = (possibleOffsets[j % 4] % 12);
    if (offsets[0] >= 0) offsets[0] -= 12;
    j--;
    j = (j + 4) % 4;
    INFO("1: j, j%%4, po[j%%4], po[j%%4]%%12: %d, %d, %d, %d \n", j, j%4, possibleOffsets[j%4], possibleOffsets[j % 4] % 12);
    offsets[1] = (possibleOffsets[j % 4] % 12);
    if (offsets[1] >= 0) offsets[1] -= 12;
    j--;
    j = (j + 4) % 4;
    INFO("2: j, j%%4, po[j%%4], po[j%%4]%%12: %d, %d, %d, %d \n", j, j%4, possibleOffsets[j%4], possibleOffsets[j % 4] % 12);
    offsets[2] = (possibleOffsets[j % 4] % 12);
    if (offsets[2] >= 0) offsets[2] -= 12;
    INFO("offsets: %d, %d, %d\n", offsets[0], offsets[1], offsets[2]);
    lastOffsets[0] = offsets[0];;
    lastOffsets[1] = offsets[1];
    lastOffsets[2] = offsets[2];
  }
  virtual std::string displayName() override {
    return "Random Passing";
  }
};

struct NestlingAudio_Jazz : NestlingAudio {
	enum ParamId {
		ROOT_PARAM,
		CHORD_PARAM,
		HARMONY_PARAM,
    VOICING_PARAM,
		PARAMS_LEN
	};
	enum InputId {
		ROOT_INPUT,
		CHORD_INPUT,
    HARMONY_INPUT,
    VOICING_INPUT,
		MEL_INPUT,
		GATE_INPUT,
		INPUTS_LEN
	};
	enum OutputId {
		OUT1_OUTPUT,
		OUT2_OUTPUT,
		OUT3_OUTPUT,
    ROOT_OUTPUT,
    TRIG_OUTPUT,
    MEL_8VA_OUTPUT,
		OUTPUTS_LEN
	};
	enum LightId {
		LIGHTS_LEN
	};

  std::vector<Harmonizer*> harmonizers;
  std::vector<Voicer*> voicers;
  int lastMelNote;
  bool lastGate;

	NestlingAudio_Jazz() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);

		std::vector<std::string> noteNames = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
		configParam<LabelQuantity>(ROOT_PARAM, 0.f, 11.f, 0.f, "root of chord");
    ((LabelQuantity*)getParamQuantity(ROOT_PARAM))->setLabels(noteNames);

		loadChords();
		configParam<LabelQuantity>(CHORD_PARAM, 0.f, (float) (chordNames.size() - 1), 1.f, "chord type");
    ((LabelQuantity*)getParamQuantity(CHORD_PARAM))->setLabels(chordNames);

		std::vector<std::string> harmonizerNames;
    Harmonizer* harm;
    harm = new BasicHarmonizer();
    harm->init(chordIntervals);
    harmonizers.push_back(harm);
    harmonizerNames.push_back(harm->displayName());
    harm = new ParallelHarmonizer();
    harm->init(chordIntervals);
    harmonizers.push_back(harm);
    harmonizerNames.push_back(harm->displayName());
    harm = new RandomPassingHarmonizer();
    harm->init(chordIntervals);
    harmonizers.push_back(harm);
    harmonizerNames.push_back(harm->displayName());
		configParam<LabelQuantity>(HARMONY_PARAM, 0.f, (float) (harmonizers.size() - 1), 1.f, "harmonizer type");
    ((LabelQuantity*)getParamQuantity(HARMONY_PARAM))->setLabels(harmonizerNames);

		std::vector<std::string> voicerNames;
    Voicer* voic;
    voic = new CloseVoicer();
    voicers.push_back(voic);
    voicerNames.push_back(voic->displayName());
    voic = new DropTwoVoicer();
    voicers.push_back(voic);
    voicerNames.push_back(voic->displayName());
    voic = new DropTwoFourVoicer();
    voicers.push_back(voic);
    voicerNames.push_back(voic->displayName());
    assert(voicers.size() == 3);
    assert(voicerNames.size() == 3);
		configParam<LabelQuantity>(VOICING_PARAM, 0.f, 2.f, 0.f, "voicing");
    ((LabelQuantity*)getParamQuantity(VOICING_PARAM))->setLabels(voicerNames);

		configInput(ROOT_INPUT, "chord root: 1v/oct");
		configInput(CHORD_INPUT, "chord type: 0v to 10v");  // TODO: change if we make this configurable
		configInput(HARMONY_INPUT, "harmonizer type: 0v to 10v");  // TODO: change if we make this configurable
		configInput(VOICING_INPUT, "voicing: 0v to 10v");  // TODO: change if we make this configurable
		configInput(MEL_INPUT, "melody note in: 1v/oct");
		configInput(GATE_INPUT, "gate in (affects tie behavior)");
		configOutput(OUT1_OUTPUT, "harmony note 1: 1v/oct");
		configOutput(OUT2_OUTPUT, "harmony note 2: 1v/oct");
		configOutput(OUT3_OUTPUT, "harmony note 3: 1v/oct");
		configOutput(ROOT_OUTPUT, "root sub: 1v/oct");
		configOutput(TRIG_OUTPUT, "trigger out when other outs change");
		configOutput(MEL_8VA_OUTPUT, "melody note 8va out: 1v/oct");
	}

	void process(const ProcessArgs& args) override {
    // INPUT
		size_t chordTypeIndex;
		if (inputs[CHORD_INPUT].isConnected()) {
    	chordTypeIndex = cvToIndex(inputs[CHORD_INPUT].getVoltage(), chordNames.size());
      getParamQuantity(CHORD_PARAM)->setValue(chordTypeIndex);
		} else {
			chordTypeIndex = (size_t) params[CHORD_PARAM].getValue();
		}

		int rootBase;
		if (inputs[ROOT_INPUT].isConnected()) {
			float rootVoltage = inputs[ROOT_INPUT].getVoltage();
			rootBase = cvToMidi(rootVoltage) % 12;
      getParamQuantity(ROOT_PARAM)->setValue(rootBase);
		} else {
			rootBase = ((int) params[ROOT_PARAM].getValue()) % 12;
		}

    size_t harmonizerIndex;
		if (inputs[HARMONY_INPUT].isConnected()) {
    	harmonizerIndex = cvToIndex(inputs[HARMONY_INPUT].getVoltage(), harmonizers.size());
      getParamQuantity(HARMONY_PARAM)->setValue(harmonizerIndex);
		} else {
			harmonizerIndex = (size_t) params[HARMONY_PARAM].getValue();
		}

    size_t voicerIndex;
		if (inputs[VOICING_INPUT].isConnected()) {
    	voicerIndex = cvToIndex(inputs[VOICING_INPUT].getVoltage(), voicers.size());
      getParamQuantity(VOICING_PARAM)->setValue(voicerIndex);
		} else {
			voicerIndex = (size_t) params[VOICING_PARAM].getValue();
		}

    float melVoltage = inputs[MEL_INPUT].getVoltage();
    int melNote = cvToMidi(melVoltage);
    float gateVoltage = inputs[GATE_INPUT].getVoltage();
    bool gate = (gateVoltage > 5.f);

    bool outsChanged = false;
    float oldVoltage1 = outputs[OUT1_OUTPUT].getVoltage();
    float oldVoltage2 = outputs[OUT2_OUTPUT].getVoltage();
    float oldVoltage3 = outputs[OUT3_OUTPUT].getVoltage();
    if (!inputs[GATE_INPUT].isConnected() || lastMelNote != melNote || (gate && !lastGate)) {
      // ALGORITHM
      int offsets[3];
      harmonizers[harmonizerIndex]->computeOffsets(chordTypeIndex, melNote, rootBase, offsets);
      voicers[voicerIndex]->adjustOffsets(offsets);

      // OUTPUT
      int out1Note = melNote + offsets[0];
      if (out1Note < 0) out1Note = melNote;
      int out2Note = melNote + offsets[1];
      if (out2Note < 0) out2Note = melNote;
      int out3Note = melNote + offsets[2];
      if (out3Note < 0) out3Note = melNote;
      assert(out1Note >= 0);
      assert(out2Note >= 0);
      assert(out3Note >= 0);

      float newVoltage1 = midiToCV(out1Note);
      float newVoltage2 = midiToCV(out2Note);
      float newVoltage3 = midiToCV(out3Note);
      outputs[OUT1_OUTPUT].setVoltage(newVoltage1);
      outputs[OUT2_OUTPUT].setVoltage(newVoltage2);
      outputs[OUT3_OUTPUT].setVoltage(newVoltage3);
      if (oldVoltage1 != newVoltage1) outsChanged = true;
      if (oldVoltage2 != newVoltage2) outsChanged = true;
      if (oldVoltage3 != newVoltage3) outsChanged = true;
    }
    outputs[TRIG_OUTPUT].setVoltage(outsChanged ? 10.0f : 0.0f);
    outputs[MEL_8VA_OUTPUT].setVoltage(melVoltage + 1.0f);
    lastGate = gate;
    lastMelNote = melNote;

    // TODO: move to ChordOut expander
    int rootSub = rootBase + 36;
    assert(rootSub >= 0);
    outputs[ROOT_OUTPUT].setVoltage(midiToCV(rootSub));
	}
};


struct NestlingAudio_JazzWidget : ModuleWidget {
	NestlingAudio_JazzWidget(NestlingAudio_Jazz* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/NestlingAudio-Jazz.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<RoundBlackSnapKnob>(mm2px(Vec(30.48, 19.373)), module, NestlingAudio_Jazz::ROOT_PARAM));
		addParam(createParamCentered<RoundBlackSnapKnob>(mm2px(Vec(10.556, 39.184)), module, NestlingAudio_Jazz::CHORD_PARAM));
		addParam(createParamCentered<RoundBlackSnapKnob>(mm2px(Vec(49.833, 39.184)), module, NestlingAudio_Jazz::HARMONY_PARAM));
		addParam(createParamCentered<CKSSThreeHorizontal>(mm2px(Vec(30.48, 58.995)), module, NestlingAudio_Jazz::VOICING_PARAM));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10.506, 19.373)), module, NestlingAudio_Jazz::ROOT_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(49.833, 19.373)), module, NestlingAudio_Jazz::HARMONY_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(30.48, 39.184)), module, NestlingAudio_Jazz::CHORD_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(49.79, 58.995)), module, NestlingAudio_Jazz::VOICING_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10.506, 73.156)), module, NestlingAudio_Jazz::MEL_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10.506, 83.966)), module, NestlingAudio_Jazz::GATE_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(13.491, 102.632)), module, NestlingAudio_Jazz::OUT1_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(30.48, 102.632)), module, NestlingAudio_Jazz::OUT2_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(47.326, 102.632)), module, NestlingAudio_Jazz::OUT3_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(13.491, 112.692)), module, NestlingAudio_Jazz::ROOT_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(30.48, 112.692)), module, NestlingAudio_Jazz::TRIG_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(47.326, 112.692)), module, NestlingAudio_Jazz::MEL_8VA_OUTPUT));
	}
};


Model* modelNestlingAudio_Jazz = createModel<NestlingAudio_Jazz, NestlingAudio_JazzWidget>("NestlingAudio-Jazz");
