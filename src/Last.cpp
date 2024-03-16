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

	float lastKnobValues[4] = {0};
	float epsilon = 1e-6;
	float out = 0;

	Last() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configParam(A_PARAM, 0.f, 10.f, 0.f, "A")->smoothEnabled = false;
		configParam(B_PARAM, 0.f, 10.f, 0.f, "B")->smoothEnabled = false;
		configParam(C_PARAM, 0.f, 10.f, 0.f, "C")->smoothEnabled = false;
		configParam(D_PARAM, 0.f, 10.f, 0.f, "D")->smoothEnabled = false;
		configOutput(OUT_OUTPUT, "Last value");
	}

	void process(const ProcessArgs& args) override {
		for (size_t i = 0; i<4; ++i)
		{
			if (params[i].getValue() != lastKnobValues[i])
			{
				out = params[i].getValue();
				params[i].setValue(params[i].getValue() + epsilon);
			}
			lastKnobValues[i] = params[i].getValue();
		}

		outputs[OUT_OUTPUT].setVoltage(out);
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
