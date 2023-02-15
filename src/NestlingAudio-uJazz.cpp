#include "plugin.hpp"
#include "NestlingAudio.hpp"


struct NestlingAudio_uJazz : NestlingAudio {
	enum ParamId {
		PARAMS_LEN
	};
	enum InputId {
		ROOT_INPUT,
		CHORD_INPUT,
		MEL_INPUT,
		INPUTS_LEN
	};
	enum OutputId {
		OUT1_OUTPUT,
		OUT2_OUTPUT,
		OUT3_OUTPUT,
		OUTPUTS_LEN
	};
	enum LightId {
		LIGHTS_LEN
	};

  Harmonizer* harmonizer;

	NestlingAudio_uJazz() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configInput(ROOT_INPUT, "chord root 1v/oct");
		configInput(CHORD_INPUT, "chord type 0v to 10v");  // TODO: change if we make this configurable
		configInput(MEL_INPUT, "melody note 1v/oct");
		configOutput(OUT1_OUTPUT, "harmony note 1, 1v/oct");
		configOutput(OUT2_OUTPUT, "harmony note 2, 1v/oct");
		configOutput(OUT3_OUTPUT, "harmony note 3, 1v/oct");

    loadChords();
		harmonizer = new BasicHarmonizer();
    harmonizer->init(chordIntervals);
	}

	void process(const ProcessArgs& args) override {
    // INPUT
    float rootVoltage = inputs[ROOT_INPUT].getVoltage();
    int rootBase = cvToMidi(rootVoltage) % 12;
    getInputInfo(ROOT_INPUT)->description = midiToString(rootBase);

  	size_t chordTypeIndex = cvToIndex(inputs[CHORD_INPUT].getVoltage(), chordNames.size());
    getInputInfo(CHORD_INPUT)->description = chordNames[chordTypeIndex];

    float melVoltage = inputs[MEL_INPUT].getVoltage();
    int melNote = cvToMidi(melVoltage);
    getInputInfo(MEL_INPUT)->description = midiToString(melNote);

		// ALGORITHM
    int offsets[3];
    harmonizer->computeOffsets(chordTypeIndex, melNote, rootBase, offsets);

    outputs[OUT1_OUTPUT].setVoltage(midiToCV(melNote + offsets[0]));
    outputs[OUT2_OUTPUT].setVoltage(midiToCV(melNote + offsets[1]));
    outputs[OUT3_OUTPUT].setVoltage(midiToCV(melNote + offsets[2]));
	}
};


struct NestlingAudio_uJazzWidget : ModuleWidget {
	NestlingAudio_uJazzWidget(NestlingAudio_uJazz* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/NestlingAudio-uJazz.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10.506, 19.373)), module, NestlingAudio_uJazz::ROOT_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10.506, 33.832)), module, NestlingAudio_uJazz::CHORD_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10.506, 68.394)), module, NestlingAudio_uJazz::MEL_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(13.491, 100.359)), module, NestlingAudio_uJazz::OUT1_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(13.491, 108.536)), module, NestlingAudio_uJazz::OUT2_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(13.491, 116.714)), module, NestlingAudio_uJazz::OUT3_OUTPUT));
	}
};


Model* modelNestlingAudio_uJazz = createModel<NestlingAudio_uJazz, NestlingAudio_uJazzWidget>("NestlingAudio-uJazz");
