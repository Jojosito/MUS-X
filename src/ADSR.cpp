#include "plugin.hpp"


struct ADSR : Module {
	enum ParamId {
		A_PARAM,
		VELSCALE_PARAM,
		D_PARAM,
		S_PARAM,
		SUSMOD_PARAM,
		R_PARAM,
		PARAMS_LEN
	};
	enum InputId {
		VEL_INPUT,
		SUSMOD_INPUT,
		GATE_INPUT,
		RETRIG_INPUT,
		INPUTS_LEN
	};
	enum OutputId {
		SGATE_OUTPUT,
		ENV_OUTPUT,
		OUTPUTS_LEN
	};
	enum LightId {
		LIGHTS_LEN
	};

	enum Stage {
		STAGE_A = 0,
		STAGE_DS = 1,
		STAGE_R = 2
	};

	float voltage = 0;
	int stage = STAGE_R;

	dsp::TSchmittTrigger<float> triggerGate;
	dsp::TSchmittTrigger<float> triggerRetrigger;

	float attackCoeff = 0.f;
	float decayCoeff = 0.f;
	float releaseCoeff = 0.f;
	float sustainLevel = 0.5f;

	float lastAttackParam = 0.f;
	float lastDecayParam = 0.f;
	float lastReleaseParam = 0.f;

	ADSR() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configParam(A_PARAM, 2.f, 15.f, 0.f, "Attack time");
		configParam(D_PARAM, 2.f, 15.f, 0.f, "Decay time");
		configParam(S_PARAM, 0.f, 10.f, 5.f, "Sustain level");
		configParam(R_PARAM, 2.f, 15.f, 0.f, "Release time");

		configParam(VELSCALE_PARAM, 0.f, 1.f, 0.f, "Velocity CV");
		configParam(SUSMOD_PARAM, -1.f, 1.f, 0.f, "Sustain CV");

		configInput(VEL_INPUT, "Velocity CV input");
		configInput(SUSMOD_INPUT, "Sustain CV input");

		configInput(GATE_INPUT, "Gate input");
		configInput(RETRIG_INPUT, "Retrigger input");

		configOutput(SGATE_OUTPUT, "Decay/Sustain stage output");
		configOutput(ENV_OUTPUT, "Envelope output");
	}

	void process(const ProcessArgs& args) override {
		// coefficient calculation; only recalculate if params have changed
		if (params[A_PARAM].getValue() != lastAttackParam ||
			params[D_PARAM].getValue() != lastDecayParam ||
			params[R_PARAM].getValue() != lastReleaseParam)
		{
			lastAttackParam = params[A_PARAM].getValue();
			lastDecayParam = params[D_PARAM].getValue();
			lastReleaseParam = params[R_PARAM].getValue();

			attackCoeff = 1.f - exp(-lastAttackParam*44100/args.sampleRate);
			decayCoeff = 1.f - exp(-lastDecayParam*44100/args.sampleRate);
			releaseCoeff = 1.f - exp(-lastReleaseParam*44100/args.sampleRate);
		}

		// check release
		stage = triggerGate.isHigh() ? stage : STAGE_R;

		// check retrig / attack
		stage = triggerGate.process(inputs[GATE_INPUT].getVoltage(), 0.1f, 1.f) ? STAGE_A : stage;
		stage = triggerRetrigger.process(inputs[RETRIG_INPUT].getVoltage(), 0.1f, 1.f) ? STAGE_A : stage;

		switch (stage)
		{
			case STAGE_A:
				voltage = attackCoeff*voltage + (1.f-attackCoeff)*12.f;
				if (voltage > 10.f)
				{
					voltage = 10.f;
					stage = STAGE_DS;
				}
				break;
			case STAGE_DS:
				sustainLevel = params[S_PARAM].getValue() + params[SUSMOD_PARAM].getValue() * inputs[SUSMOD_INPUT].getVoltage();
				sustainLevel = clamp(sustainLevel, 0.f, 10.f);
				voltage = decayCoeff*voltage + (1.f-decayCoeff)*sustainLevel;
				break;
			case STAGE_R:
				voltage = releaseCoeff*voltage;
		}

		// velocity
		float velScaling = 1.f - params[VELSCALE_PARAM].getValue() + 0.1f*inputs[VEL_INPUT].getVoltage() * params[VELSCALE_PARAM].getValue();

		outputs[SGATE_OUTPUT].setVoltage((stage == STAGE_DS) ? 10.f : 0.f);
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
		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(22.86, 24.094)), module, ADSR::VELSCALE_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(7.62, 32.125)), module, ADSR::D_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(7.62, 48.188)), module, ADSR::S_PARAM));
		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(22.86, 56.219)), module, ADSR::SUSMOD_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(7.62, 64.25)), module, ADSR::R_PARAM));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.62, 80.313)), module, ADSR::VEL_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(22.86, 80.313)), module, ADSR::SUSMOD_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.62, 96.375)), module, ADSR::GATE_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(22.86, 96.375)), module, ADSR::RETRIG_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(7.62, 112.438)), module, ADSR::SGATE_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(22.86, 112.438)), module, ADSR::ENV_OUTPUT));
	}
};


Model* modelADSR = createModel<ADSR, ADSRWidget>("ADSR");
