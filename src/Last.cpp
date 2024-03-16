#include "plugin.hpp"

namespace musx {

using namespace rack;

struct Last : Module {
	enum ParamId {
		A_PARAM,
		B_PARAM,
		C_PARAM,
		D_PARAM,
		PARAMS_LEN
	};
	enum InputId {
		INPUTS_LEN
	};
	enum OutputId {
		OUT_OUTPUT,
		OUTPUTS_LEN
	};
	enum LightId {
		LIGHTS_LEN
	};

	Last() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configParam(A_PARAM, 0.f, 1.f, 0.f, "A");
		configParam(B_PARAM, 0.f, 1.f, 0.f, "B");
		configParam(C_PARAM, 0.f, 1.f, 0.f, "C");
		configParam(D_PARAM, 0.f, 1.f, 0.f, "D");
		configOutput(OUT_OUTPUT, "Last value");
	}

	void process(const ProcessArgs& args) override {
	}
};


struct LastWidget : ModuleWidget {
	LastWidget(Last* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/Last.svg"), asset::plugin(pluginInstance, "res/Last-dark.svg")));

		addChild(createWidget<ThemedScrew>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ThemedScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ThemedScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ThemedScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(7.62, 24.094)), module, Last::A_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(7.62, 40.157)), module, Last::B_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(7.62, 56.22)), module, Last::C_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(7.62, 72.283)), module, Last::D_PARAM));

		addOutput(createOutputCentered<ThemedPJ301MPort>(mm2px(Vec(7.62, 112.438)), module, Last::OUT_OUTPUT));
	}
};


Model* modelLast = createModel<Last, LastWidget>("Last");

}
