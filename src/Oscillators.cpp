#include "plugin.hpp"
#include "decimator.hpp"

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
		FM_PARAM,
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
		SYNC_INPUT,
		FM_INPUT,
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
		LIGHTS_LEN
	};

	static const int maxOversamplingRate = 1024;
	static const int minFreq = 0.01f; // min frequency of the oscillators in Hz
	static const int maxFreq = 20000.f; // max frequency of the oscillators in Hz

	int oversamplingRate = 16;

	HalfBandDecimatorCascade<float_4> decimator[4];

	int channels = 1;

	// integers overflow, so phase resets automatically
	int32_4 phasor1[4] = {0};
	int32_4 phasor2[4] = {0};

	float_4 mix[4][maxOversamplingRate] = {0};

	// CV modulated parameters
	float_4 osc1Shape[4] = {0};
	float_4 osc1PW[4] = {0};
	float_4 osc1Vol[4] = {0};

	float_4 osc2Shape[4] = {0};
	float_4 osc2PW[4] = {0};
	float_4 osc2Vol[4] = {0};

	float_4 fm[4] = {0};
	float_4 ringmod[4] = {0};

	Oscillators() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configParam(OSC1SHAPE_PARAM, 	0.f, 1.f, 0.f, 	"Oscillator 1 shape");
		configParam(OSC1PW_PARAM, 		0.f, 1.f, 0.5f, "Oscillator 1 pulse width", " %", 0.f, 100.f);
		configParam(OSC1VOL_PARAM, 		0.f, 1.f, 0.f,  "Oscillator 1 volume", 		" %", 0.f, 100.f);
		configParam(OSC2SHAPE_PARAM, 	0.f, 1.f, 0.f, 	"Oscillator 2 shape");
		configParam(OSC2PW_PARAM, 		0.f, 1.f, 0.5f, "Oscillator 2 pulse width", " %", 0.f, 100.f);
		configParam(OSC2VOL_PARAM, 		0.f, 1.f, 0.5f, "Oscillator 2 volume", 		" %", 0.f, 100.f);
		configSwitch(SYNC_PARAM, 		-1,  1,   0, 	"Sync", {"Sync osc 2 to osc 1", "off", "Sync osc 1 to osc 2"});
		configParam(FM_PARAM, 	  		0.f, 1.f, 0.f, 	"Osc 1 to osc 2 FM amount", " %", 0.f, 100.f);
		configParam(RINGMOD_PARAM, 		0.f, 1.f, 0.f, 	"Ring modulator volume", 	" %", 0.f, 100.f);
		configInput(OSC1SHAPE_INPUT, 	"Oscillator 1 shape CV");
		configInput(OSC1PW_INPUT, 		"Oscillator 1 pulse width CV");
		configInput(OSC1VOL_INPUT, 		"Oscillator 1 volume CV");
		configInput(OSC2SHAPE_INPUT, 	"Oscillator 2 shape CV");
		configInput(OSC2PW_INPUT, 		"Oscillator 2 pulse width CV");
		configInput(OSC2VOL_INPUT, 		"Oscillator 2 volume CV");
		configInput(SYNC_INPUT, 		"Sync CV (mono)");
		configInput(FM_INPUT, 	"FM amount CV");
		configInput(RINGMOD_INPUT, 		"Ring modulator volume CV");
		configInput(OSC1VOCT_INPUT, 	"Oscillator 1 V/Oct");
		configInput(OSC2VOCT_INPUT, 	"Oscillator 2 V/Oct");
		configOutput(OUT_OUTPUT, 		"Mix");
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

			decimator[c/4].reset();
		}
	}

	void process(const ProcessArgs& args) override {
		channels = std::max(1, inputs[OSC1VOCT_INPUT].getChannels());
		outputs[OUT_OUTPUT].setChannels(channels);

		for (int c = 0; c < channels; c += 4) {
			// parameters and CVs
			osc1Shape[c/4] 	= simd::clamp(params[OSC1SHAPE_PARAM].getValue() + 0.1f *inputs[OSC1SHAPE_INPUT].getVoltageSimd<float_4>(c), 0.f, 1.f);
			osc1PW[c/4] 	= simd::clamp(params[OSC1PW_PARAM].getValue() 	 + 0.1f *inputs[OSC1PW_INPUT].getVoltageSimd<float_4>(c),    0.f, 1.f);
			osc1Vol[c/4] 	= simd::clamp(params[OSC1VOL_PARAM].getValue()   + 0.1f *inputs[OSC1VOL_INPUT].getVoltageSimd<float_4>(c),   0.f, 1.f);
			osc1Vol[c/4] *= 5.f / INT32_MAX;

			osc2Shape[c/4] 	= simd::clamp(params[OSC2SHAPE_PARAM].getValue() + 0.1f *inputs[OSC2SHAPE_INPUT].getVoltageSimd<float_4>(c), 0.f, 1.f);
			osc2PW[c/4] 	= simd::clamp(params[OSC2PW_PARAM].getValue() 	 + 0.1f *inputs[OSC2PW_INPUT].getVoltageSimd<float_4>(c),    0.f, 1.f);
			osc2Vol[c/4] 	= simd::clamp(params[OSC2VOL_PARAM].getValue()   + 0.1f *inputs[OSC2VOL_INPUT].getVoltageSimd<float_4>(c),   0.f, 1.f);
			osc2Vol[c/4] *= 5.f / INT32_MAX;

			fm[c/4] 	= simd::clamp(params[FM_INPUT].getValue()  + 0.1f *inputs[FM_INPUT].getVoltageSimd<float_4>(c),  0.f, 1.f);
			fm[c/4]   = fm[c/4] * fm[c/4] * 0.1f / oversamplingRate; // scale
			ringmod[c/4] 	= simd::clamp(params[RINGMOD_PARAM].getValue()   + 0.1f *inputs[RINGMOD_INPUT].getVoltageSimd<float_4>(c),   0.f, 1.f);
			ringmod[c/4] *= 5.f / INT32_MAX / INT32_MAX;

			int sync = std::round(clamp(params[SYNC_PARAM].getValue() + inputs[SYNC_INPUT].getVoltage() / 5.f, -1.f, 1.f));

			// frequencies and phase increments
			float_4 freq1 = simd::clamp(dsp::FREQ_C4 * dsp::exp2_taylor5(inputs[OSC1VOCT_INPUT].getVoltageSimd<float_4>(c)), minFreq, maxFreq);
			int32_4 phase1Inc = INT32_MAX / args.sampleRate * freq1 / oversamplingRate * 2;
			int32_4 phase1Offset = osc1PW[c/4] * INT32_MAX; // for pulse wave = saw + inverted saw with phaseshift

			float_4 freq2 = simd::clamp(dsp::FREQ_C4 * dsp::exp2_taylor5(inputs[OSC2VOCT_INPUT].getVoltageSimd<float_4>(c)), minFreq, maxFreq);
			int32_4 phase2Inc = INT32_MAX / args.sampleRate * freq2 / oversamplingRate * 2;
			int32_4 phase2Offset = osc2PW[c/4] * INT32_MAX; // for pulse wave

			// calculate the oversampled oscillators and mix
			switch (sync)
			{
				case -1: // sync osc 2 to osc 1
					for (int i = 0; i < oversamplingRate; ++i)
					{
						float_4 doSync = phasor1[c/4] + phase1Inc < phasor1[c/4];

						phasor1[c/4] += phase1Inc;
						float_4 wave1 = (phasor1[c/4] - phase1Offset - phase1Offset) * osc1Shape[c/4] - 1.f * phasor1[c/4]; // +-INT32_MAX

						int32_4 phase2IncWithFm = phase2Inc + int32_4(fm[c/4] * wave1);
						phasor2[c/4] = simd::ifelse(doSync, -INT32_MAX, phasor2[c/4] + phase2IncWithFm);
						float_4 wave2 = (phasor2[c/4] - phase2Offset - phase2Offset) * osc2Shape[c/4] - 1.f * phasor2[c/4]; // +-INT32_MAX

						mix[c/4][i] = osc1Vol[c/4] * wave1 + osc2Vol[c/4] * wave2 + ringmod[c/4] * wave1 * wave2; // +-5V each
					}
					break;
				case 1: // sync osc 1 to osc 2
					for (int i = 0; i < oversamplingRate; ++i)
					{
						float_4 doSync = phasor2[c/4] + phase2Inc < phasor2[c/4];

						phasor1[c/4] = simd::ifelse(doSync, -INT32_MAX, phasor1[c/4] + phase1Inc);
						float_4 wave1 = (phasor1[c/4] - phase1Offset - phase1Offset) * osc1Shape[c/4] - 1.f * phasor1[c/4]; // +-INT32_MAX

						int32_4 phase2IncWithFm = phase2Inc + int32_4(fm[c/4] * wave1);
						phasor2[c/4] += phase2IncWithFm;
						float_4 wave2 = (phasor2[c/4] - phase2Offset - phase2Offset) * osc2Shape[c/4] - 1.f * phasor2[c/4]; // +-INT32_MAX

						mix[c/4][i] = osc1Vol[c/4] * wave1 + osc2Vol[c/4] * wave2 + ringmod[c/4] * wave1 * wave2; // +-5V each
					}
					break;
				default: // sync is off
					for (int i = 0; i < oversamplingRate; ++i)
					{
						phasor1[c/4] += phase1Inc;
						float_4 wave1 = (phasor1[c/4] - phase1Offset - phase1Offset) * osc1Shape[c/4] - 1.f * phasor1[c/4]; // +-INT32_MAX

						int32_4 phase2IncWithFm = phase2Inc + int32_4(fm[c/4] * wave1);
						phasor2[c/4] += phase2IncWithFm;
						float_4 wave2 = (phasor2[c/4] - phase2Offset - phase2Offset) * osc2Shape[c/4] - 1.f * phasor2[c/4]; // +-INT32_MAX

						mix[c/4][i] = osc1Vol[c/4] * wave1 + osc2Vol[c/4] * wave2 + ringmod[c/4] * wave1 * wave2; // +-5V each
					}
			}

			// downsampling
			outputs[OUT_OUTPUT].setVoltageSimd(decimator[c/4].process(mix[c/4], oversamplingRate), c);
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
			setOversamplingRate(json_integer_value(oversamplingRateJ));
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

		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(15.24, 18.179)), module, Oscillators::OSC1SHAPE_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(30.48, 18.179)), module, Oscillators::OSC1PW_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(45.72, 18.179)), module, Oscillators::OSC1VOL_PARAM));
		addParam(createParamCentered<BefacoSwitch>  (mm2px(Vec(15.26, 50.304)), module, Oscillators::SYNC_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(30.5, 50.304)), module, Oscillators::FM_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(45.816, 50.304)), module, Oscillators::RINGMOD_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(15.26, 82.429)), module, Oscillators::OSC2SHAPE_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(30.5, 82.429)), module, Oscillators::OSC2PW_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(45.816, 82.429)), module, Oscillators::OSC2VOL_PARAM));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(15.24, 28.888)), module, Oscillators::OSC1SHAPE_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(30.48, 28.888)), module, Oscillators::OSC1PW_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(45.72, 28.888)), module, Oscillators::OSC1VOL_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(15.24, 61.012)), module, Oscillators::SYNC_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(30.48, 61.012)), module, Oscillators::FM_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(45.72, 61.012)), module, Oscillators::RINGMOD_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(15.24, 93.137)), module, Oscillators::OSC2SHAPE_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(30.48, 93.137)), module, Oscillators::OSC2PW_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(45.72, 93.137)), module, Oscillators::OSC2VOL_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(15.26, 112.438)), module, Oscillators::OSC1VOCT_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(30.5, 112.438)), module, Oscillators::OSC2VOCT_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(45.816, 112.438)), module, Oscillators::OUT_OUTPUT));
	}

	void appendContextMenu(Menu* menu) override {
		Oscillators* module = getModule<Oscillators>();

		menu->addChild(new MenuSeparator);

		menu->addChild(createIndexSubmenuItem("Oversampling rate", {"1x", "2x", "4x", "8x", "16x", "32x", "64x", "128x", "256x", "512x", "1024x"},
			[=]() {
				return log2(module->oversamplingRate);
			},
			[=](int mode) {
				module->setOversamplingRate(std::pow(2, mode));
			}
		));
	}
};


Model* modelOscillators = createModel<Oscillators, OscillatorsWidget>("Oscillators");

