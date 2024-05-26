#include "plugin.hpp"
#include <math.hpp>
#include "dsp/decimator.hpp"
#include "dsp/filters.hpp"
#include "dsp/functions.hpp"

namespace musx {

using namespace rack;
using simd::float_4;

struct SKF : Module {
	enum ParamId {
		CUTOFF_PARAM,
		RESONANCE_PARAM,
		MODE_PARAM,
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
	const float maxFreq = 20480.f; // max freq [Hz] // must be 10 octaves for 1V/Oct cutoff CV scaling to work!
	const float base = maxFreq/minFreq; // max freq/min freq
	const float logBase = std::log(base);

	static const int maxOversamplingRate = 64;
	int oversamplingRate = 4;
	HalfBandDecimatorCascade<float_4> decimator[4];

	int iterations = 2;
	musx::AntialiasedCheapSaturator saturator1[4];
	musx::AntialiasedCheapSaturator saturator2[4];

	int channels = 1;

	musx::TOnePoleZDF<float_4> filter1[4];
	musx::TOnePoleZDF<float_4> filter2[4];

	float_4 prevInput[4] = {0};

	SKF() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configParam(CUTOFF_PARAM, 0.f, 1.f, 0.f, "Cutoff frequency", " Hz", base, minFreq);
		configParam(RESONANCE_PARAM, 0.f, 1.f, 0.f, "Resonance", " %", 0, 100.);
		configSwitch(MODE_PARAM, 0, 2, 0, "Mode", {"Lowpass", "Bandpass", "Highpass"});
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
			frequency  = simd::clamp(frequency, minFreq, simd::fmin(maxFreq, args.sampleRate * oversamplingRate / 2.2f));
			frequency /= args.sampleRate * oversamplingRate;
			// TODO fit again
			frequency  = (simd::exp(12.45 * frequency) - 1.) / 12.45; // prewarp. I fitted this, its not perfect, but good enough

			filter1[c/4].setCutoffFreq(frequency);
			filter2[c/4].copyCutoffFreq(filter1[c/4]);

			// resonance
			float_4 feedback = 5. * (params[RESONANCE_PARAM].getValue() + 0.1f * inputs[RESONANCE_INPUT].getPolyVoltageSimd<float_4>(c));


			float_4* inBuffer = decimator[c/4].getInputArray(oversamplingRate);

			for (int i = 0; i < oversamplingRate; ++i)
			{
				// linear interpolation for input
				// TODO proper upsampling with halfband filters!!! !
				float_4 in = crossfade(prevInput[c/4], inputs[IN_INPUT].getVoltageSimd<float_4>(c), (i+1.f)/oversamplingRate);

				// filtering
				switch ((int)params[MODE_PARAM].getValue())
				{
				case 0: // LP
					for (int i=0; i<iterations; i++)
					{
						filter1[c/4].processDry(in + saturator1[c/4].processNonBandlimited(feedback * filter2[c/4].highpass()));
						filter2[c/4].processDry(saturator2[c/4].processNonBandlimited(filter1[c/4].lowpass()));
					}
					filter1[c/4].process(in + saturator1[c/4].process(feedback * filter2[c/4].highpass()));
					filter2[c/4].process(saturator2[c/4].process(filter1[c/4].lowpass()));
					inBuffer[i] = filter2[c/4].lowpass();
					break;
				case 1: // BP
					for (int i=0; i<iterations; i++)
					{
						filter1[c/4].processDry(in + saturator1[c/4].processNonBandlimited(feedback * filter2[c/4].highpass()));
						filter2[c/4].processDry(saturator2[c/4].processNonBandlimited(filter1[c/4].lowpass()));
					}
					filter1[c/4].process(in + saturator1[c/4].process(feedback * filter2[c/4].highpass()));
					filter2[c/4].process(saturator2[c/4].process(filter1[c/4].lowpass()));
					inBuffer[i] = filter2[c/4].highpass();
					break;
				case 2: // HP
					for (int i=0; i<iterations; i++)
					{
						filter1[c/4].processDry(in + saturator1[c/4].processNonBandlimited(feedback * filter2[c/4].lowpass()));
						filter2[c/4].processDry(saturator2[c/4].processNonBandlimited(filter1[c/4].highpass()));
					}
					filter1[c/4].process(in + saturator1[c/4].process(feedback * filter2[c/4].lowpass()));
					filter2[c/4].process(saturator2[c/4].process(filter1[c/4].highpass()));
					inBuffer[i] = filter2[c/4].highpass();
				}
			}

			prevInput[c/4] = inputs[IN_INPUT].getVoltageSimd<float_4>(c);

			// downsampling
			float_4 out = decimator[c/4].process(oversamplingRate);

			outputs[OUT_OUTPUT].setVoltageSimd(out, c);
		}
	}


	json_t* dataToJson() override {
		json_t* rootJ = json_object();
		json_object_set_new(rootJ, "oversamplingRate", json_integer(oversamplingRate));
		return rootJ;
	}

	void dataFromJson(json_t* rootJ) override {
		json_t* oversamplingRateJ = json_object_get(rootJ, "oversamplingRate");
		if (oversamplingRateJ)
		{
			oversamplingRate = json_integer_value(oversamplingRateJ);
		}
	}
};


struct SKFWidget : ModuleWidget {
	SKFWidget(SKF* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/SKF.svg"), asset::plugin(pluginInstance, "res/SKF-dark.svg")));

		addChild(createWidget<ThemedScrew>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ThemedScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ThemedScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ThemedScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(7.62, 16.062)), module, SKF::CUTOFF_PARAM));
		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(7.62, 26.26)), module, SKF::CUTOFF_INPUT));

		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(7.62, 44.52)), module, SKF::RESONANCE_PARAM));
		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(7.62, 54.59)), module, SKF::RESONANCE_INPUT));

		addParam(createParamCentered<NKK>(mm2px(Vec(7.62, 72.04)), module, SKF::MODE_PARAM));

		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(7.62, 96.375)), module, SKF::IN_INPUT));

		addOutput(createOutputCentered<ThemedPJ301MPort>(mm2px(Vec(7.62, 112.438)), module, SKF::OUT_OUTPUT));
	}

	void appendContextMenu(Menu* menu) override {
		SKF* module = getModule<SKF>();

		menu->addChild(new MenuSeparator);

		menu->addChild(createIndexSubmenuItem("Oversampling rate", {"1x", "2x", "4x", "8x", "16x", "32x", "64x"},
			[=]() {
				return log2(module->oversamplingRate);
			},
			[=](int mode) {
				module->oversamplingRate = std::pow(2, mode);
			}
		));

		menu->addChild(createIndexSubmenuItem("iterations", {"0", "1", "2", "3", "4", "5"},
			[=]() {
				return module->iterations;
			},
			[=](int mode) {
				module->iterations = mode;
			}
		));
	}
};


Model* modelSKF = createModel<SKF, SKFWidget>("SKF");

}
