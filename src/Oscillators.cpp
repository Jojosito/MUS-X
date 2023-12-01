#include "plugin.hpp"
#include "dsp/decimator.hpp"
#include "dsp/filters.hpp"
#include "dsp/functions.hpp"

namespace musx {

using namespace rack;
using simd::float_4;
using simd::int32_4;

struct Oscillators : Module {
	enum ParamId {
		OSC1SHAPE_PARAM,
		OSC1PW_PARAM,
		OSC1VOL_PARAM,
		OSC1SUBVOL_PARAM,
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
		OSC1SUBVOL_INPUT,
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
		SYNC_LIGHT,
		LIGHTS_LEN
	};

	static const int maxOversamplingRate = 1024;
	static const int minFreq = 0.0001f; // min frequency of the oscillators in Hz
	static const int maxFreq = 20000.f; // max frequency of the oscillators in Hz

	bool lfoMode = false;

	int sampleRate = 48000;
	int oversamplingRate = 16;
	int actualOversamplingRate = 16;

	HalfBandDecimatorCascade<float_4> decimator[4];

	int channels = 1;

	// integers overflow, so phase resets automatically
	int32_4 phasor1Sub[4] = {0};
	int32_4 phasor2[4] = {0};

	float_4 mix[4][maxOversamplingRate] = {0};

	// CV modulated parameters
	float_4 osc1Shape[4] = {0};
	float_4 osc1PW[4] = {0};
	float_4 osc1Vol[4] = {0};
	float_4 osc1Subvol[4] = {0};

	float_4 osc2Shape[4] = {0};
	float_4 osc2PW[4] = {0};
	float_4 osc2Vol[4] = {0};

	float_4 fm[4] = {0};
	float_4 ringmod[4] = {0};

	bool dcBlock = true;
	musx::TOnePole<float_4> dcBlocker[4];

	dsp::ClockDivider lightDivider;

	Oscillators() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configParam(OSC1SHAPE_PARAM,   -1.f, 1.f, 0.f,  "Oscillator 1 shape");
		configParam(OSC1PW_PARAM, 	   -1.f, 1.f, 0.f,  "Oscillator 1 triangle phase / pulse width", " %", 0.f, 50.f, 50.f);
		configParam(OSC1VOL_PARAM, 		0.f, 1.f, 0.f,  "Oscillator 1 volume", 		" %", 0.f, 100.f);
		configParam(OSC1SUBVOL_PARAM, 	0.f, 1.f, 0.f,  "Oscillator 1 sub-oscillator volume", " %", 0.f, 100.f);
		configParam(OSC2SHAPE_PARAM,   -1.f, 1.f, 0.f,  "Oscillator 2 shape");
		configParam(OSC2PW_PARAM, 	   -1.f, 1.f, 0.f,  "Oscillator 2 triangle phase / pulse width", " %", 0.f, 50.f, 50.f);
		configParam(OSC2VOL_PARAM, 		0.f, 1.f, 0.5f, "Oscillator 2 volume", 		" %", 0.f, 100.f);
		configSwitch(SYNC_PARAM, 		  0,   1,   0,  "Sync", {"Off", "Sync osc 2 to osc 1"});
		configParam(FM_PARAM, 	  		0.f, 1.f, 0.f, 	"Osc 1 to osc 2 FM amount", " %", 0.f, 100.f);
		configParam(RINGMOD_PARAM, 		0.f, 1.f, 0.f, 	"Ring modulator volume", 	" %", 0.f, 100.f);
		configInput(OSC1SHAPE_INPUT, 	"Oscillator 1 shape CV");
		configInput(OSC1PW_INPUT, 		"Oscillator 1 pulse width CV");
		configInput(OSC1VOL_INPUT, 		"Oscillator 1 volume CV");
		configInput(OSC1SUBVOL_INPUT, 	"Oscillator 1 sub-oscillator volume CV");
		configInput(OSC2SHAPE_INPUT, 	"Oscillator 2 shape CV");
		configInput(OSC2PW_INPUT, 		"Oscillator 2 pulse width CV");
		configInput(OSC2VOL_INPUT, 		"Oscillator 2 volume CV");
		configInput(SYNC_INPUT, 		"Sync CV");
		configInput(FM_INPUT, 			"FM amount CV");
		configInput(RINGMOD_INPUT, 		"Ring modulator volume CV");
		configInput(OSC1VOCT_INPUT, 	"Oscillator 1 V/Oct");
		configInput(OSC2VOCT_INPUT, 	"Oscillator 2 V/Oct");
		configOutput(OUT_OUTPUT, 		"Mix");

