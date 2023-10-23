#include "plugin.hpp"


struct ADSR : Module {
	enum ParamId {
		A_PARAM,
		D_PARAM,
		S_PARAM,
		R_PARAM,
		VELSCALE_PARAM,
		PARAMS_LEN
	};
	enum InputId {
		GATE_INPUT,
		RETRIG_INPUT,
		VEL_INPUT,
		INPUTS_LEN
	};
	enum OutputId {
		ENV_OUTPUT,
		OUTPUTS_LEN
	};
	enum LightId {
		BLINK_LIGHT,
		LIGHTS_LEN
	};
	enum Stage {
		STAGE_A = 0,
		STAGE_D = 1,
		STAGE_R = 2
	};

	float voltage = 0;
	int stage = STAGE_R;

	ADSR() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configParam(A_PARAM, 0.99f, 1.f, 0.f, "Attack");
		configParam(D_PARAM, 0.99f, 1.f, 0.f, "Decay");
		configParam(S_PARAM, 0.f, 1.f, 1.f, "Sustain");
		configParam(R_PARAM, 0.99f, 1.f, 0.f, "Release");
		configParam(VELSCALE_PARAM, 0.f, 1.f, 0.f, "Velocity scaling");
		configInput(GATE_INPUT, "Gate");
		configInput(RETRIG_INPUT, "Retrigger");
		configInput(VEL_INPUT, "Velocity");
		configOutput(ENV_OUTPUT, "Envelope");
	}

	void process(const ProcessArgs& args) override {
		float gate = inputs[GATE_INPUT].getVoltage() - inputs[RETRIG_INPUT].getVoltage();

		// TODO scaling
		float attackParam = params[A_PARAM].getValue();
		float decayParam = params[D_PARAM].getValue();
		float sustainParam = 10.f * params[S_PARAM].getValue();
		float releaseParam = params[R_PARAM].getValue();

		// TODO branchless
		// check retrig / attack
		if (gate > 1.f && stage == STAGE_R)
		{
			stage = STAGE_A;
		}
		// check release
		if (gate < 0.1f && stage != STAGE_R)
		{
			stage = STAGE_R;
		}


		switch (stage)
		{
			case STAGE_A:
				voltage = attackParam*voltage + (1-attackParam)*gate*1.2f;
				if (voltage > 10.f)
				{
					voltage = 10.f;
					stage = STAGE_D;
				}
				break;
			case STAGE_D:
				voltage = decayParam*voltage + (1-decayParam)*sustainParam;
				break;
			case STAGE_R:
				voltage = releaseParam*voltage;
		}

		// TODO velocity
		outputs[ENV_OUTPUT].setVoltage(voltage);
	}
};


struct ADSRWidget : ModuleWidget {
	ADSRWidget(ADSR* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/ADSR.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(7.62, 16.062)), module, ADSR::A_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(7.62, 32.125)), module, ADSR::D_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(7.62, 48.188)), module, ADSR::S_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(7.62, 64.25)), module, ADSR::R_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(7.62, 80.313)), module, ADSR::VELSCALE_PARAM));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.62, 96.375)), module, ADSR::GATE_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(22.86, 96.375)), module, ADSR::RETRIG_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.62, 112.438)), module, ADSR::VEL_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(22.86, 112.438)), module, ADSR::ENV_OUTPUT));

		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(22.86, 80.313)), module, ADSR::BLINK_LIGHT));
	}
};


Model* modelADSR = createModel<ADSR, ADSRWidget>("ADSR");
