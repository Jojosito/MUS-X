#include "plugin.hpp"
#include "blocks/ADSRBlock.hpp"

namespace musx {

using namespace rack;
using simd::float_4;

struct ADSR : Module {
	enum ParamId {
		A_PARAM,
		D_PARAM,
		S_PARAM,
		R_PARAM,
		VELSCALE_PARAM,
		RANDSCALE_PARAM,
		SUSMOD_PARAM,
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

	int channels = 1;

	float lastAttackParam = -1.f;
	float lastDecayParam = -1.f;
	float lastReleaseParam = -1.f;
	float lastRandParam = -1.f;

	float_4 randomA[4] = {};
	float_4 randomD[4] = {};
	float_4 randomS[4] = {};
	float_4 randomR[4] = {};

	static constexpr float MIN_TIME = 5e-4f;
	static constexpr float MAX_TIME = 20.f;
	static constexpr float LAMBDA_BASE = MAX_TIME / MIN_TIME;
	static constexpr float ATT_TARGET = 1.2f;
	musx::ADSRBlock adsrBlock[4] = {musx::ADSRBlock(MIN_TIME, MAX_TIME, ATT_TARGET)};

	ADSR() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configParam(A_PARAM, 0.f, 1.f, 0.1f, "Attack", " ms", LAMBDA_BASE, MIN_TIME * 1000);
		configParam(D_PARAM, 0.f, 1.f, 0.1f, "Decay", " ms", LAMBDA_BASE, MIN_TIME * 1000);
		configParam(S_PARAM, 0.f, 1.f, 1.f, "Sustain", " %", 0, 100);
		configParam(R_PARAM, 0.f, 1.f, 0.1f, "Release", " ms", LAMBDA_BASE, MIN_TIME * 1000);

		configParam(VELSCALE_PARAM, 0.f, 1.f, 0.f, "Velocity scaling", " %", 0, 100);
		configParam(RANDSCALE_PARAM, 0.f, 1.f, 0.f, "Polyphonic random scaling", " %", 0, 100);
		configParam(SUSMOD_PARAM, -1.f, 1.f, 0.f, "Sustain CV", " %", 0, 100);

		configInput(VEL_INPUT, "Velocity CV");
		configInput(SUSMOD_INPUT, "Sustain CV");

		configInput(GATE_INPUT, "Gate");
		configInput(RETRIG_INPUT, "Retrigger");

		configOutput(SGATE_OUTPUT, "Decay/Sustain stage");
		configOutput(ENV_OUTPUT, "Envelope");

		for (int c = 0; c < 16; c += 4) {
			randomA[c/4] = {rack::random::get<float>() - 0.5f,
						  rack::random::get<float>() - 0.5f,
						  rack::random::get<float>() - 0.5f,
						  rack::random::get<float>() - 0.5f};
			randomD[c/4] = {rack::random::get<float>() - 0.5f,
						  rack::random::get<float>() - 0.5f,
						  rack::random::get<float>() - 0.5f,
						  rack::random::get<float>() - 0.5f};
			randomS[c/4] = {rack::random::get<float>() - 0.5f,
						  rack::random::get<float>() - 0.5f,
						  rack::random::get<float>() - 0.5f,
						  rack::random::get<float>() - 0.5f};
			randomS[c/4] *= 0.5;
			randomR[c/4] = {rack::random::get<float>() - 0.5f,
						  rack::random::get<float>() - 0.5f,
						  rack::random::get<float>() - 0.5f,
						  rack::random::get<float>() - 0.5f};
		}
	}

	void process(const ProcessArgs& args) override {

		// coefficient calculation; only recalculate if params have changed
		if (channels != std::max(1, inputs[GATE_INPUT].getChannels()) ||
			params[A_PARAM].getValue() != lastAttackParam ||
			params[D_PARAM].getValue() != lastDecayParam ||
			params[R_PARAM].getValue() != lastReleaseParam ||
			params[RANDSCALE_PARAM].getValue() != lastRandParam)
		{
			channels = std::max(1, inputs[GATE_INPUT].getChannels());
			outputs[SGATE_OUTPUT].setChannels(channels);
			outputs[ENV_OUTPUT].setChannels(channels);

			lastAttackParam = params[A_PARAM].getValue();
			lastDecayParam = params[D_PARAM].getValue();
			lastReleaseParam = params[R_PARAM].getValue();
			lastRandParam = params[RANDSCALE_PARAM].getValue();

			for (int c = 0; c < channels; c += 4) {
				adsrBlock[c/4].setAttackTime(lastAttackParam);
				adsrBlock[c/4].multAttackLambda(1.f + lastRandParam * randomA[c/4]);
				adsrBlock[c/4].setDecayTime(lastDecayParam);
				adsrBlock[c/4].multDecayLambda(1.f + lastRandParam * randomD[c/4]);
				adsrBlock[c/4].setReleaseTime(lastReleaseParam);
				adsrBlock[c/4].multReleaseLambda(1.f + lastRandParam * randomR[c/4]);
			}
		}


		for (int c = 0; c < channels; c += 4) {
			adsrBlock[c/4].setSustainLevel(
					(params[S_PARAM].getValue() + inputs[SUSMOD_INPUT].getPolyVoltageSimd<float_4>(c) * 0.1f * params[SUSMOD_PARAM].getValue())
					* (1.f + lastRandParam * randomS[c/4]));

			adsrBlock[c/4].setVelocityScaling(params[VELSCALE_PARAM].getValue());
			adsrBlock[c/4].setVelocity(inputs[VEL_INPUT].getVoltageSimd<float_4>(c));
			adsrBlock[c/4].setGate(inputs[GATE_INPUT].getVoltageSimd<float_4>(c));
			adsrBlock[c/4].setRetrigger(inputs[RETRIG_INPUT].getVoltageSimd<float_4>(c));

			// Set output
			outputs[ENV_OUTPUT].setVoltageSimd(adsrBlock[c/4].process(args.sampleTime), c);

			outputs[SGATE_OUTPUT].setVoltageSimd(adsrBlock[c/4].getDecaySustainGate(), c);
		}

	}

