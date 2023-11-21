#include "plugin.hpp"
#include <decimator.hpp>

namespace musx {

using namespace rack;
using simd::float_4;
using simd::int32_4;

struct OscillatorsDev : Module {
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
		SYNC_LIGHT,
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

	dsp::ClockDivider lightDivider;

	OscillatorsDev() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configParam(OSC1SHAPE_PARAM, 	-1.f, 1.f, 0.f, 	"Oscillator 1 shape");
		configParam(OSC1PW_PARAM, 		-1.f, 1.f, 0.f, "Oscillator 1 pulse width", " %", 0.f, 50.f, 50.f);
		configParam(OSC1VOL_PARAM, 		0.f, 1.f, 0.f,  "Oscillator 1 volume", 		" %", 0.f, 100.f);
		configParam(OSC2SHAPE_PARAM, 	-1.f, 1.f, 0.f, 	"Oscillator 2 shape");
		configParam(OSC2PW_PARAM, 		-1.f, 1.f, 0.f, "Oscillator 2 pulse width", " %", 0.f, 50.f, 50.f);
		configParam(OSC2VOL_PARAM, 		0.f, 1.f, 0.5f, "Oscillator 2 volume", 		" %", 0.f, 100.f);
		configSwitch(SYNC_PARAM, 		  0,   1,   0, 	"Sync", {"Off", "Sync osc 2 to osc 1"});
		configParam(FM_PARAM, 	  		0.f, 1.f, 0.f, 	"Osc 1 to osc 2 FM amount", " %", 0.f, 100.f);
		configParam(RINGMOD_PARAM, 		0.f, 1.f, 0.f, 	"Ring modulator volume", 	" %", 0.f, 100.f);
		configInput(OSC1SHAPE_INPUT, 	"Oscillator 1 shape CV");
		configInput(OSC1PW_INPUT, 		"Oscillator 1 pulse width CV");
		configInput(OSC1VOL_INPUT, 		"Oscillator 1 volume CV");
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
			osc1Shape[c/4] 	= simd::clamp(params[OSC1SHAPE_PARAM].getValue() + 0.2f *inputs[OSC1SHAPE_INPUT].getPolyVoltageSimd<float_4>(c), -1.f, 1.f);
			osc1PW[c/4] 	= simd::clamp(params[OSC1PW_PARAM].getValue() 	 + 0.2f *inputs[OSC1PW_INPUT].getPolyVoltageSimd<float_4>(c),    -1.f, 1.f);
			osc1Vol[c/4] 	= simd::clamp(params[OSC1VOL_PARAM].getValue()   + 0.1f *inputs[OSC1VOL_INPUT].getPolyVoltageSimd<float_4>(c),    0.f, 1.f);
			osc1Vol[c/4]   *= 5.f / INT32_MAX;

			osc2Shape[c/4] 	= simd::clamp(params[OSC2SHAPE_PARAM].getValue() + 0.2f *inputs[OSC2SHAPE_INPUT].getPolyVoltageSimd<float_4>(c), -1.f, 1.f);
			osc2PW[c/4] 	= simd::clamp(params[OSC2PW_PARAM].getValue() 	 + 0.2f *inputs[OSC2PW_INPUT].getPolyVoltageSimd<float_4>(c),    -1.f, 1.f);
			osc2Vol[c/4] 	= simd::clamp(params[OSC2VOL_PARAM].getValue()   + 0.1f *inputs[OSC2VOL_INPUT].getPolyVoltageSimd<float_4>(c),    0.f, 1.f);
			osc2Vol[c/4]   *= 5.f / INT32_MAX;

			fm[c/4] 		= simd::clamp(params[FM_INPUT].getValue()  + 0.1f *inputs[FM_INPUT].getPolyVoltageSimd<float_4>(c),  0.f, 1.f);
			fm[c/4] 		= fm[c/4] * fm[c/4] * 0.5f / oversamplingRate; // scale
			ringmod[c/4]  	= simd::clamp(params[RINGMOD_PARAM].getValue() + 0.1f *inputs[RINGMOD_INPUT].getPolyVoltageSimd<float_4>(c), 0.f, 1.f);
			ringmod[c/4]   *= 5.f / INT32_MAX / INT32_MAX;

			int32_4 sync = simd::round(clamp(params[SYNC_PARAM].getValue() + inputs[SYNC_INPUT].getPolyVoltageSimd<float_4>(c) / 5.f, 0.f, 1.f));

			// frequencies, phase increments, factors etc
			float_4 freq1 = simd::clamp(dsp::FREQ_C4 * dsp::exp2_taylor5(inputs[OSC1VOCT_INPUT].getVoltageSimd<float_4>(c)), minFreq, maxFreq);
			int32_4 phase1Inc = INT32_MAX / args.sampleRate * freq1 / oversamplingRate * 2;
			float_4 tri1Amt = 2.f * simd::fmax(-osc1Shape[c/4], 0.f);  // [2, 0, 0]
			float_4 sawSq1Amt = simd::fmin(1.f + osc1Shape[c/4], 1.f); // [0, 1, 1]
			float_4 sq1Amt = simd::fmax(osc1Shape[c/4], 0.f);          // [0, 0, 1]
			int32_4 phase1Offset = simd::ifelse(osc1PW[c/4] < 0, (-1.f - osc1PW[c/4]) * INT32_MAX, (1.f - osc1PW[c/4]) * INT32_MAX); // for pulse wave = saw + inverted saw with phaseshift

