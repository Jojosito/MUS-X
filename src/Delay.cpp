#include "plugin.hpp"


struct Delay : Module {
	enum ParamId {
		TIME_PARAM,
		FEEDBACK_PARAM,
		HP_PARAM,
		LP_PARAM,
		STEREO_WIDTH_PARAM,
		MIX_PARAM,
		PARAMS_LEN
	};
	enum InputId {
		L_INPUT,
		R_INPUT,
		INPUTS_LEN
	};
	enum OutputId {
		L_OUTPUT,
		R_OUTPUT,
		OUTPUTS_LEN
	};
	enum LightId {
		LIGHTS_LEN
	};

	static const int delayLineSize = 48000;
	float delayLine1[delayLineSize] = {0};
	float delayLine2[delayLineSize] = {0};
	int indexIn = 0;
	int indexOut = 0;
	float delayR = 0;

	Delay() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configParam(TIME_PARAM, 0.f, 1.f, 0.5f, "Delay time", " ms");
		configParam(FEEDBACK_PARAM, 0.f, 1.f, 0.f, "Feedback", " %");
		configParam(HP_PARAM, 0.f, 1.f, 0.f, "High pass", " Hz");
		configParam(LP_PARAM, 0.f, 1.f, 0.f, "Low pass", " Hz");
		configParam(STEREO_WIDTH_PARAM, 0.f, 1.f, 1.f, "Stereo width", " %");
		configParam(MIX_PARAM, 0.f, 1.f, 0.5f, "Dry-wet mix");
		configInput(L_INPUT, "Left / Mono");
		configInput(R_INPUT, "Right");
		configOutput(L_OUTPUT, "Left");
		configOutput(R_OUTPUT, "Right");
	}

	void process(const ProcessArgs& args) override {
		float inL = inputs[L_INPUT].getVoltage();
		float inR = inL;
		if (inputs[R_INPUT].isConnected())
		{
			inR = inputs[R_INPUT].getVoltage();
		}

		float inMono = 0.5f * (inL + inR) + params[FEEDBACK_PARAM].getValue() * delayR;

		delayLine1[indexIn] = inMono;
		float delayL = delayLine1[indexOut];

		delayLine2[indexIn] = delayL;
		delayR = delayLine2[indexOut];


		++indexIn;
		indexIn = (indexIn >= delayLineSize) ? 0 : indexIn;

		indexOut = indexIn - 0.9f * params[TIME_PARAM].getValue() * delayLineSize;
		indexOut = (indexOut < 0) ? indexOut + delayLineSize : indexOut;


		outputs[L_OUTPUT].setVoltage(std::min(1.f, (2.f - 2.f * params[MIX_PARAM].getValue())) * inL +
				std::min(1.f, 2.f * params[MIX_PARAM].getValue()) * delayL);

		outputs[R_OUTPUT].setVoltage(std::min(1.f, (2.f - 2.f * params[MIX_PARAM].getValue())) * inR +
				std::min(1.f, 2.f * params[MIX_PARAM].getValue()) * delayR);
	}
};


struct DelayWidget : ModuleWidget {
	DelayWidget(Delay* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/Delay.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<RoundBigBlackKnob>(mm2px(Vec(15.24, 24.094)), module, Delay::TIME_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(15.24, 48.188)), module, Delay::FEEDBACK_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(7.62, 64.25)), module, Delay::HP_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(22.86, 64.25)), module, Delay::LP_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(7.62, 80.313)), module, Delay::STEREO_WIDTH_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(22.86, 80.313)), module, Delay::MIX_PARAM));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.62, 96.375)), module, Delay::L_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(22.86, 96.375)), module, Delay::R_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(7.62, 112.438)), module, Delay::L_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(22.86, 112.438)), module, Delay::R_OUTPUT));
	}
};


Model* modelDelay = createModel<Delay, DelayWidget>("Delay");