		lightDivider.setDivision(512);
	}

	void onSampleRateChange(const SampleRateChangeEvent& e) override {
		sampleRate = e.sampleRate;
		setOversamplingRate(oversamplingRate);
	}

	void setOversamplingRate(int arg)
	{
		oversamplingRate = arg;

		// reset phasors and mix
		for (int c = 0; c < 16; c += 4) {
			phasor1Sub[c/4] = 0.f;
			phasor2[c/4] = 0.f;

			for (int i = 0; i < maxOversamplingRate; ++i)
			{
				mix[c/4][i] = 0.f;
			}

			decimator[c/4].reset();
			dcBlocker[c/4].setCutoffFreq(20.f/sampleRate/oversamplingRate);
		}
	}

	void process(const ProcessArgs& args) override {
		channels = std::max(1, inputs[OSC1VOCT_INPUT].getChannels());
		outputs[OUT_OUTPUT].setChannels(channels);

		for (int c = 0; c < channels; c += 4) {
			// parameters and CVs
			osc1Shape[c/4] 	= simd::clamp(params[OSC1SHAPE_PARAM].getValue() + 0.2f *inputs[OSC1SHAPE_INPUT].getPolyVoltageSimd<float_4>(c), -1.f, 1.f);
			osc1PW[c/4] 	= simd::clamp(params[OSC1PW_PARAM].getValue() 	 + 0.2f *inputs[OSC1PW_INPUT].getPolyVoltageSimd<float_4>(c),    -1.f, 1.f);
			osc1Vol[c/4] 	= simd::clamp(params[OSC1VOL_PARAM].getValue()   + 0.1f *inputs[OSC1VOL_INPUT].getPolyVoltageSimd<float_4>(c),    0.f, 1.f);
			osc1Vol[c/4]   *= 10.f / INT32_MAX;
			osc1Subvol[c/4] = simd::clamp(params[OSC1SUBVOL_PARAM].getValue() + 0.1f *inputs[OSC1SUBVOL_INPUT].getPolyVoltageSimd<float_4>(c), 0.f, 1.f);
			osc1Subvol[c/4]   *= 10.f / INT32_MAX;

			osc2Shape[c/4] 	= simd::clamp(params[OSC2SHAPE_PARAM].getValue() + 0.2f *inputs[OSC2SHAPE_INPUT].getPolyVoltageSimd<float_4>(c), -1.f, 1.f);
			osc2PW[c/4] 	= simd::clamp(params[OSC2PW_PARAM].getValue() 	 + 0.2f *inputs[OSC2PW_INPUT].getPolyVoltageSimd<float_4>(c),    -1.f, 1.f);
			osc2Vol[c/4] 	= simd::clamp(params[OSC2VOL_PARAM].getValue()   + 0.1f *inputs[OSC2VOL_INPUT].getPolyVoltageSimd<float_4>(c),    0.f, 1.f);
			osc2Vol[c/4]   *= 10.f / INT32_MAX;

			fm[c/4] 		= simd::clamp(params[FM_INPUT].getValue()  + 0.1f *inputs[FM_INPUT].getPolyVoltageSimd<float_4>(c),  0.f, 1.f);
			fm[c/4] 		= fm[c/4] * fm[c/4] * 0.5f / oversamplingRate; // scale
			ringmod[c/4]  	= simd::clamp(params[RINGMOD_PARAM].getValue() + 0.1f *inputs[RINGMOD_INPUT].getPolyVoltageSimd<float_4>(c), 0.f, 1.f);
			ringmod[c/4]   *= 10.f / INT32_MAX / INT32_MAX;

			int32_4 sync = simd::round(clamp(params[SYNC_PARAM].getValue() + inputs[SYNC_INPUT].getPolyVoltageSimd<float_4>(c) / 5.f, 0.f, 1.f));

			// frequencies, phase increments, factors etc
			float_4 freq1 = dsp::FREQ_C4 * dsp::exp2_taylor5(inputs[OSC1VOCT_INPUT].getVoltageSimd<float_4>(c));
			float_4 freq2 = dsp::FREQ_C4 * dsp::exp2_taylor5(inputs[OSC2VOCT_INPUT].getPolyVoltageSimd<float_4>(c));

			actualOversamplingRate = oversamplingRate;
			if (lfoMode)
			{
				// bring frequency down 7 octaves, ca. 2 Hz @ 0V CV/Oct input
				freq1 /= 128;
				freq2 /= 128;

				actualOversamplingRate = 1;
			}

			freq1 = simd::clamp(freq1, minFreq, maxFreq);
			freq2 = simd::clamp(freq2, minFreq, maxFreq);


			int32_4 phase1SubInc = INT32_MAX / args.sampleRate * freq1 / actualOversamplingRate;
			int32_4 phase1Inc = phase1SubInc + phase1SubInc;
			float_4 tri1Amt = 2.f * simd::fmax(-osc1Shape[c/4], 0.f);  // [2, 0, 0]
			float_4 sawSq1Amt = simd::fmin(1.f + osc1Shape[c/4], 1.f); // [0, 1, 1]
			float_4 sq1Amt = simd::fmax(osc1Shape[c/4], 0.f);          // [0, 0, 1]
			int32_4 phase1Offset = simd::ifelse(osc1PW[c/4] < 0, (-1.f - osc1PW[c/4]) * INT32_MAX, (1.f - osc1PW[c/4]) * INT32_MAX); // for pulse wave = saw + inverted saw with phaseshift

			int32_4 phase2Inc = INT32_MAX / args.sampleRate * freq2 / actualOversamplingRate * 2;
			float_4 tri2Amt = 2.f * simd::fmax(-osc2Shape[c/4], 0.f);
			float_4 sawSq2Amt = simd::fmin(1.f + osc2Shape[c/4], 1.f);
			float_4 sq2Amt = simd::fmax(osc2Shape[c/4], 0.f);
			int32_4 phase2Offset = simd::ifelse(osc2PW[c/4] < 0, (-1.f - osc2PW[c/4]) * INT32_MAX, (1.f - osc2PW[c/4]) * INT32_MAX); // for pulse wave

			// calculate the oversampled oscillators and mix
			float_4* inBuffer = decimator[c/4].getInputArray(actualOversamplingRate);

			bool calcDcBlock = dcBlock && !lfoMode;

			for (int i = 0; i < actualOversamplingRate; ++i)
			{
				// phasors for subosc 1 and osc 1
				phasor1Sub[c/4] += phase1SubInc;
				int32_4 phasor1 = phasor1Sub[c/4] + phasor1Sub[c/4];
				int32_4 phasor1Offset = phasor1 + phase1Offset;

				// osc 1 waveform
				float_4 wave1 = tri1Amt * ((1.f*phasor1Offset + (phasor1Offset > 0) * 2.f * phasor1Offset) + INT32_MAX/2); // +-INT32_MAX
				wave1 += sawSq1Amt * (phasor1Offset * sq1Amt - 1.f * phasor1); // +-INT32_MAX

				// osc 1 suboscillator
				float_4 sub1 = 1.f * (phasor1Sub[c/4] + INT32_MAX) - 1.f * phasor1Sub[c/4]; // +-INT32_MAX

				// phasor for osc 2
				phasor2[c/4] += phase2Inc + int32_4(fm[c/4] * wave1);

				// sync / reset phasor2 ?
				phasor2[c/4] -= (sync & (phasor1 + phase1Inc < phasor1)) * (phasor2[c/4] + INT32_MAX);
				int32_4 phasor2Offset = phasor2[c/4] + phase2Offset;

				// osc 2 waveform
				float_4 wave2 = tri2Amt * ((1.f*phasor2Offset + (phasor2Offset > 0) * 2.f * phasor2Offset) + INT32_MAX/2); // +-INT32_MAX
				wave2 += sawSq2Amt * (phasor2Offset * sq2Amt - 1.f * phasor2[c/4]); // +-INT32_MAX

				// mix
				float_4 out = osc1Subvol[c/4] * sub1 + osc1Vol[c/4] * wave1 + osc2Vol[c/4] * wave2 + ringmod[c/4] * wave1 * wave2; // +-5V each

				// DC blocker
				if (calcDcBlock)
				{
					dcBlocker[c/4].process(out);
					out = dcBlocker[c/4].highpass();
				}

				// saturator +-10V
				out = musx::cheapSaturator(out);

				inBuffer[i] = out;
			}

			// downsampling
			float_4 out = decimator[c/4].process(actualOversamplingRate);

			outputs[OUT_OUTPUT].setVoltageSimd(out, c);
		}

		// Light
		if (lightDivider.process()) {
			lights[SYNC_LIGHT].setBrightness(params[SYNC_PARAM].getValue());
		}
	}

