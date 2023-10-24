#include "plugin.hpp"

using simd::float_4;
using simd::int32_4;

struct Oscillators : Module {
	enum ParamId {
		OSC1SHAPE_PARAM,
		OSC1PW_PARAM,
		OSC1VOL_PARAM,
		OSC2SHAPE_PARAM,
		OSC2PW_PARAM,
		OSC2VOL_PARAM,
		SYNC_PARAM,
		CROSSMOD_PARAM,
		RINGMOD_PARAM,
		PARAMS_LEN
	};
	enum InputId {
		OSC1SHAPE_INPUT,
		OSC1PW_INPUT,
		OSC1VOL_INPUT,
		OSC2SHAPE_INPUT,
		OSC2PW_INPUT,
		OSC2VOL_INPUT,
		CROSSMOD_INPUT,
		RINGMOD_INPUT,
		OSC1VOCT_INPUT,
		OSC2VOCT_INPUT,
		INPUTS_LEN
	};
	enum OutputId {
		OUT_OUTPUT,
		OUTPUTS_LEN
	};
	enum LightId {
		SYNC_LIGHT,
		LIGHTS_LEN
	};

	int oversamplingRate = 16;
	static const int maxOversamplingRate = 64;
	static const int oversamplingQuality = 16;
	dsp::Decimator<4, oversamplingQuality, float_4> decimator4[4];
	dsp::Decimator<16, oversamplingQuality, float_4> decimator16[4];
	dsp::Decimator<64, oversamplingQuality, float_4> decimator64[4];

	int channels = 1;

	// integers overflow, so phase resets automatically
	int32_4 phasor1[4] = {0};
	int32_4 phasor2[4] = {0};

	float_4 mix[4][maxOversamplingRate] = {0};

	dsp::ClockDivider CvDivider;
	dsp::ClockDivider lightDivider;

	// CV modulated parameters
	float_4 osc1Shape[4] = {0};
	float_4 osc1PW[4] = {0};
	float_4 osc1Vol[4] = {0};

	float_4 osc2Shape[4] = {0};
	float_4 osc2PW[4] = {0};
	float_4 osc2Vol[4] = {0};

	float_4 crossmod[4] = {0};
	float_4 ringmod[4] = {0};

	Oscillators() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configParam(OSC1SHAPE_PARAM, 	0.f, 1.f, 0.f, 	"Oscillator 1 shape");
		configParam(OSC1PW_PARAM, 		0.f, 1.f, 0.5f, "Oscillator 1 pulse width", " %", 0.f, 100.f);
		configParam(OSC1VOL_PARAM, 		0.f, 1.f, 0.5f, "Oscillator 1 volume", 		" %", 0.f, 100.f);
		configParam(OSC2SHAPE_PARAM, 	0.f, 1.f, 0.f, 	"Oscillator 2 shape");
		configParam(OSC2PW_PARAM, 		0.f, 1.f, 0.5f, "Oscillator 2 pulse width", " %", 0.f, 100.f);
		configParam(OSC2VOL_PARAM, 		0.f, 1.f, 0.5f, "Oscillator 2 volume", 		" %", 0.f, 100.f);
		configParam(SYNC_PARAM, 		0.f, 1.f, 0.f, 	"Sync Osc 2 to Osc 1");
		configParam(CROSSMOD_PARAM, 	0.f, 1.f, 0.f, 	"Osc 1 to Osc2 crossmod");
		configParam(RINGMOD_PARAM, 		0.f, 1.f, 0.f, 	"Ringmod volume", 			" %", 0.f, 100.f);
		configInput(OSC1SHAPE_INPUT, 	"Oscillator 1 shape CV");
		configInput(OSC1PW_INPUT, 		"Oscillator 1 pulse width CV");
		configInput(OSC1VOL_INPUT, 		"Oscillator 1 volume CV");
		configInput(OSC2SHAPE_INPUT, 	"Oscillator 2 shape CV");
		configInput(OSC2PW_INPUT, 		"Oscillator 2 pulse width CV");
		configInput(OSC2VOL_INPUT, 		"Oscillator 2 volume CV");
		configInput(CROSSMOD_INPUT, 	"Crossmod CV");
		configInput(RINGMOD_INPUT, 		"Ringmod volume CV");
		configInput(OSC1VOCT_INPUT, 	"Oscillator 1 V/Oct");
		configInput(OSC2VOCT_INPUT, 	"Oscillator 2 V/Oct");
		configOutput(OUT_OUTPUT, 		"Mix");

