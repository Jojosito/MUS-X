#include "plugin.hpp"
#include <math.hpp>
#include "dsp/decimator.hpp"
#include "dsp/odeFilters.hpp"
#include "dsp/functions.hpp"

namespace musx {

using namespace rack;
using simd::float_4;

struct Filter : Module {
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
	int oversamplingRate = 8;
	HalfBandDecimatorCascade<float_4> decimator[4];

	int channels = 1;

	Method method = Method::RK4;
	IntegratorType integratorType = IntegratorType::Transistor;
	NonlinearityType nonlinearityType = NonlinearityType::alt3;

	Filter1Pole<float_4> filter1Pole[4];
	LadderFilter2Pole<float_4> ladderFilter2Pole[4];
	LadderFilter4Pole<float_4> ladderFilter4Pole[4];
	SallenKeyFilterLpBp<float_4> sallenKeyFilterLpBp[4];
	SallenKeyFilterHp<float_4> sallenKeyFilterHp[4];

	float_4 prevInput[4] = {0};

	bool saturate = true;
	musx::AntialiasedCheapSaturator<float_4> saturator[4];

	Filter() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configParam(CUTOFF_PARAM, 0.f, 1.f, 0.f, "Cutoff frequency", " Hz", base, minFreq);
		configParam(RESONANCE_PARAM, 0.f, 1.f, 0.f, "Resonance", " %", 0, 100.);
		configSwitch(MODE_PARAM, 0, 11, 1, "Mode", {
				"1-pole lowpass, 6 dB/Oct (non-resonant)",
				"1-pole highpass, 6 dB/Oct (non-resonant)",
				"2-pole ladder lowpass, 12 dB/Oct",
				"2-pole ladder bandpass, 6 dB/Oct",
				"4-pole ladder lowpass, 6 dB/Oct",
				"4-pole ladder lowpass, 12 dB/Oct",
				"4-pole ladder lowpass, 18 dB/Oct",
				"4-pole ladder lowpass, 24 dB/Oct",
				"2-pole Sallen-Key lowpass, 12 dB/Oct",
				"2-pole Sallen-Key bandpass, 6 dB/Oct",
				"2-pole Sallen-Key highpass, 6 dB/Oct",
				"2-pole Sallen-Key highpass, 12 dB/Oct"});
		configInput(CUTOFF_INPUT, "Cutoff frequency CV");
		configInput(RESONANCE_INPUT, "Resonance CV");
		configInput(IN_INPUT, "Audio");
		configOutput(OUT_OUTPUT, "Filtered");

		configBypass(IN_INPUT, OUT_OUTPUT);
	}

	void setIntegratorType(IntegratorType t)
	{
		integratorType = t;
		for (int c = 0; c < channels; c += 4)
		{
			filter1Pole[c/4].setIntegratorType(integratorType);
			ladderFilter2Pole[c/4].setIntegratorType(integratorType);
			ladderFilter4Pole[c/4].setIntegratorType(integratorType);
			sallenKeyFilterLpBp[c/4].setIntegratorType(integratorType);
			sallenKeyFilterHp[c/4].setIntegratorType(integratorType);
		}
	}

	void setNonlinearityType(NonlinearityType t)
	{
		nonlinearityType = t;
		for (int c = 0; c < channels; c += 4)
		{
			filter1Pole[c/4].setNonlinearityType(nonlinearityType);
			ladderFilter2Pole[c/4].setNonlinearityType(nonlinearityType);
			ladderFilter4Pole[c/4].setNonlinearityType(nonlinearityType);
			sallenKeyFilterLpBp[c/4].setNonlinearityType(nonlinearityType);
			sallenKeyFilterHp[c/4].setNonlinearityType(nonlinearityType);
		}
	}

