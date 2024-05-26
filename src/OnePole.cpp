#include "plugin.hpp"
#include "dsp/filters.hpp"

namespace musx {

using namespace rack;
using simd::float_4;

struct OnePole : Module {
	enum ParamId {
		HIGHPASS_PARAM,
		LOWPASS_PARAM,
		PARAMS_LEN
	};
	enum InputId {
		HIGHPASS_INPUT,
		LOWPASS_INPUT,
		IN_INPUT,
		INPUTS_LEN
	};
	enum OutputId {
		OUT_OUTPUT,
		OUTPUTS_LEN
	};
	enum LightId {
		LIGHTS_LEN
	};

	const float minFreq = 5.f; // min freq [Hz]
	const float base = 22000.f/minFreq; // max freq/min freq
	const float logBase = std::log(base);

	int channels = 1;

	musx::TOnePoleZDF<float_4> highpass[4];
	musx::TOnePoleZDF<float_4> lowpass[4];

	dsp::ClockDivider cvDivider;

	OnePole() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configParam(HIGHPASS_PARAM, 0.f, 1.f, 0.f, "High pass cutoff frequency", " Hz", base, minFreq);
		configParam(LOWPASS_PARAM, 0.f, 1.f, 1.f, "Low pass cutoff frequency", " Hz", base, minFreq);
		configInput(HIGHPASS_INPUT, "High pass cutoff frequency CV");
		configInput(LOWPASS_INPUT, "Low pass cutoff frequency CV");
		configInput(IN_INPUT, "Audio");
		configOutput(OUT_OUTPUT, "Filtered");

		cvDivider.setDivision(8);

		configBypass(IN_INPUT, OUT_OUTPUT);
	}

	void process(const ProcessArgs& args) override {
		if (cvDivider.process())
		{
			channels = std::max(1, inputs[IN_INPUT].getChannels());
			outputs[OUT_OUTPUT].setChannels(channels);

			for (int c = 0; c < channels; c += 4) {
				float_4 voltage = params[HIGHPASS_PARAM].getValue() + 0.1f * inputs[HIGHPASS_INPUT].getPolyVoltageSimd<float_4>(c);
				float_4 frequency = simd::exp(logBase * voltage) * minFreq;
				frequency = simd::clamp(frequency, 1.f, args.sampleRate/2.1f);
				highpass[c/4].setCutoffFreq(frequency / args.sampleRate);

				voltage = params[LOWPASS_PARAM].getValue() + 0.1f * inputs[LOWPASS_INPUT].getPolyVoltageSimd<float_4>(c);
				frequency = simd::exp(logBase * voltage) * minFreq;
				frequency = simd::clamp(frequency, 0.f, args.sampleRate/2.f);
				lowpass[c/4].setCutoffFreq(frequency / args.sampleRate);
			}
		}

		for (int c = 0; c < channels; c += 4) {

			highpass[c/4].process(inputs[IN_INPUT].getVoltageSimd<float_4>(c));
			lowpass[c/4].process(highpass[c/4].highpass());

			outputs[OUT_OUTPUT].setVoltageSimd(lowpass[c/4].lowpass(), c);
		}

	}
};


struct OnePoleWidget : ModuleWidget {
	OnePoleWidget(OnePole* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/OnePole.svg"), asset::plugin(pluginInstance, "res/OnePole-dark.svg")));

		addChild(createWidget<ThemedScrew>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ThemedScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ThemedScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ThemedScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(7.62, 16.062)), module, OnePole::HIGHPASS_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(7.62, 48.188)), module, OnePole::LOWPASS_PARAM));

		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(7.62, 32.125)), module, OnePole::HIGHPASS_INPUT));
		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(7.62, 64.25)), module, OnePole::LOWPASS_INPUT));
		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(7.62, 96.375)), module, OnePole::IN_INPUT));

		addOutput(createOutputCentered<ThemedPJ301MPort>(mm2px(Vec(7.62, 112.438)), module, OnePole::OUT_OUTPUT));
	}
};


Model* modelOnePole = createModel<OnePole, OnePoleWidget>("OnePole");

}