	json_t* dataToJson() override {
		json_t* rootJ = json_object();
		json_object_set_new(rootJ, "oversamplingRate", json_integer(oversamplingRate));
		json_object_set_new(rootJ, "dcBlock", json_boolean(dcBlock));
		json_object_set_new(rootJ, "lfoMode", json_boolean(lfoMode));
		return rootJ;
	}

	void dataFromJson(json_t* rootJ) override {
		json_t* oversamplingRateJ = json_object_get(rootJ, "oversamplingRate");
		if (oversamplingRateJ)
		{
			setOversamplingRate(json_integer_value(oversamplingRateJ));
		}
		json_t* dcBlockJ = json_object_get(rootJ, "dcBlock");
		if (dcBlockJ)
		{
			dcBlock = (json_boolean_value(dcBlockJ));
		}
		json_t* lfoModeJ = json_object_get(rootJ, "lfoMode");
		if (lfoModeJ)
		{
			lfoMode = (json_boolean_value(lfoModeJ));
		}
	}
};


struct OscillatorsWidget : ModuleWidget {
	OscillatorsWidget(Oscillators* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/Oscillators.svg"), asset::plugin(pluginInstance, "res/Oscillators-dark.svg")));

		addChild(createWidget<ThemedScrew>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ThemedScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ThemedScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ThemedScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(8.361, 18.179)), module, Oscillators::OSC1SHAPE_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(23.601, 18.179)), module, Oscillators::OSC1PW_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(37.253, 18.179)), module, Oscillators::OSC1VOL_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(48.683, 18.179)), module, Oscillators::OSC1SUBVOL_PARAM));
		addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<WhiteLight>>>(mm2px(Vec(10.497, 50.304)), module, Oscillators::SYNC_PARAM, Oscillators::SYNC_LIGHT));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(25.737, 50.304)), module, Oscillators::FM_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(42.641, 50.304)), module, Oscillators::RINGMOD_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(12.614, 83.488)), module, Oscillators::OSC2SHAPE_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(28.912, 83.488)), module, Oscillators::OSC2PW_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(45.816, 83.488)), module, Oscillators::OSC2VOL_PARAM));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(8.361, 28.888)), module, Oscillators::OSC1SHAPE_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(23.601, 28.888)), module, Oscillators::OSC1PW_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(37.253, 28.888)), module, Oscillators::OSC1VOL_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(48.683, 28.888)), module, Oscillators::OSC1SUBVOL_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10.478, 61.012)), module, Oscillators::SYNC_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(25.717, 61.012)), module, Oscillators::FM_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(42.545, 61.012)), module, Oscillators::RINGMOD_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(12.594, 94.196)), module, Oscillators::OSC2SHAPE_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(28.892, 94.196)), module, Oscillators::OSC2PW_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(45.72, 94.196)), module, Oscillators::OSC2VOL_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(12.7, 112.438)), module, Oscillators::OSC1VOCT_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(27.94, 112.438)), module, Oscillators::OSC2VOCT_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(43.256, 112.438)), module, Oscillators::OUT_OUTPUT));
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

		menu->addChild(createBoolMenuItem("DC blocker", "",
			[=]() {
				return module->dcBlock;
			},
			[=](int mode) {
				module->dcBlock = mode;
			}
		));

		menu->addChild(new MenuSeparator);

		menu->addChild(createBoolMenuItem("LFO mode", "",
			[=]() {
				return module->lfoMode;
			},
			[=](int mode) {
				module->lfoMode = mode;
			}
		));
	}
};


Model* modelOscillators = createModel<Oscillators, OscillatorsWidget>("Oscillators");

}