	void onUnBypass(const UnBypassEvent& e) override {
		// force recalculation and setting of output channels
		lastAttackParam = -1.f;
	}

	void onPortChange(const PortChangeEvent& e) override {
		// force recalculation and setting of output channels
		lastAttackParam = -1.f;
	}

	json_t* dataToJson() override {
		json_t* rootJ = json_object();

		json_t* randomAJ = json_array();
		for (int i = 0; i < 16; i++)
		{
			json_array_insert_new(randomAJ, i, json_real(randomA[i/4][i%4]));
		}
		json_object_set_new(rootJ, "randomA", randomAJ);

		json_t* randomDJ = json_array();
		for (int i = 0; i < 16; i++)
		{
			json_array_insert_new(randomDJ, i, json_real(randomD[i/4][i%4]));
		}
		json_object_set_new(rootJ, "randomD", randomDJ);

		json_t* randomSJ = json_array();
		for (int i = 0; i < 16; i++)
		{
			json_array_insert_new(randomSJ, i, json_real(randomS[i/4][i%4]));
		}
		json_object_set_new(rootJ, "randomS", randomSJ);

		json_t* randomRJ = json_array();
		for (int i = 0; i < 16; i++)
		{
			json_array_insert_new(randomRJ, i, json_real(randomR[i/4][i%4]));
		}
		json_object_set_new(rootJ, "randomR", randomRJ);

		return rootJ;
	}

	void dataFromJson(json_t* rootJ) override {
		json_t* randomAsJ = json_object_get(rootJ, "randomA");
		json_t* randomDsJ = json_object_get(rootJ, "randomD");
		json_t* randomSsJ = json_object_get(rootJ, "randomS");
		json_t* randomRsJ = json_object_get(rootJ, "randomR");
		if (randomAsJ && randomDsJ && randomSsJ && randomRsJ)
		{
			for (int i = 0; i < 16; i++)
			{
				json_t* randomAJ = json_array_get(randomAsJ, i);
				if (randomAJ)
				{
					randomA[i/4][i%4] = json_real_value(randomAJ);
				}

				json_t* randomDJ = json_array_get(randomDsJ, i);
				if (randomDJ)
				{
					randomD[i/4][i%4] = json_real_value(randomDJ);
				}

				json_t* randomSJ = json_array_get(randomSsJ, i);
				if (randomSJ)
				{
					randomS[i/4][i%4] = json_real_value(randomSJ);
				}

				json_t* randomRJ = json_array_get(randomRsJ, i);
				if (randomRJ)
				{
					randomR[i/4][i%4] = json_real_value(randomRJ);
				}
			}
		}
	}
};


struct ADSRWidget : ModuleWidget {
	ADSRWidget(ADSR* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/ADSR.svg"), asset::plugin(pluginInstance, "res/ADSR-dark.svg")));

		addChild(createWidget<ThemedScrew>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ThemedScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ThemedScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ThemedScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(7.62, 16.062)), module, ADSR::A_PARAM));
		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(22.86, 16.062)), module, ADSR::VELSCALE_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(7.62, 32.125)), module, ADSR::D_PARAM));
		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(22.86, 32.125)), module, ADSR::RANDSCALE_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(7.62, 48.188)), module, ADSR::S_PARAM));
		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(22.86, 56.219)), module, ADSR::SUSMOD_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(7.62, 64.25)), module, ADSR::R_PARAM));

		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(7.62, 80.313)), module, ADSR::VEL_INPUT));
		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(22.86, 80.313)), module, ADSR::SUSMOD_INPUT));
		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(7.62, 96.375)), module, ADSR::GATE_INPUT));
		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(22.86, 96.375)), module, ADSR::RETRIG_INPUT));

		addOutput(createOutputCentered<ThemedPJ301MPort>(mm2px(Vec(7.62, 112.438)), module, ADSR::SGATE_OUTPUT));
		addOutput(createOutputCentered<ThemedPJ301MPort>(mm2px(Vec(22.86, 112.438)), module, ADSR::ENV_OUTPUT));
	}
};


Model* modelADSR = createModel<ADSR, ADSRWidget>("ADSR");

}