	void process(const ProcessArgs& args) override {

		channels = std::max(1, inputs[IN_INPUT].getChannels());
		outputs[OUT_OUTPUT].setChannels(channels);

		for (int c = 0; c < channels; c += 4) {
			// set cutoff
			float_4 voltage = params[CUTOFF_PARAM].getValue() + 0.1f * inputs[CUTOFF_INPUT].getPolyVoltageSimd<float_4>(c);
			float_4 frequency = simd::exp(logBase * voltage) * minFreq;
			frequency  = simd::clamp(frequency, minFreq, simd::fmin(maxFreq, args.sampleRate * oversamplingRate / 8.f));

			// resonance
			float_4 resonance = 5. * (params[RESONANCE_PARAM].getValue() + 0.1f * inputs[RESONANCE_INPUT].getPolyVoltageSimd<float_4>(c));
			resonance = fmax(0.f, resonance);

			switch ((int)params[MODE_PARAM].getValue())
			{
			case 0:
			case 1:
				filter1Pole[c/4].setCutoffFreq(frequency);
				filter1Pole[c/4].setResonance(resonance);
				break;
			case 2:
			case 3:
				ladderFilter2Pole[c/4].setCutoffFreq(frequency);
				ladderFilter2Pole[c/4].setResonance(resonance);
				break;
			case 4:
			case 5:
			case 6:
			case 7:
				ladderFilter4Pole[c/4].setCutoffFreq(frequency);
				ladderFilter4Pole[c/4].setResonance(resonance);
				break;
			case 8:
			case 9:
				sallenKeyFilterLpBp[c/4].setCutoffFreq(frequency);
				sallenKeyFilterLpBp[c/4].setResonance(resonance);
				break;
			case 10:
			case 11:
				sallenKeyFilterHp[c/4].setCutoffFreq(frequency);
				sallenKeyFilterHp[c/4].setResonance(resonance);
				break;
			}

			// process
			float_4* inBuffer = decimator[c/4].getInputArray(oversamplingRate);
			for (int i = 0; i < oversamplingRate; ++i)
			{
				// linear interpolation for input
				float_4 in = crossfade(prevInput[c/4], inputs[IN_INPUT].getVoltageSimd<float_4>(c), (i + 1.f)/oversamplingRate);

				switch ((int)params[MODE_PARAM].getValue())
				{
				case 0:
					filter1Pole[c/4].process(in, args.sampleTime / oversamplingRate, method);
					inBuffer[i] = filter1Pole[c/4].lowpass();
					break;
				case 1:
					filter1Pole[c/4].process(in, args.sampleTime / oversamplingRate, method);
					inBuffer[i] = filter1Pole[c/4].highpass();
					break;
				case 2:
					ladderFilter2Pole[c/4].process(in, args.sampleTime / oversamplingRate, method);
					inBuffer[i] = ladderFilter2Pole[c/4].lowpass();
					break;
				case 3 :
					ladderFilter2Pole[c/4].process(in, args.sampleTime / oversamplingRate, method);
					inBuffer[i] = ladderFilter2Pole[c/4].bandpass();
					break;
				case 4:
					ladderFilter4Pole[c/4].process(in, args.sampleTime / oversamplingRate, method);
					inBuffer[i] = ladderFilter4Pole[c/4].lowpass6();
					break;
				case 5:
					ladderFilter4Pole[c/4].process(in, args.sampleTime / oversamplingRate, method);
					inBuffer[i] = ladderFilter4Pole[c/4].lowpass12();
					break;
				case 6:
					ladderFilter4Pole[c/4].process(in, args.sampleTime / oversamplingRate, method);
					inBuffer[i] = ladderFilter4Pole[c/4].lowpass18();
					break;
				case 7:
					ladderFilter4Pole[c/4].process(in, args.sampleTime / oversamplingRate, method);
					inBuffer[i] = ladderFilter4Pole[c/4].lowpass24();
					break;
				case 8:
					sallenKeyFilterLpBp[c/4].process(in, args.sampleTime / oversamplingRate, method);
					inBuffer[i] = sallenKeyFilterLpBp[c/4].lowpass();
					break;
				case 9:
					sallenKeyFilterLpBp[c/4].process(in, args.sampleTime / oversamplingRate, method);
					inBuffer[i] =sallenKeyFilterLpBp[c/4].bandpass();
					break;
				case 10:
					sallenKeyFilterHp[c/4].process(in, args.sampleTime / oversamplingRate, method);
					inBuffer[i] = sallenKeyFilterHp[c/4].highpass6();
					break;
				case 11:
					sallenKeyFilterHp[c/4].process(in, args.sampleTime / oversamplingRate, method);
					inBuffer[i] = sallenKeyFilterHp[c/4].highpass12();
					break;
				}

				if (saturate)
				{
					inBuffer[i] = saturator[c/4].processBandlimited(inBuffer[i]);
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
		json_object_set_new(rootJ, "method", json_integer((int)method));
		json_object_set_new(rootJ, "integratorType", json_integer((int)integratorType));
		json_object_set_new(rootJ, "nonlinearityType", json_integer((int)nonlinearityType));
		json_object_set_new(rootJ, "saturate", json_boolean(saturate));
		return rootJ;
	}

	void dataFromJson(json_t* rootJ) override {
		json_t* oversamplingRateJ = json_object_get(rootJ, "oversamplingRate");
		if (oversamplingRateJ)
		{
			oversamplingRate = json_integer_value(oversamplingRateJ);
		}
		json_t* methodJ = json_object_get(rootJ, "method");
		if (methodJ)
		{
			method = (Method)json_integer_value(methodJ);
		}
		json_t* integratorTypeJ = json_object_get(rootJ, "integratorType");
		if (integratorTypeJ)
		{
			integratorType = (IntegratorType)json_integer_value(integratorTypeJ);
		}
		json_t* nonlinearityTypeJ = json_object_get(rootJ, "nonlinearityType");
		if (nonlinearityTypeJ)
		{
			nonlinearityType = (NonlinearityType)json_integer_value(nonlinearityTypeJ);
		}
		json_t* saturateJ = json_object_get(rootJ, "saturate");
		if (saturateJ)
		{
			saturate = (json_boolean_value(saturateJ));
		}
	}
};


struct FilterWidget : ModuleWidget {
	FilterWidget(Filter* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/Filter.svg"), asset::plugin(pluginInstance, "res/Filter-dark.svg")));

		addChild(createWidget<ThemedScrew>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ThemedScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ThemedScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ThemedScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(7.62, 16.062)), module, Filter::CUTOFF_PARAM));
		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(7.62, 26.26)), module, Filter::CUTOFF_INPUT));

		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(7.62, 44.52)), module, Filter::RESONANCE_PARAM));
		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(7.62, 54.59)), module, Filter::RESONANCE_INPUT));

		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(7.62, 72.04)), module, Filter::MODE_PARAM));

		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(7.62, 96.375)), module, Filter::IN_INPUT));

		addOutput(createOutputCentered<ThemedPJ301MPort>(mm2px(Vec(7.62, 112.438)), module, Filter::OUT_OUTPUT));
	}

	void appendContextMenu(Menu* menu) override {
		Filter* module = getModule<Filter>();

		menu->addChild(new MenuSeparator);

		menu->addChild(createIndexSubmenuItem("Oversampling rate", {"1x", "2x", "4x", "8x", "16x", "32x", "64x"},
			[=]() {
				return log2(module->oversamplingRate);
			},
			[=](int mode) {
				module->oversamplingRate = std::pow(2, mode);
			}
		));

		menu->addChild(createIndexSubmenuItem("ODE Solver", {"1st order Euler", "2nd order Runge-Kutta", "4th order Runge-Kutta"},
			[=]() {
				return (int)module->method;
			},
			[=](int mode) {
				module->method = (Method)mode;
			}
		));

		menu->addChild(createIndexSubmenuItem("Integrator type", {"Linear", "OTA", "Transistor"},
			[=]() {
				return (int)module->integratorType;
			},
			[=](int mode) {
				module->setIntegratorType((IntegratorType)mode);
			}
		));

		menu->addChild(createIndexSubmenuItem("Nonlinearity type", {"tanh", "alt1", "alt2", "alt3", "hardclip"},
			[=]() {
				return (int)module->nonlinearityType;
			},
			[=](int mode) {
				module->setNonlinearityType((NonlinearityType)mode);
			}
		));

		menu->addChild(createBoolMenuItem("Saturator", "",
			[=]() {
				return module->saturate;
			},
			[=](int mode) {
				module->saturate = mode;
			}
		));
	}
};


Model* modelFilter = createModel<Filter, FilterWidget>("Filter");

}
