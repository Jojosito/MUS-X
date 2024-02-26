#include "plugin.hpp"

namespace musx {

using namespace rack;
using simd::float_4;
using simd::int32_4;

struct LFO : Module {
	enum ParamId {
		SHAPE_PARAM,
		FREQ_PARAM,
		AMP_PARAM,
		RESET_PARAM,
		PARAMS_LEN
	};
	enum InputId {
		FREQ_INPUT,
		AMP_INPUT,
		RESET_INPUT,
		INPUTS_LEN
	};
	enum OutputId {
		OUT_OUTPUT,
		OUTPUTS_LEN
	};
	enum LightId {
		LIGHTS_LEN
	};

	static constexpr int octaveRange = 7;
	static constexpr float minFreq = 2 * std::pow(2, -octaveRange); // Hz
	static constexpr float maxFreq = 2 * std::pow(2,  octaveRange); // Hz
	float logMaxOverMin = std::log(maxFreq/minFreq); // log(maxFreq/minFreq)

	int channels = 1;

	int sampleRateReduction = 1;
	bool bipolar = true;

	// integers overflow, so phase resets automatically
	int32_4 phasor[4] = {0};

	float_4 wave[4] = {0}; // -1..1

	float_4 reset[4] = {0};

	dsp::ClockDivider divider;

	LFO() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configSwitch(SHAPE_PARAM, 0.f, 6.f, 0.f, "Shape", {"Sine", "Triangle", "Square", "Ramp", "Saw", "Sample & hold", "Warped"});
		getParamQuantity(SHAPE_PARAM)->snapEnabled = true;
		configParam(FREQ_PARAM, -octaveRange, octaveRange, 0.f, "Frequency", " Hz", 2., 2.);
		configParam(AMP_PARAM, 0.f, 5.f, 5.f, "Amplitude", " V");
		configButton(RESET_PARAM, "Reset phase");
		configInput(FREQ_INPUT, "Frequency CV");
		configInput(AMP_INPUT, "Amplitude CV");
		configInput(RESET_INPUT, "Reset trigger");
		configOutput(OUT_OUTPUT, "LFO");
	}

	void setSampleRateReduction(int arg)
	{
		sampleRateReduction = arg;
		divider.setDivision(sampleRateReduction);
	}

	void process(const ProcessArgs& args) override {
		if (divider.process())
		{
			//
			// channels
			//
			channels = 1;

			channels = std::max(channels, inputs[FREQ_INPUT].getChannels());
			channels = std::max(channels, inputs[AMP_INPUT].getChannels());
			channels = std::max(channels, inputs[RESET_INPUT].getChannels());

			outputs[OUT_OUTPUT].setChannels(channels);

			for (int c = 0; c < channels; c += 4) {
				int32_4 lastPhasor = phasor[c/4];

				// reset
				float_4 lastReset = reset[c/4];
				reset[c/4] = params[RESET_PARAM].getValue() + inputs[RESET_INPUT].getPolyVoltageSimd<float_4>(c);

				phasor[c/4] = simd::ifelse(reset[c/4] > lastReset + 0.5, -INT32_MAX, phasor[c/4]);

				// frequencies, phase increments, factors etc
				float_4 freq = 2. * dsp::exp2_taylor5(params[FREQ_PARAM].getValue() + inputs[FREQ_INPUT].getPolyVoltageSimd<float_4>(c));
				int32_4 phaseInc = INT32_MAX / args.sampleRate * freq * sampleRateReduction;

				float_4 oldWave = wave[c/4];

				switch((int)params[SHAPE_PARAM].getValue())
				{
					case 0:
						// sine
						phasor[c/4] += 2*phaseInc;
						wave[c/4] = -1. * simd::sin((float_4)(phasor[c/4]/INT32_MAX)*M_PI);
						break;
					case 1:
						// tri
						phasor[c/4] += 2*phaseInc;
						wave[c/4] = 2. * simd::ifelse(phasor[c/4] < 0, (float_4)(phasor[c/4]/INT32_MAX), -(float_4)(phasor[c/4]/INT32_MAX)) + 1.;
						break;
					case 2:
						// square
						phasor[c/4] += 2*phaseInc;
						wave[c/4] = 2. * (float_4)(phasor[c/4] > 0 * 2. - 1.) + 1.;
						break;
					case 3:
						// ramp
						phasor[c/4] += 2*phaseInc;
						wave[c/4] = (float_4)(phasor[c/4]/INT32_MAX);
						break;
					case 4:
						// saw
						phasor[c/4] += 2*phaseInc;
						wave[c/4] = -(float_4)(phasor[c/4]/INT32_MAX);
						break;
					case 5:
						// s&h
						phasor[c/4] += 4*phaseInc;
						wave[c/4] = simd::ifelse(lastPhasor > phasor[c/4], 2. * rack::random::get<float>() - 1.f, oldWave);
						break;
					case 6:
						// warped
						phasor[c/4] += phaseInc;
						wave[c/4] = 0.7*simd::sin((float_4)(phasor[c/4]/INT32_MAX)*M_PI);
						wave[c/4] -= simd::sin((float_4)(2*phasor[c/4]/INT32_MAX)*M_PI + 0.4 * M_PI);
						break;
				}

				// unipolar/bipolar, amplitude
				float offset = 1 - bipolar;
				float_4 amp = params[AMP_PARAM].getValue() + inputs[AMP_INPUT].getPolyVoltageSimd<float_4>(c);
				outputs[OUT_OUTPUT].setVoltageSimd(amp*(wave[c/4] + offset), c);
			}

		}
	}

