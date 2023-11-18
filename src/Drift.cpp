#include "plugin.hpp"

using simd::float_4;

namespace musx {

struct Drift : Module {
	enum ParamId {
		CONST_PARAM,
		RANDOMIZE_PARAM,
		DRIFT_PARAM,
		RATE_PARAM,
		PARAMS_LEN
	};
	enum InputId {
		POLY_INPUT,
		INPUTS_LEN
	};
	enum OutputId {
		OUT_OUTPUT,
		OUTPUTS_LEN
	};
	enum LightId {
		LIGHTS_LEN
	};

	const float minFreq = 0.01f; // min freq [Hz]
	const float base = 1000.f/minFreq; // max freq/min freq
	const int clockDivider = 16;

	int channels = 1;

	float_4 diverge[4];

	dsp::TRCFilter<float_4> lowpass[4];

	dsp::ClockDivider divider;

	float lastRateParam = -1.f;
	float driftScale = 1.f;

	float prevRandomizeValue = 0.f;

	Drift() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configParam(CONST_PARAM, 0.f, 1.f, 0.f, "Random constant offset", " %", 0.f, 100.f);
		configButton(RANDOMIZE_PARAM, "Randomize constant offsets");
		configParam(DRIFT_PARAM, 0.f, 1.f, 0.f, "Random drift", " %", 0.f, 100.f);
		configParam(RATE_PARAM, 0.f, 1.f, 0.f, "Drift rate", " Hz", base, minFreq);
		getParamQuantity(RATE_PARAM)->smoothEnabled = false;
		configInput(POLY_INPUT, "Polyphony channels");
		configOutput(OUT_OUTPUT, "Signal");

		divider.setDivision(clockDivider);

		randomizeDiverge();
	}

	void randomizeDiverge()
	{
		for (int c = 0; c < 4; c += 1) {
			diverge[c][0] = rack::random::get<float>() - 0.5f;
			diverge[c][1] = rack::random::get<float>() - 0.5f;
			diverge[c][2] = rack::random::get<float>() - 0.5f;
			diverge[c][3] = rack::random::get<float>() - 0.5f;

			diverge[c] *= 10.f; // +-5V
		}
	}

	void process(const ProcessArgs& args) override {
		if (divider.process())
		{
			channels = std::max(1, inputs[POLY_INPUT].getChannels());
			outputs[OUT_OUTPUT].setChannels(channels);

			// randomize
			if (params[RANDOMIZE_PARAM].getValue() && prevRandomizeValue == 0.f)
			{
				randomizeDiverge();
			}
			prevRandomizeValue = params[RANDOMIZE_PARAM].getValue();

			// update filter frequency
			if (params[RATE_PARAM].getValue() != lastRateParam)
			{
				for (int c = 0; c < 16; c += 4) {
					float cutoffFreq = simd::pow(base, params[RATE_PARAM].getValue()) * minFreq / args.sampleRate * clockDivider;
					lowpass[c/4].setCutoffFreq(cutoffFreq);
					lowpass[c/4].reset();

					driftScale = 1.e30;

					lastRateParam = params[RATE_PARAM].getValue();
				}
			}

			for (int c = 0; c < channels; c += 4) {
				float_4 rn = {rack::random::get<float>() - 0.5f,
							  rack::random::get<float>() - 0.5f,
							  rack::random::get<float>() - 0.5f,
							  rack::random::get<float>() - 0.5f};

				lowpass[c/4].process(rn);
				float_4 drift = lowpass[c/4].lowpass();

				// limit drift to +-5V
				for (int i = 0; i < 4; ++i)
				{
					driftScale = simd::fmin(driftScale, 5.f / std::abs(drift[i]));
				}

				outputs[OUT_OUTPUT].setVoltageSimd(simd::clamp(
						params[CONST_PARAM].getValue() * params[CONST_PARAM].getValue() * diverge[c/4] +
						params[DRIFT_PARAM].getValue() * params[DRIFT_PARAM].getValue() * driftScale * drift,
						-10.f, 10.f),
						c);
			}
		}
	}

	void onSampleRateChange(const SampleRateChangeEvent& e) override {
		lastRateParam = -1.f;
	}
};


struct DriftWidget : ModuleWidget {
	DriftWidget(Drift* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/Drift.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(7.62, 16.062)), module, Drift::CONST_PARAM));
		addParam(createParamCentered<VCVButton>(mm2px(Vec(7.62, 32.125)), module, Drift::RANDOMIZE_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(7.62, 48.188)), module, Drift::DRIFT_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(7.62, 64.251)), module, Drift::RATE_PARAM));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.62, 96.375)), module, Drift::POLY_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(7.62, 112.438)), module, Drift::OUT_OUTPUT));
	}
};


Model* modelDrift = createModel<Drift, DriftWidget>("Drift");

}
