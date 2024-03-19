#include "plugin.hpp"

namespace musx {

using namespace rack;

struct ParamQuantityIsSet : ParamQuantity {
	bool valueSet = false;
	void setValue(float value) override
	{
		ParamQuantity::setValue(value);
		valueSet = true;
	}
};

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
	float out = 0;

	bool detectChangesToSameValue = false;

	Last() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configParam<ParamQuantityIsSet>(A_PARAM, 0.f, 10.f, 0.f, "A")->smoothEnabled = false;
		configParam<ParamQuantityIsSet>(B_PARAM, 0.f, 10.f, 0.f, "B")->smoothEnabled = false;
		configParam<ParamQuantityIsSet>(C_PARAM, 0.f, 10.f, 0.f, "C")->smoothEnabled = false;
		configParam<ParamQuantityIsSet>(D_PARAM, 0.f, 10.f, 0.f, "D")->smoothEnabled = false;
		configOutput(OUT_OUTPUT, "Last value");
	}

	void process(const ProcessArgs& args) override {
		for (size_t i = 0; i<4; ++i)
		{
			if (detectChangesToSameValue && static_cast<ParamQuantityIsSet*>(paramQuantities[i])->valueSet)
			{
				static_cast<ParamQuantityIsSet*>(paramQuantities[i])->valueSet = false;
				out = params[i].getValue();
			}
			else if (params[i].getValue() != lastKnobValues[i])
			{
				out = params[i].getValue();
			}
			lastKnobValues[i] = params[i].getValue();
		}

		outputs[OUT_OUTPUT].setVoltage(out);
	}

	json_t* dataToJson() override {
		json_t* rootJ = json_object();
		json_object_set_new(rootJ, "detectChangesToSameValue", json_boolean(detectChangesToSameValue));
		return rootJ;
	}

	void dataFromJson(json_t* rootJ) override {
		json_t* detectChangesToSameValueJ = json_object_get(rootJ, "detectChangesToSameValue");
		if (detectChangesToSameValueJ)
		{
			detectChangesToSameValue = json_boolean_value(detectChangesToSameValueJ);
		}
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

	void appendContextMenu(Menu* menu) override {
		Last* module = getModule<Last>();

		menu->addChild(new MenuSeparator);

		menu->addChild(createBoolMenuItem("Detect changes to same value", "",
			[=]() {
				return module->detectChangesToSameValue;
			},
			[=](int mode) {
				module->detectChangesToSameValue = mode;
			}
		));
	}
};


Model* modelLast = createModel<Last, LastWidget>("Last");

}