	json_t* dataToJson() override {
		json_t* rootJ = json_object();
		json_object_set_new(rootJ, "sampleRateReduction", json_integer(sampleRateReduction));
		json_object_set_new(rootJ, "bipolar", json_boolean(bipolar));
		return rootJ;
	}

	void dataFromJson(json_t* rootJ) override {
		json_t* sampleRateReductionJ = json_object_get(rootJ, "sampleRateReduction");
		if (sampleRateReductionJ)
		{
			setSampleRateReduction(json_integer_value(sampleRateReductionJ));
		}
		json_t* bipolarJ = json_object_get(rootJ, "bipolar");
		if (bipolarJ)
		{
			bipolar = json_boolean_value(bipolarJ);
		}
	}
};


struct LFOWidget : ModuleWidget {
	LFOWidget(LFO* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/LFO.svg"), asset::plugin(pluginInstance, "res/LFO-dark.svg")));

		addChild(createWidget<ThemedScrew>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ThemedScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ThemedScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ThemedScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(7.62, 16.591)), module, LFO::SHAPE_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(7.62, 33.183)), module, LFO::FREQ_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(7.62, 59.114)), module, LFO::AMP_PARAM));
		addParam(createParamCentered<VCVButton>(mm2px(Vec(7.62, 84.64)), module, LFO::RESET_PARAM));

		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(7.62, 43.581)), module, LFO::FREQ_INPUT));
		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(7.62, 69.635)), module, LFO::AMP_INPUT));
		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(7.62, 92.671)), module, LFO::RESET_INPUT));

		addOutput(createOutputCentered<ThemedPJ301MPort>(mm2px(Vec(7.62, 112.438)), module, LFO::OUT_OUTPUT));
	}

	void appendContextMenu(Menu* menu) override {
		LFO* module = getModule<LFO>();

		menu->addChild(new MenuSeparator);

		menu->addChild(createIndexSubmenuItem("Reduce internal sample rate", {"1x", "2x", "4x", "8x", "16x", "32x", "64x", "128x", "256x", "512x", "1024x"},
			[=]() {
				return log2(module->sampleRateReduction);
			},
			[=](int mode) {
				module->setSampleRateReduction(std::pow(2, mode));
			}
		));

		menu->addChild(createBoolMenuItem("Bipolar", "",
			[=]() {
				return module->bipolar;
			},
			[=](int mode) {
				module->bipolar = mode;
			}
		));
	}
};


Model* modelLFO = createModel<LFO, LFOWidget>("LFO");

}