		CvDivider.setDivision(4);
		lightDivider.setDivision(128);
	}

	void setOversamplingRate(int arg)
	{
		oversamplingRate = arg;

		// reset phasors and mix
		for (int c = 0; c < 16; c += 4) {
			phasor1[c/4] = 0.f;
			phasor2[c/4] = 0.f;
			for (int i = 0; i < maxOversamplingRate; ++i)
			{
				mix[c/4][i] = 0.f;
			}
		}
	}

	void process(const ProcessArgs& args) override {
		channels = std::max(1, inputs[OSC1VOCT_INPUT].getChannels());
		outputs[OUT_OUTPUT].setChannels(channels);

		// CV
		if (CvDivider.process()) {
			for (int c = 0; c < channels; c += 4) {
				osc1Shape[c/4] 	= simd::clamp(params[OSC1SHAPE_PARAM].getValue() + 0.1f *inputs[OSC1SHAPE_INPUT].getVoltageSimd<float_4>(c), 0.f, 1.f);
				osc1PW[c/4] 	= simd::clamp(params[OSC1PW_PARAM].getValue() 	 + 0.1f *inputs[OSC1PW_INPUT].getVoltageSimd<float_4>(c),    0.f, 1.f);
				osc1Vol[c/4] 	= simd::clamp(params[OSC1VOL_PARAM].getValue()   + 0.1f *inputs[OSC1VOL_INPUT].getVoltageSimd<float_4>(c),   0.f, 1.f);

				osc2Shape[c/4] 	= simd::clamp(params[OSC2SHAPE_PARAM].getValue() + 0.1f *inputs[OSC2SHAPE_INPUT].getVoltageSimd<float_4>(c), 0.f, 1.f);
				osc2PW[c/4] 	= simd::clamp(params[OSC2PW_PARAM].getValue() 	 + 0.1f *inputs[OSC2PW_INPUT].getVoltageSimd<float_4>(c),    0.f, 1.f);
				osc2Vol[c/4] 	= simd::clamp(params[OSC2VOL_PARAM].getValue()   + 0.1f *inputs[OSC2VOL_INPUT].getVoltageSimd<float_4>(c),   0.f, 1.f);

				crossmod[c/4] 	= simd::clamp(params[CROSSMOD_PARAM].getValue()  + 0.1f *inputs[CROSSMOD_INPUT].getVoltageSimd<float_4>(c),  0.f, 1.f);
				crossmod[c/4]   = crossmod[c/4] * crossmod[c/4] * 0.01f; // scale
				ringmod[c/4] 	= simd::clamp(params[RINGMOD_PARAM].getValue()   + 0.1f *inputs[RINGMOD_INPUT].getVoltageSimd<float_4>(c),   0.f, 1.f);
			}
		}

		for (int c = 0; c < channels; c += 4) {
			float_4 freq1 = simd::clamp(dsp::FREQ_C4 * dsp::exp2_taylor5(inputs[OSC1VOCT_INPUT].getVoltageSimd<float_4>(c)), 0.1f, 20000.f);
			int32_4 phase1Inc = INT32_MAX / args.sampleRate * freq1 / oversamplingRate;
			int32_4 phase1Offset = osc1PW[c/4] * INT32_MAX; // for pulse wave = saw + inverted saw with phaseshift

			float_4 freq2 = simd::clamp(dsp::FREQ_C4 * dsp::exp2_taylor5(inputs[OSC2VOCT_INPUT].getVoltageSimd<float_4>(c)), 0.1f, 20000.f);
			int32_4 phase2Inc = INT32_MAX / args.sampleRate * freq2 / oversamplingRate;
			int32_4 phase2Offset = osc2PW[c/4] * INT32_MAX; // for pulse wave

			for (int i = 0; i < oversamplingRate; ++i)
			{
				float_4 doSync = params[SYNC_PARAM].getValue() & (phasor1[c/4] + phase1Inc < phasor1[c/4]);

				phasor1[c/4] += phase1Inc;
				float_4 wave1 = (phasor1[c/4] + phase1Offset + phase1Offset) * osc1Shape[c/4] - 1.f * phasor1[c/4];

				int32_4 phase2IncWithCrossmod = phase2Inc + int32_4(crossmod[c/4] * wave1);
				phasor2[c/4] = simd::ifelse(doSync, -INT32_MAX, phasor2[c/4] + phase2IncWithCrossmod);
				float_4 wave2 = (phasor2[c/4] + phase2Offset + phase2Offset) * osc2Shape[c/4] - 1.f * phasor2[c/4];

				// normalize to -1..1
				wave1 /= INT32_MAX;
				wave2 /= INT32_MAX;

				mix[c/4][i] = 5.f * (osc1Vol[c/4] * wave1 + osc2Vol[c/4] * wave2 + ringmod[c/4] * wave1 * wave2);
			}

			// downsampling
			float_4 decimatedMix;
			switch (oversamplingRate)
			{
				case 1:
					decimatedMix = mix[c/4][0];
					break;
				case 4:
					decimatedMix = decimator4[c/4].process(mix[c/4]);
					break;
				case 16:
					decimatedMix = decimator16[c/4].process(mix[c/4]);
					break;
				case 64:
					decimatedMix = decimator64[c/4].process(mix[c/4]);
					break;
			}

			outputs[OUT_OUTPUT].setVoltageSimd(decimatedMix, c);
		}

		// Light
		if (lightDivider.process()) {
			lights[SYNC_LIGHT].setBrightness(params[SYNC_PARAM].getValue());
		}
	}
};


