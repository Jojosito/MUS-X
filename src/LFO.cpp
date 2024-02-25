#include "plugin.hpp"

namespace musx {

using namespace rack;
using simd::float_4;
using simd::int32_4;

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

	static constexpr float minFreq = 0.004f; // Hz
	static constexpr float maxFreq = 1000.f; // Hz

	int channels = 1;

	int sampleRateReduction = 1;
	bool bipolar = true;

	// integers overflow, so phase resets automatically
	int32_4 phasor[4] = {0};

	dsp::ClockDivider divider;

	LFO() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configParam(SHAPE_PARAM, 0.f, 1.f, 0.f, "Shape");
		configParam(FREQ_PARAM, 0.f, 1.f, 0.5f, "Frequency", " Hz", maxFreq/minFreq, minFreq);
		configParam(AMP_PARAM, 0.f, 5.f, 0.f, "Amplitude", " V");
		configButton(RESET_PARAM, "Reset phase");
		configInput(FREQ_INPUT, "Frequency CV");
		configInput(AMP_INPUT, "Amplitude CV");
		configInput(RESET_INPUT, "Reset trigger");
		configOutput(OUT_OUTPUT, "LFO");
	}

	void setSampleRateReduction(int arg)
	{
		sampleRateReduction = arg;
		divider.setDivision(sampleRateReduction);
	}

	void process(const ProcessArgs& args) override {
		if (divider.process())
		{
			//
			// channels
			//
			channels = 1;

			channels = std::max(channels, inputs[FREQ_INPUT].getChannels());
			channels = std::max(channels, inputs[AMP_INPUT].getChannels());
			channels = std::max(channels, inputs[RESET_INPUT].getChannels());

			outputs[OUT_OUTPUT].setChannels(channels);
		}
	}

	json_t* dataToJson() override {
		json_t* rootJ = json_object();
		json_object_set_new(rootJ, "sampleRateReduction", json_integer(sampleRateReduction));
		json_object_set_new(rootJ, "bipolar", json_boolean(bipolar));
		return rootJ;
	}

	void dataFromJson(json_t* rootJ) override {
		json_t* sampleRateReductionJ = json_object_get(rootJ, "sampleRateReduction");
		if (sampleRateReductionJ)
		{
			setSampleRateReduction(json_integer_value(sampleRateReductionJ));
		}
		json_t* bipolarJ = json_object_get(rootJ, "bipolar");
		if (bipolarJ)
		{
			bipolar = json_boolean_value(bipolarJ);
		}
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

	void appendContextMenu(Menu* menu) override {
		LFO* module = getModule<LFO>();

		menu->addChild(new MenuSeparator);

		menu->addChild(createIndexSubmenuItem("Reduce internal sample rate", {"1x", "2x", "4x", "8x", "16x", "32x", "64x", "128x", "256x", "512x", "1024x"},
			[=]() {
				return log2(module->sampleRateReduction);
			},
			[=](int mode) {
				module->setSampleRateReduction(std::pow(2, mode));
			}
		));

		menu->addChild(createBoolMenuItem("Bipolar", "",
			[=]() {
				return module->bipolar;
			},
			[=](int mode) {
				module->bipolar = mode;
			}
		));
	}
};


Model* modelLFO = createModel<LFO, LFOWidget>("LFO");

}
