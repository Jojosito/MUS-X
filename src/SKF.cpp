#include "plugin.hpp"
#include "dsp/filters.hpp"
#include "dsp/functions.hpp"

namespace musx {

using namespace rack;
using simd::float_4;

struct SKF : Module {
	enum ParamId {
		CUTOFF_PARAM,
		RESONANCE_PARAM,
		PARAMS_LEN
	};
	enum InputId {
		CUTOFF_INPUT,
		RESONANCE_INPUT,
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

	const float minFreq = 20.f; // min freq [Hz]
	const float maxFreq = 20000.f; // min freq [Hz]
	const float base = maxFreq/minFreq; // max freq/min freq
	const float logBase = std::log(base);

	int channels = 1;

	musx::TOnePole<float_4> filter1[4];
	musx::TOnePole<float_4> filter2[4];

	SKF() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configParam(CUTOFF_PARAM, 0.f, 1.f, 0.f, "Cutoff frequency", " Hz", base, minFreq);
		configParam(RESONANCE_PARAM, 0.f, 1.f, 0.f, "Resonance", " %", 0, 100.);
		configInput(CUTOFF_INPUT, "Cutoff frequency CV");
		configInput(RESONANCE_INPUT, "Resonance CV");
		configInput(IN_INPUT, "Audio");
		configOutput(OUT_OUTPUT, "Filtered");

		configBypass(IN_INPUT, OUT_OUTPUT);
	}

	void process(const ProcessArgs& args) override {

		channels = std::max(1, inputs[IN_INPUT].getChannels());
		outputs[OUT_OUTPUT].setChannels(channels);

		for (int c = 0; c < channels; c += 4) {
			// set cutoff
			float_4 voltage = params[CUTOFF_PARAM].getValue() + 0.1f * inputs[CUTOFF_INPUT].getPolyVoltageSimd<float_4>(c);
			float_4 frequency = simd::exp(logBase * voltage) * minFreq;
			frequency = simd::clamp(frequency, minFreq, simd::fmin(maxFreq, args.sampleRate/2.1f));
			filter1[c/4].setCutoffFreq(frequency / args.sampleRate);
			filter2[c/4].copyCutoffFreq(filter1[c/4]);

			// resonance
			float_4 feedback = 10. * (params[RESONANCE_PARAM].getValue() + 0.1f * inputs[RESONANCE_INPUT].getPolyVoltageSimd<float_4>(c));

			// feedback and saturation
			float_4 in = inputs[IN_INPUT].getVoltageSimd<float_4>(c) + cheapSaturator(feedback * filter2[c/4].highpass());

			// filtering
			filter1[c/4].process(in);
			filter2[c/4].process(filter1[c/4].lowpass());

			outputs[OUT_OUTPUT].setVoltageSimd(filter2[c/4].lowpass(), c);
		}
	}
};


struct SKFWidget : ModuleWidget {
	SKFWidget(SKF* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/SKF.svg")));

		addChild(createWidget<ThemedScrew>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ThemedScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ThemedScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ThemedScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(7.62, 16.062)), module, SKF::CUTOFF_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(7.62, 48.188)), module, SKF::RESONANCE_PARAM));

		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(7.62, 32.125)), module, SKF::CUTOFF_INPUT));
		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(7.62, 64.25)), module, SKF::RESONANCE_INPUT));
		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(7.62, 96.375)), module, SKF::IN_INPUT));

		addOutput(createOutputCentered<ThemedPJ301MPort>(mm2px(Vec(7.62, 112.438)), module, SKF::OUT_OUTPUT));
	}
};


Model* modelSKF = createModel<SKF, SKFWidget>("SKF");

}