struct OscillatorsWidget : ModuleWidget {
	OscillatorsWidget(Oscillators* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/Oscillators.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(22.936, 16.062)), module, Oscillators::OSC1SHAPE_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(38.176, 16.062)), module, Oscillators::OSC1PW_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(53.491, 16.062)), module, Oscillators::OSC1VOL_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(22.936, 32.125)), module, Oscillators::OSC2SHAPE_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(38.176, 32.125)), module, Oscillators::OSC2PW_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(53.491, 32.125)), module, Oscillators::OSC2VOL_PARAM));
		addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<WhiteLight>>>(mm2px(Vec(22.936, 48.188)), module, Oscillators::SYNC_PARAM, Oscillators::SYNC_LIGHT));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(38.176, 48.188)), module, Oscillators::CROSSMOD_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(53.491, 48.188)), module, Oscillators::RINGMOD_PARAM));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(22.936, 64.25)), module, Oscillators::OSC1SHAPE_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(38.176, 64.25)), module, Oscillators::OSC1PW_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(53.491, 64.25)), module, Oscillators::OSC1VOL_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(22.936, 80.313)), module, Oscillators::OSC2SHAPE_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(38.176, 80.313)), module, Oscillators::OSC2PW_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(53.491, 80.313)), module, Oscillators::OSC2VOL_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(38.176, 96.375)), module, Oscillators::CROSSMOD_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(53.491, 96.375)), module, Oscillators::RINGMOD_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(22.936, 112.438)), module, Oscillators::OSC1VOCT_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(38.176, 112.438)), module, Oscillators::OSC2VOCT_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(53.491, 112.438)), module, Oscillators::OUT_OUTPUT));
	}

	void appendContextMenu(Menu* menu) override {
		Oscillators* module = getModule<Oscillators>();

		menu->addChild(new MenuSeparator);

		menu->addChild(createIndexSubmenuItem("Oversampling rate", {"1x", "4x", "16x", "64x"},
			[=]() {
				switch (module->oversamplingRate)
				{
					case 1:
						return 0;
					case 4:
						return 1;
					case 16:
						return 2;
					case 64:
						return 3;
					default:
						return 0;
				}
			},
			[=](int mode) {
				switch (mode)
				{
					case 0:
						module->setOversamplingRate(1);
						break;
					case 1:
						module->setOversamplingRate(4);
						break;
					case 2:
						module->setOversamplingRate(16);
						break;
					case 3:
						module->setOversamplingRate(64);
						break;
				}
			}
		));
	}
};


Model* modelOscillators = createModel<Oscillators, OscillatorsWidget>("Oscillators");

