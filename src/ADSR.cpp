/**
 * This file uses code from https://github.com/VCVRack/Fundamental/blob/v2/src/ADSR.cpp
 * copyright © 2016-2023 VCV
 * copyright © 2023 MUS-X
 */

#include "plugin.hpp"

using simd::float_4;

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


	static constexpr float MIN_TIME = 5e-4f;
	static constexpr float MAX_TIME = 20.f;
	static constexpr float LAMBDA_BASE = MAX_TIME / MIN_TIME;
	static constexpr float ATT_TARGET = 1.2f;

	int channels = 1;
	float_4 gate[4] = {};
	float_4 attacking[4] = {};
	float_4 env[4] = {};
	dsp::TSchmittTrigger<float_4> trigger[4];
	float_4 attackLambda[4] = {};
	float_4 decayLambda[4] = {};
	float_4 releaseLambda[4] = {};
	float_4 sustain[4] = {};

	float lastAttackParam = -1.f;
	float lastDecayParam = -1.f;
	float lastReleaseParam = -1.f;

	ADSR() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configParam(A_PARAM, 0.f, 1.f, 0.5f, "Attack", " ms", LAMBDA_BASE, MIN_TIME * 1000);
		configParam(D_PARAM, 0.f, 1.f, 0.5f, "Decay", " ms", LAMBDA_BASE, MIN_TIME * 1000);
		configParam(S_PARAM, 0.f, 1.f, 0.5f, "Sustain", "%", 0, 100);
		configParam(R_PARAM, 0.f, 1.f, 0.5f, "Release", " ms", LAMBDA_BASE, MIN_TIME * 1000);

		configParam(VELSCALE_PARAM, 0.f, 1.f, 0.f, "Velocity CV");
		configParam(SUSMOD_PARAM, -1.f, 1.f, 0.f, "Sustain CV");

		configInput(VEL_INPUT, "Velocity CV");
		configInput(SUSMOD_INPUT, "Sustain CV");

		configInput(GATE_INPUT, "Gate");
		configInput(RETRIG_INPUT, "Retrigger");

		configOutput(SGATE_OUTPUT, "Decay/Sustain stage");
		configOutput(ENV_OUTPUT, "Envelope");
	}

	void process(const ProcessArgs& args) override {

		// coefficient calculation; only recalculate if params have changed
		if (channels != std::max(1, inputs[GATE_INPUT].getChannels()) ||
			params[A_PARAM].getValue() != lastAttackParam ||
			params[D_PARAM].getValue() != lastDecayParam ||
			params[R_PARAM].getValue() != lastReleaseParam)
		{
			channels = std::max(1, inputs[GATE_INPUT].getChannels());
			outputs[SGATE_OUTPUT].setChannels(channels);
			outputs[ENV_OUTPUT].setChannels(channels);

			lastAttackParam = params[A_PARAM].getValue();
			lastDecayParam = params[D_PARAM].getValue();
			lastReleaseParam = params[R_PARAM].getValue();

			for (int c = 0; c < channels; c += 4) {
				attackLambda[c/4] = simd::pow(LAMBDA_BASE, -lastAttackParam) / MIN_TIME;
				decayLambda[c/4] = simd::pow(LAMBDA_BASE, -lastDecayParam) / MIN_TIME;
				releaseLambda[c/4] = simd::pow(LAMBDA_BASE, -lastReleaseParam) / MIN_TIME;
			}
		}


		for (int c = 0; c < channels; c += 4) {
			this->sustain[c/4] = params[S_PARAM].getValue() +
					inputs[SUSMOD_INPUT].getPolyVoltageSimd<float_4>(c) / 10.f * params[SUSMOD_PARAM].getValue();

			// Gate
			float_4 oldGate = gate[c/4];
			gate[c/4] = inputs[GATE_INPUT].getVoltageSimd<float_4>(c) >= 1.f;
			attacking[c/4] |= (gate[c/4] & ~oldGate);

			// Retrigger
			float_4 triggered = trigger[c/4].process(inputs[RETRIG_INPUT].getPolyVoltageSimd<float_4>(c));
			attacking[c/4] |= triggered;

			// Turn off attacking state if gate is LOW
			attacking[c/4] &= gate[c/4];

			// Get target and lambda for exponential decay
			float_4 target = simd::ifelse(attacking[c/4], ATT_TARGET, simd::ifelse(gate[c/4], sustain[c/4], 0.f));
			float_4 lambda = simd::ifelse(attacking[c/4], attackLambda[c/4], simd::ifelse(gate[c/4], decayLambda[c/4], releaseLambda[c/4]));

			// Adjust env
			env[c/4] += (target - env[c/4]) * lambda * args.sampleTime;

			// Turn off attacking state if envelope is HIGH
			attacking[c/4] &= (env[c/4] < 1.f);

			// velocity
			float_4 velScaling = 1.f - params[VELSCALE_PARAM].getValue() +
					0.1f*inputs[VEL_INPUT].getPolyVoltageSimd<float_4>(c) * params[VELSCALE_PARAM].getValue();

			// Set output
			outputs[ENV_OUTPUT].setVoltageSimd(10.f * velScaling * env[c/4], c);

			outputs[SGATE_OUTPUT].setVoltageSimd(simd::ifelse((gate[c/4] & ~attacking[c/4]), 10.f, 0.f), c);
		}

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
