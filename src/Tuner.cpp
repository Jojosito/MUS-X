#include "plugin.hpp"

namespace musx {

using namespace rack;
using simd::float_4;

struct Tuner : Module {
	enum ParamId {
		OCTAVE_PARAM,
		SEMI_PARAM,
		FINE_PARAM,
		PARAMS_LEN
	};
	enum InputId {
		VOCT1_INPUT,
		VOCT2_INPUT,
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

	// ranges must be max, snap must be off so that parameter values can be loaded correctly
	// order of calls: 1. constructor   2. parameter values loaded   3. dataFromJson   4. process
	int octaveRange = 7;
	bool snapOctaves = false;
	int semiRange = 12;
	bool snapSemitones = false;
	bool newModule = true;

	Tuner() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		setSnap();
		configParam(FINE_PARAM, -1.f/12.f, 1.f/12.f, 0.f, "Fine tune", " cents", 0.f, 1200.f);

		configInput(VOCT1_INPUT, "V/Oct");
		configInput(VOCT2_INPUT, "V/Oct");
		configInput(FINE_INPUT, "5V/Semi");

		configOutput(VOCT_OUTPUT, "V/Oct");

		configBypass(VOCT1_INPUT, VOCT_OUTPUT);
	}

	void setSnap()
	{
		float newValue = params[OCTAVE_PARAM].getValue();
		newValue = std::min(newValue, (float)octaveRange);
		newValue = std::max(newValue, -(float)octaveRange);
		if (snapOctaves)
		{
			newValue = (float)(int)newValue;
		}
		configParam(OCTAVE_PARAM, -octaveRange, octaveRange, 0.f, "Octave");
		getParamQuantity(OCTAVE_PARAM)->snapEnabled = snapOctaves;
		getParamQuantity(OCTAVE_PARAM)->smoothEnabled = !snapOctaves;
		params[OCTAVE_PARAM].setValue(newValue);

		newValue = params[SEMI_PARAM].getValue();
		newValue = std::min(newValue, (float)semiRange);
		newValue = std::max(newValue, -(float)semiRange);
		if (snapSemitones)
		{
			newValue = (float)(int)newValue;
		}
		configParam(SEMI_PARAM, -semiRange, semiRange, 0.f, "Coarse tune", " cents", 0.f, 100.f);
		getParamQuantity(SEMI_PARAM)->snapEnabled = snapSemitones;
		getParamQuantity(SEMI_PARAM)->smoothEnabled = !snapSemitones;
		params[SEMI_PARAM].setValue(newValue);
	}

	void process(const ProcessArgs& args) override {
		if (newModule)
		{
			// default values for new modules
			newModule = false;
			octaveRange = 4;
			snapOctaves = true;
			snapSemitones = true;
			setSnap();
		}

		int channels = std::max(1, inputs[VOCT1_INPUT].getChannels());
		channels = std::max(channels, inputs[VOCT2_INPUT].getChannels());
		channels = std::max(channels, inputs[FINE_INPUT].getChannels());
		outputs[VOCT_OUTPUT].setChannels(channels);

		for (int c = 0; c < channels; c += 4) {
			float_4 v = inputs[VOCT1_INPUT].getPolyVoltageSimd<float_4>(c) + inputs[VOCT2_INPUT].getPolyVoltageSimd<float_4>(c)
							+ inputs[FINE_INPUT].getPolyVoltageSimd<float_4>(c) / 60.f
							+ params[OCTAVE_PARAM].getValue() + params[SEMI_PARAM].getValue()/12.f + params[FINE_PARAM].getValue();
			outputs[VOCT_OUTPUT].setVoltageSimd(simd::clamp(v, -12.f, 12.f), c);
		}
	}

	json_t* dataToJson() override {
		json_t* rootJ = json_object();
		json_object_set_new(rootJ, "octaveRange", json_integer(octaveRange));
		json_object_set_new(rootJ, "snapOctaves", json_boolean(snapOctaves));

		json_object_set_new(rootJ, "semiRange", json_integer(semiRange));
		json_object_set_new(rootJ, "snapSemitones", json_boolean(snapSemitones));
		return rootJ;
	}

	void dataFromJson(json_t* rootJ) override {
		json_t* octaveRangeJ = json_object_get(rootJ, "octaveRange");
		if (octaveRangeJ)
		{
			octaveRange = json_integer_value(octaveRangeJ);
		}
		json_t* snapOctavesJ = json_object_get(rootJ, "snapOctaves");
		if (snapOctavesJ)
		{
			snapOctaves = json_boolean_value(snapOctavesJ);
		}

		json_t* semiRangeJ = json_object_get(rootJ, "semiRange");
		if (semiRangeJ)
		{
			semiRange = json_integer_value(semiRangeJ);
		}
		json_t* snapSemitonesJ = json_object_get(rootJ, "snapSemitones");
		if (snapSemitonesJ)
		{
			snapSemitones = json_boolean_value(snapSemitonesJ);
		}
		newModule = false;
		setSnap();
	}
};


struct TunerWidget : ModuleWidget {
	TunerWidget(Tuner* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/Tuner.svg"), asset::plugin(pluginInstance, "res/Tuner-dark.svg")));

		addChild(createWidget<ThemedScrew>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ThemedScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ThemedScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ThemedScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(7.62, 16.062)), module, Tuner::OCTAVE_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(7.62, 32.125)), module, Tuner::SEMI_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(7.62, 48.188)), module, Tuner::FINE_PARAM));

		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(7.62, 71.281)), module, Tuner::VOCT1_INPUT));
		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(7.62, 80.313)), module, Tuner::VOCT2_INPUT));
		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(7.62, 96.375)), module, Tuner::FINE_INPUT));

		addOutput(createOutputCentered<ThemedPJ301MPort>(mm2px(Vec(7.62, 112.438)), module, Tuner::VOCT_OUTPUT));
	}

	void appendContextMenu(Menu* menu) override {
		Tuner* module = getModule<Tuner>();

		menu->addChild(new MenuSeparator);

		menu->addChild(createIndexSubmenuItem("Octave range", {"3 (±1)", "5 (±2)", "7 (±3)", "9 (±4)", "11 (±5)", "13 (±6)", "15 (±7)"},
			[=]() {
				return module->octaveRange - 1;
			},
			[=](int mode) {
				module->octaveRange = mode + 1;
				module->setSnap();
			}
		));

		menu->addChild(createBoolMenuItem("Snap octaves", "",
			[=]() {
				return module->snapOctaves;
			},
			[=](int mode) {
				module->snapOctaves = mode;
				module->setSnap();
			}
		));

		menu->addChild(new MenuSeparator);

		menu->addChild(createIndexSubmenuItem("Semitone range", {"3 (±1)", "5 (±2)", "7 (±3)", "9 (±4)", "11 (±5)", "13 (±6)", "15 (±7)", "17 (±8)", "19 (±9)", "21 (±10)", "23 (±11)", "25 (±12)"},
			[=]() {
				return module->semiRange - 1;
			},
			[=](int mode) {
				module->semiRange = mode + 1;
				module->setSnap();
			}
		));

		menu->addChild(createBoolMenuItem("Snap semitones", "",
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
