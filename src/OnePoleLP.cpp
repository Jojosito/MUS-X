#include "plugin.hpp"
#include "dsp/filters.hpp"

namespace musx {

using namespace rack;
using simd::float_4;

struct OnePoleLP : Module {
	enum ParamId {
		LOWPASS_PARAM,
		PARAMS_LEN
	};
	enum InputId {
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

	const float minFreq = 0.01f; // min freq [Hz]
	const float base = 22000.f/minFreq; // max freq/min freq
	const float logBase = std::log(base);

	int channels = 1;

	musx::TOnePoleZDF<float_4> lowpass[4];

	dsp::ClockDivider cvDivider;

	OnePoleLP() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configParam(LOWPASS_PARAM, 0.f, 1.f, 1.f, "Low pass cutoff frequency", " Hz", base, minFreq);
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
				float_4 voltage = params[LOWPASS_PARAM].getValue() + 0.1f * inputs[LOWPASS_INPUT].getPolyVoltageSimd<float_4>(c);
				float_4 frequency = simd::exp(logBase * voltage) * minFreq;
				frequency = simd::clamp(frequency, 0.f, args.sampleRate/2.f);
				lowpass[c/4].setCutoffFreq(frequency / args.sampleRate);
			}
		}

		for (int c = 0; c < channels; c += 4) {

			lowpass[c/4].process(inputs[IN_INPUT].getVoltageSimd<float_4>(c));

			outputs[OUT_OUTPUT].setVoltageSimd(lowpass[c/4].lowpass(), c);
		}

	}
};


struct OnePoleLPWidget : ModuleWidget {
	OnePoleLPWidget(OnePoleLP* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/OnePoleLP.svg"), asset::plugin(pluginInstance, "res/OnePoleLP-dark.svg")));

		addChild(createWidget<ThemedScrew>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ThemedScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ThemedScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ThemedScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(7.62, 48.188)), module, OnePoleLP::LOWPASS_PARAM));

		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(7.62, 64.25)), module, OnePoleLP::LOWPASS_INPUT));
		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(7.62, 96.375)), module, OnePoleLP::IN_INPUT));

		addOutput(createOutputCentered<ThemedPJ301MPort>(mm2px(Vec(7.62, 112.438)), module, OnePoleLP::OUT_OUTPUT));
	}
};


Model* modelOnePoleLP = createModel<OnePoleLP, OnePoleLPWidget>("OnePoleLP");

}
