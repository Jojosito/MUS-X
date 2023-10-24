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
		SYNC_INPUT,
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
		LIGHTS_LEN
	};

	int channels = 1;
	int32_4 phasor1[4] = {0};
	int32_4 phasor2[4] = {0};
	dsp::Decimator<16, 16> decimator;

	Oscillators() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configParam(OSC1SHAPE_PARAM, 0.f, 1.f, 0.f, "Oscillator 1 shape");
		configParam(OSC1PW_PARAM, 0.f, 1.f, 0.5f, "Oscillator 1 pulse width", " %", 0.f, 100.f);
		configParam(OSC1VOL_PARAM, 0.f, 1.f, 0.f, "Oscillator 1 volume", " %", 0.f, 100.f);
		configParam(OSC2SHAPE_PARAM, 0.f, 1.f, 0.f, "Oscillator 2 shape");
		configParam(OSC2PW_PARAM, 0.f, 1.f, 0.5f, "Oscillator 2 pulse width", " %", 0.f, 100.f);
		configParam(OSC2VOL_PARAM, 0.f, 1.f, 0.5f, "Oscillator 2 volume", " %", 0.f, 100.f);
		configParam(SYNC_PARAM, 0.f, 1.f, 0.f, "Sync Osc 2 to Osc 1");
		configParam(CROSSMOD_PARAM, 0.f, 1.f, 0.f, "Osc 1 to Osc2 crossmod");
		configParam(RINGMOD_PARAM, 0.f, 1.f, 0.f, "Ringmod volume", " %", 0.f, 100.f);
		configInput(OSC1SHAPE_INPUT, "Oscillator 1 shape CV");
		configInput(OSC1PW_INPUT, "Oscillator 1 pulse width CV");
		configInput(OSC1VOL_INPUT, "Oscillator 1 volume CV");
		configInput(OSC2SHAPE_INPUT, "Oscillator 2 shape CV");
		configInput(OSC2PW_INPUT, "Oscillator 2 pulse width CV");
		configInput(OSC2VOL_INPUT, "Oscillator 2 volume CV");
		configInput(SYNC_INPUT, "Sync CV");
		configInput(CROSSMOD_INPUT, "Crossmod CV");
		configInput(RINGMOD_INPUT, "Ringmod volume CV");
		configInput(OSC1VOCT_INPUT, "Oscillator 1 V/Oct");
		configInput(OSC2VOCT_INPUT, "Oscillator 2 V/Oct");
		configOutput(OUT_OUTPUT, "Mix");
	}

	void process(const ProcessArgs& args) override {
		channels = std::max(1, inputs[OSC1VOCT_INPUT].getChannels());
		outputs[OUT_OUTPUT].setChannels(channels);

		for (int c = 0; c < channels; c += 4) {
			float_4 freq = simd::clamp(dsp::FREQ_C4 * dsp::exp2_taylor5(inputs[OSC1VOCT_INPUT].getVoltageSimd<float_4>(c)), 0.1f, 20000.f);
			int32_4 phase1Inc = INT32_MAX / args.sampleRate * freq;

			phasor1[c/4] += phase1Inc;

			float_4 mix = -phasor1[c/4] / INT32_MAX;

			outputs[OUT_OUTPUT].setVoltageSimd(mix, c);
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
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(22.936, 48.188)), module, Oscillators::SYNC_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(38.176, 48.188)), module, Oscillators::CROSSMOD_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(53.491, 48.188)), module, Oscillators::RINGMOD_PARAM));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(22.936, 64.25)), module, Oscillators::OSC1SHAPE_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(38.176, 64.25)), module, Oscillators::OSC1PW_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(53.491, 64.25)), module, Oscillators::OSC1VOL_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(22.936, 80.313)), module, Oscillators::OSC2SHAPE_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(38.176, 80.313)), module, Oscillators::OSC2PW_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(53.491, 80.313)), module, Oscillators::OSC2VOL_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(22.936, 96.375)), module, Oscillators::SYNC_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(38.176, 96.375)), module, Oscillators::CROSSMOD_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(53.491, 96.375)), module, Oscillators::RINGMOD_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(22.936, 112.438)), module, Oscillators::OSC1VOCT_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(38.176, 112.438)), module, Oscillators::OSC2VOCT_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(53.491, 112.438)), module, Oscillators::OUT_OUTPUT));
	}
};


Model* modelOscillators = createModel<Oscillators, OscillatorsWidget>("Oscillators");
