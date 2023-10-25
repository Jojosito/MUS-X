#include "plugin.hpp"

using simd::float_4;

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

	Tuner() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configParam(OCTAVE_PARAM, -4.f, 4.f, 0.f, "Octave");
		getParamQuantity(OCTAVE_PARAM)->snapEnabled = true;
		configParam(SEMI_PARAM, -1.f, 1.f, 0.f, "Coarse tune", " cents", 0.f, 1200.f);
		configParam(FINE_PARAM, -1.f/12.f, 1.f/12.f, 0.f, "Fine tune", " cents", 0.f, 1200.f);
		configInput(VOCT_INPUT, "V/Oct");
		configInput(FINE_INPUT, "5V/Semi");
		configOutput(VOCT_OUTPUT, "V/Oct");

		configBypass(VOCT_INPUT, VOCT_OUTPUT);
	}

	void process(const ProcessArgs& args) override {
		int channels = std::max(1, inputs[VOCT_INPUT].getChannels());
		outputs[VOCT_OUTPUT].setChannels(channels);

		for (int c = 0; c < channels; c += 4) {
			float_4 v = inputs[VOCT_INPUT].getVoltageSimd<float_4>(c) + inputs[FINE_INPUT].getVoltageSimd<float_4>(c) / 60.f;
			v += params[OCTAVE_PARAM].getValue() + params[SEMI_PARAM].getValue() + params[FINE_PARAM].getValue();
			outputs[VOCT_OUTPUT].setVoltageSimd(simd::clamp(v, -12.f, 12.f), c);
		}
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
};


Model* modelTuner = createModel<Tuner, TunerWidget>("Tuner");
