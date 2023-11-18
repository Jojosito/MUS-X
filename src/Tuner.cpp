#include "plugin.hpp"

using simd::float_4;

namespace musx {

struct Tuner : Module {
	enum ParamId {
		OCTAVE_PARAM,
		SEMI_PARAM,
		FINE_PARAM,
		PARAMS_LEN
	};
	enum InputId {
		VOCT_INPUT,
		FINE_INPUT,
		INPUTS_LEN
	};
	enum OutputId {
		VOCT_OUTPUT,
		OUTPUTS_LEN
	};
	enum LightId {
		LIGHTS_LEN
	};

	bool snapOctaves = true;
	bool snapSemitones = true;

	Tuner() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configParam(OCTAVE_PARAM, -4.f, 4.f, 0.f, "Octave");
		configParam(SEMI_PARAM, -12.f, 12.f, 0.f, "Coarse tune", " cents", 0.f, 100.f);
		configParam(FINE_PARAM, -1.f/12.f, 1.f/12.f, 0.f, "Fine tune", " cents", 0.f, 1200.f);

		configInput(VOCT_INPUT, "V/Oct");
		configInput(FINE_INPUT, "5V/Semi");

		configOutput(VOCT_OUTPUT, "V/Oct");

		setSnap();
		configBypass(VOCT_INPUT, VOCT_OUTPUT);
	}

	void setSnap()
	{
		getParamQuantity(OCTAVE_PARAM)->snapEnabled = snapOctaves;
		getParamQuantity(OCTAVE_PARAM)->smoothEnabled = !snapOctaves;

		getParamQuantity(SEMI_PARAM)->snapEnabled = snapSemitones;
		getParamQuantity(SEMI_PARAM)->smoothEnabled = !snapSemitones;
	}

	void process(const ProcessArgs& args) override {
		int channels = std::max(1, inputs[VOCT_INPUT].getChannels());
		outputs[VOCT_OUTPUT].setChannels(channels);

		for (int c = 0; c < channels; c += 4) {
			float_4 v = inputs[VOCT_INPUT].getVoltageSimd<float_4>(c) + inputs[FINE_INPUT].getVoltageSimd<float_4>(c) / 60.f;
			v += params[OCTAVE_PARAM].getValue() + params[SEMI_PARAM].getValue()/12.f + params[FINE_PARAM].getValue();
			outputs[VOCT_OUTPUT].setVoltageSimd(simd::clamp(v, -12.f, 12.f), c);
		}
	}

	json_t* dataToJson() override {
		json_t* rootJ = json_object();
		json_object_set_new(rootJ, "snapOctaves", json_boolean(snapOctaves));
		json_object_set_new(rootJ, "snapSemitones", json_boolean(snapSemitones));
		return rootJ;
	}

	void dataFromJson(json_t* rootJ) override {
		json_t* snapOctavesJ = json_object_get(rootJ, "snapOctaves");
		if (snapOctavesJ)
		{
			snapOctaves = json_boolean_value(snapOctavesJ);
		}
		json_t* snapSemitonesJ = json_object_get(rootJ, "snapSemitones");
		if (snapSemitonesJ)
		{
			snapSemitones = json_boolean_value(snapSemitonesJ);
		}
		setSnap();
	}
};


struct TunerWidget : ModuleWidget {
	TunerWidget(Tuner* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/Tuner.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(7.62, 16.062)), module, Tuner::OCTAVE_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(7.62, 32.125)), module, Tuner::SEMI_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(7.62, 48.188)), module, Tuner::FINE_PARAM));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.62, 80.313)), module, Tuner::VOCT_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.62, 96.375)), module, Tuner::FINE_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(7.62, 112.438)), module, Tuner::VOCT_OUTPUT));
	}

	void appendContextMenu(Menu* menu) override {
		Tuner* module = getModule<Tuner>();

		menu->addChild(new MenuSeparator);

		menu->addChild(createIndexSubmenuItem("Snap octaves", {"no", "yes"},
			[=]() {
				return module->snapOctaves;
			},
			[=](int mode) {
				module->snapOctaves = mode;
				module->setSnap();
			}
		));

		menu->addChild(createIndexSubmenuItem("Snap semitones", {"no", "yes"},
			[=]() {
				return module->snapSemitones;
			},
			[=](int mode) {
				module->snapSemitones = mode;
				module->setSnap();
			}
		));
	}
};


Model* modelTuner = createModel<Tuner, TunerWidget>("Tuner");

}
