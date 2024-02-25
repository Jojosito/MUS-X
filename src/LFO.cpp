#include "plugin.hpp"

namespace musx {

using namespace rack;
using simd::float_4;

struct LFO : Module {
	enum ParamId {
		SHAPE_PARAM,
		FREQ_PARAM,
		AMP_PARAM,
		RESET_PARAM,
		PARAMS_LEN
	};
	enum InputId {
		FREQ_INPUT,
		AMP_INPUT,
		RESET_INPUT,
		INPUTS_LEN
	};
	enum OutputId {
		OUT_OUTPUT,
		OUTPUTS_LEN
	};
	enum LightId {
		LIGHTS_LEN
	};

	LFO() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configParam(SHAPE_PARAM, 0.f, 1.f, 0.f, "");
		configParam(FREQ_PARAM, 0.f, 1.f, 0.f, "");
		configParam(AMP_PARAM, 0.f, 1.f, 0.f, "");
		configParam(RESET_PARAM, 0.f, 1.f, 0.f, "");
		configInput(FREQ_INPUT, "");
		configInput(AMP_INPUT, "");
		configInput(RESET_INPUT, "");
		configOutput(OUT_OUTPUT, "");
	}

	void process(const ProcessArgs& args) override {
	}
};


struct LFOWidget : ModuleWidget {
	LFOWidget(LFO* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/LFO.svg"), asset::plugin(pluginInstance, "res/LFO-dark.svg")));

		addChild(createWidget<ThemedScrew>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ThemedScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ThemedScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ThemedScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(7.62, 16.591)), module, LFO::SHAPE_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(7.62, 33.183)), module, LFO::FREQ_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(7.62, 59.114)), module, LFO::AMP_PARAM));
		addParam(createParamCentered<VCVButton>(mm2px(Vec(7.62, 84.64)), module, LFO::RESET_PARAM));

		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(7.62, 43.581)), module, LFO::FREQ_INPUT));
		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(7.62, 69.635)), module, LFO::AMP_INPUT));
		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(7.62, 92.671)), module, LFO::RESET_INPUT));

		addOutput(createOutputCentered<ThemedPJ301MPort>(mm2px(Vec(7.62, 112.438)), module, LFO::OUT_OUTPUT));
	}
};


Model* modelLFO = createModel<LFO, LFOWidget>("LFO");

}