			float_4 freq2 = simd::clamp(dsp::FREQ_C4 * dsp::exp2_taylor5(inputs[OSC2VOCT_INPUT].getPolyVoltageSimd<float_4>(c)), minFreq, maxFreq);
			int32_4 phase2Inc = INT32_MAX / args.sampleRate * freq2 / oversamplingRate * 2;
			float_4 tri2Amt = 2.f * simd::fmax(-osc2Shape[c/4], 0.f);
			float_4 sawSq2Amt = simd::fmin(1.f + osc2Shape[c/4], 1.f);
			float_4 sq2Amt = simd::fmax(osc2Shape[c/4], 0.f);
			int32_4 phase2Offset = simd::ifelse(osc2PW[c/4] < 0, (-1.f - osc2PW[c/4]) * INT32_MAX, (1.f - osc2PW[c/4]) * INT32_MAX); // for pulse wave

			// calculate the oversampled oscillators and mix
			float_4* inBuffer = decimator[c/4].getInputArray(oversamplingRate);

			for (int i = 0; i < oversamplingRate; ++i)
			{
				float_4 doSync = sync & (phasor1[c/4] + phase1Inc < phasor1[c/4]); // reset phasor2 ?

				phasor1[c/4] += phase1Inc;
				int32_4 phasor1Offset = phasor1[c/4] + phase1Offset;
				float_4 tri1 = (1.f*phasor1Offset + (phasor1Offset > 0) * 2.f * phasor1Offset) + INT32_MAX/2; // +-INT32_MAX/2
				float_4 sawSq1 = phasor1Offset * sq1Amt - 1.f * phasor1[c/4]; // +-INT32_MAX
				float_4 wave1 = tri1Amt * tri1 + sawSq1Amt * sawSq1; // +-INT32_MAX

				int32_4 phase2IncWithFm = phase2Inc + int32_4(fm[c/4] * wave1);
				int32_4 phasor2Offset = phasor2[c/4] + phase2Offset;
				phasor2[c/4] += phase2IncWithFm;
				phasor2[c/4] -= doSync * (phasor2[c/4] + INT32_MAX);
				float_4 tri2 = (1.f*phasor2Offset + (phasor2Offset > 0) * 2.f * phasor2Offset) + INT32_MAX/2; // +-INT32_MAX/2
				float_4 sawSq2 = phasor2Offset * sq2Amt - 1.f * phasor2[c/4]; // +-INT32_MAX
				float_4 wave2 = tri2Amt * tri2 + sawSq2Amt * sawSq2; // +-INT32_MAX

				inBuffer[i] = osc1Vol[c/4] * wave1 + osc2Vol[c/4] * wave2 + ringmod[c/4] * wave1 * wave2; // +-5V each
			}

			// downsampling
			outputs[OUT_OUTPUT].setVoltageSimd(decimator[c/4].process(oversamplingRate), c);
		}

		// Light
		if (lightDivider.process()) {
			lights[SYNC_LIGHT].setBrightness(params[SYNC_PARAM].getValue());
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


struct OscillatorsDevWidget : ModuleWidget {
	OscillatorsDevWidget(OscillatorsDev* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/Oscillators.svg"), asset::plugin(pluginInstance, "res/Oscillators-dark.svg")));

		addChild(createWidget<ThemedScrew>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ThemedScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ThemedScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ThemedScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(15.24, 18.179)), module, OscillatorsDev::OSC1SHAPE_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(30.48, 18.179)), module, OscillatorsDev::OSC1PW_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(45.72, 18.179)), module, OscillatorsDev::OSC1VOL_PARAM));
		addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<WhiteLight>>>(mm2px(Vec(15.26, 50.304)), module, OscillatorsDev::SYNC_PARAM, OscillatorsDev::SYNC_LIGHT));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(30.5, 50.304)), module, OscillatorsDev::FM_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(45.816, 50.304)), module, OscillatorsDev::RINGMOD_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(15.26, 82.429)), module, OscillatorsDev::OSC2SHAPE_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(30.5, 82.429)), module, OscillatorsDev::OSC2PW_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(45.816, 82.429)), module, OscillatorsDev::OSC2VOL_PARAM));

		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(15.24, 28.888)), module, OscillatorsDev::OSC1SHAPE_INPUT));
		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(30.48, 28.888)), module, OscillatorsDev::OSC1PW_INPUT));
		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(45.72, 28.888)), module, OscillatorsDev::OSC1VOL_INPUT));
		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(15.24, 61.012)), module, OscillatorsDev::SYNC_INPUT));
		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(30.48, 61.012)), module, OscillatorsDev::FM_INPUT));
		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(45.72, 61.012)), module, OscillatorsDev::RINGMOD_INPUT));
		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(15.24, 93.137)), module, OscillatorsDev::OSC2SHAPE_INPUT));
		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(30.48, 93.137)), module, OscillatorsDev::OSC2PW_INPUT));
		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(45.72, 93.137)), module, OscillatorsDev::OSC2VOL_INPUT));
		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(15.26, 112.438)), module, OscillatorsDev::OSC1VOCT_INPUT));
		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(30.5, 112.438)), module, OscillatorsDev::OSC2VOCT_INPUT));

		addOutput(createOutputCentered<ThemedPJ301MPort>(mm2px(Vec(45.816, 112.438)), module, OscillatorsDev::OUT_OUTPUT));
	}

	void appendContextMenu(Menu* menu) override {
		OscillatorsDev* module = getModule<OscillatorsDev>();

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


Model* modelOscillatorsDev = createModel<OscillatorsDev, OscillatorsDevWidget>("OscillatorsDev");

}
