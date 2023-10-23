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

	float attackCoeff = 0.f;
	float decayCoeff = 0.f;
	float releaseCoeff = 0.f;

	float lastAttackParam = 0.f;
	float lastDecayParam = 0.f;
	float lastReleaseParam = 0.f;

	ADSR() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configParam(A_PARAM, 2.f, 15.f, 0.f, "Attack");
		configParam(D_PARAM, 2.f, 15.f, 0.f, "Decay");
		configParam(S_PARAM, 0.f, 10.f, 5.f, "Sustain");
		configParam(R_PARAM, 2.f, 15.f, 0.f, "Release");
		configParam(VELSCALE_PARAM, 0.f, 1.f, 0.f, "Velocity scaling");
		configInput(GATE_INPUT, "Gate");
		configInput(RETRIG_INPUT, "Retrigger");
		configInput(VEL_INPUT, "Velocity");
		configOutput(ENV_OUTPUT, "Envelope");
	}

	void process(const ProcessArgs& args) override {
		// coefficient calculation; only recalculate if params changed
		if (params[A_PARAM].getValue() != lastAttackParam ||
			params[D_PARAM].getValue() != lastDecayParam ||
			params[R_PARAM].getValue() != lastReleaseParam)
		{
			lights[ENV_OUTPUT].setBrightness(1);
			lastAttackParam = params[A_PARAM].getValue();
			lastDecayParam = params[D_PARAM].getValue();
			lastReleaseParam = params[R_PARAM].getValue();

			attackCoeff = 1.f - exp(-lastAttackParam*44100/args.sampleRate);
			decayCoeff = 1.f - exp(-lastDecayParam*44100/args.sampleRate);
			releaseCoeff = 1.f - exp(-lastReleaseParam*44100/args.sampleRate);
			lights[ENV_OUTPUT].setBrightness(0);
		}

		float sustainParam = params[S_PARAM].getValue();

		float gate = inputs[GATE_INPUT].getVoltage() - inputs[RETRIG_INPUT].getVoltage();

		// check retrig / attack
		stage = (gate > 1.f && stage == STAGE_R) ? STAGE_A : stage;
		// check release
		stage = (gate < 0.1f && stage != STAGE_R) ? STAGE_R : stage;

		switch (stage)
		{
			case STAGE_A:
				voltage = attackCoeff*voltage + (1.f-attackCoeff)*gate*1.2f;
				if (voltage > 10.f)
				{
					voltage = 10.f;
					stage = STAGE_D;
				}
				break;
			case STAGE_D:
				voltage = decayCoeff*voltage + (1.f-decayCoeff)*sustainParam;
				break;
			case STAGE_R:
				voltage = releaseCoeff*voltage;
		}

		// velocity
		float velScaling = 1.f - params[VELSCALE_PARAM].getValue() + 0.1f*inputs[VEL_INPUT].getVoltage() * params[VELSCALE_PARAM].getValue();
		outputs[ENV_OUTPUT].setVoltage(velScaling * voltage);
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
