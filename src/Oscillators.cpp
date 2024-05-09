#include "plugin.hpp"
#include "blocks/OscillatorsBlock.hpp"
#include "dsp/decimator.hpp"
#include "dsp/filters.hpp"

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

	static const size_t maxOversamplingRate = 1024;

	OscillatorsBlock<maxOversamplingRate> oscBlock;

	bool lfoMode = false;

	size_t sampleRate = 48000;
	size_t oversamplingRate = 8;
	size_t actualOversamplingRate = oversamplingRate;

	HalfBandDecimatorCascade<float_4> decimator[4];

	int channels = 1;

	bool dcBlock = true;
	musx::TOnePole<float_4> dcBlocker[4];

	bool antiAliasing = true;

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
		oscBlock.setSampleRate(sampleRate);
		setOversamplingRate(oversamplingRate);
	}

	void setOversamplingRate(size_t arg)
	{
		oversamplingRate = arg;

		for (int c = 0; c < 16; c += 4) {
			decimator[c/4].reset();
			dcBlocker[c/4].setCutoffFreq(20.f/sampleRate/oversamplingRate);
		}
	}

	void process(const ProcessArgs& args) override {
		channels = std::max(1, inputs[OSC1VOCT_INPUT].getChannels());
		channels = std::max(channels, inputs[OSC2VOCT_INPUT].getChannels());
		outputs[OUT_OUTPUT].setChannels(channels);

		actualOversamplingRate = lfoMode? 1 : oversamplingRate;
		oscBlock.setOversamplingRate(actualOversamplingRate);

		for (int c = 0; c < channels; c += 4) {

			// parameters and CVs
			oscBlock.setOsc1Shape(params[OSC1SHAPE_PARAM].getValue() + 0.2f *inputs[OSC1SHAPE_INPUT].getPolyVoltageSimd<float_4>(c), c);
			oscBlock.setOsc1PW(params[OSC1PW_PARAM].getValue() 	 + 0.2f *inputs[OSC1PW_INPUT].getPolyVoltageSimd<float_4>(c), c);
			oscBlock.setOsc1Vol(params[OSC1VOL_PARAM].getValue()   + 0.1f *inputs[OSC1VOL_INPUT].getPolyVoltageSimd<float_4>(c), c);
			oscBlock.setOsc1Subvol(params[OSC1SUBVOL_PARAM].getValue() + 0.1f *inputs[OSC1SUBVOL_INPUT].getPolyVoltageSimd<float_4>(c), c);

			oscBlock.setOsc2Shape(params[OSC2SHAPE_PARAM].getValue() + 0.2f *inputs[OSC2SHAPE_INPUT].getPolyVoltageSimd<float_4>(c), c);
			oscBlock.setOsc2PW(params[OSC2PW_PARAM].getValue() 	 + 0.2f *inputs[OSC2PW_INPUT].getPolyVoltageSimd<float_4>(c), c);
			oscBlock.setOsc2Vol(params[OSC2VOL_PARAM].getValue()   + 0.1f *inputs[OSC2VOL_INPUT].getPolyVoltageSimd<float_4>(c), c);

			oscBlock.setSync(params[SYNC_PARAM].getValue() + inputs[SYNC_INPUT].getPolyVoltageSimd<float_4>(c) / 5.f, c);
			oscBlock.setFmAmount(params[FM_INPUT].getValue()  + 0.1f *inputs[FM_INPUT].getPolyVoltageSimd<float_4>(c), c);
			oscBlock.setRingmodVol(params[RINGMOD_PARAM].getValue() + 0.1f *inputs[RINGMOD_INPUT].getPolyVoltageSimd<float_4>(c), c);


			// frequencies
			if (lfoMode)
			{
				oscBlock.setOsc1FreqVOctLFO(inputs[OSC1VOCT_INPUT].getVoltageSimd<float_4>(c), c);
				oscBlock.setOsc2FreqVOctLFO(inputs[OSC2VOCT_INPUT].getPolyVoltageSimd<float_4>(c), c);
			}
			else
			{
				oscBlock.setOsc1FreqVOct(inputs[OSC1VOCT_INPUT].getVoltageSimd<float_4>(c), c);
				oscBlock.setOsc2FreqVOct(inputs[OSC2VOCT_INPUT].getPolyVoltageSimd<float_4>(c), c);
			}


			// calculate the oversampled oscillators
			float_4* inBuffer = decimator[c/4].getInputArray(actualOversamplingRate);

			if (antiAliasing)
			{
				oscBlock.processBandlimited(inBuffer, c);
			}
			else
			{
				oscBlock.process(inBuffer, c);
			}

			// dc blocker and saturator
			bool calcDcBlock = dcBlock && !lfoMode;
			for (size_t i = 0; i < actualOversamplingRate; ++i)
			{
				// DC blocker
				if (calcDcBlock)
				{
					dcBlocker[c/4].process(inBuffer[i]);
					inBuffer[i] = dcBlocker[c/4].highpass();
				}

				// saturator +-10V
				inBuffer[i] = musx::cheapSaturator(inBuffer[i]);
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
		json_object_set_new(rootJ, "antiAliasing", json_boolean(antiAliasing));
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
		json_t* antiAliasingJ = json_object_get(rootJ, "antiAliasing");
		if (antiAliasingJ)
		{
			antiAliasing = (json_boolean_value(antiAliasingJ));
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

		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(8.361, 28.888)), module, Oscillators::OSC1SHAPE_INPUT));
		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(23.601, 28.888)), module, Oscillators::OSC1PW_INPUT));
		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(37.253, 28.888)), module, Oscillators::OSC1VOL_INPUT));
		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(48.683, 28.888)), module, Oscillators::OSC1SUBVOL_INPUT));
		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(10.478, 61.012)), module, Oscillators::SYNC_INPUT));
		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(25.717, 61.012)), module, Oscillators::FM_INPUT));
		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(42.545, 61.012)), module, Oscillators::RINGMOD_INPUT));
		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(12.594, 94.196)), module, Oscillators::OSC2SHAPE_INPUT));
		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(28.892, 94.196)), module, Oscillators::OSC2PW_INPUT));
		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(45.72, 94.196)), module, Oscillators::OSC2VOL_INPUT));
		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(12.7, 112.438)), module, Oscillators::OSC1VOCT_INPUT));
		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(27.94, 112.438)), module, Oscillators::OSC2VOCT_INPUT));

		addOutput(createOutputCentered<ThemedPJ301MPort>(mm2px(Vec(43.256, 112.438)), module, Oscillators::OUT_OUTPUT));
	}

	void appendContextMenu(Menu* menu) override {
		Oscillators* module = getModule<Oscillators>();

		menu->addChild(new MenuSeparator);

		menu->addChild(createIndexSubmenuItem("Oversampling rate", {"1x", "2x", "4x", "8x", "16x", "32x", "64x", "128x", "256x", "512x", "1024x"},
			[=]() {
				return log2((int)module->oversamplingRate);
			},
			[=](int mode) {
				module->setOversamplingRate((size_t)std::pow(2, mode));
			}
		));

		menu->addChild(createBoolMenuItem("Anti-aliasing (polyBLEP & polyBLAMP)", "",
			[=]() {
				return module->antiAliasing;
			},
			[=](int mode) {
				module->antiAliasing = mode;
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
