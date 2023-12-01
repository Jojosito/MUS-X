#include "plugin.hpp"
#include "dsp/compander.hpp"
#include "dsp/functions.hpp"

namespace musx {

using namespace rack;
using simd::float_4;

struct Delay : Module {
	enum ParamId {
		TIME_PARAM,
		FEEDBACK_PARAM,
		TAP_PARAM,
		CUTOFF_PARAM,
		RESONANCE_PARAM,
		NOISE_PARAM,
		BBD_SIZE_PARAM,
		POLES_PARAM,
		COMPANDER_PARAM,
		INPUT_PARAM,
		STEREO_WIDTH_PARAM,
		INVERT_PARAM,
		MIX_PARAM,
		PARAMS_LEN
	};
	enum InputId {
		TIME_CV_INPUT,
		FEEDBACK_CV_INPUT,
		L_INPUT,
		R_INPUT,
		INPUTS_LEN
	};
	enum OutputId {
		L_OUTPUT,
		R_OUTPUT,
		OUTPUTS_LEN
	};
	enum LightId {
		TAP_LIGHT,
		OVERLOAD_LIGHT,
		INVERT_LIGHT,
		LIGHTS_LEN
	};

	ParamQuantity* delayTimeQty;
	int delayLineSize = 4069;
	float minDelayTime = 1.f * delayLineSize / 256; // ms
	float maxDelayTime = 100.f * delayLineSize / 256; // ms
	float logMaxOverMin = std::log(maxDelayTime/minDelayTime); // log(maxDelayTime/minDelayTime)
	static const int maxDelayLineSize = 16384;
	float_4 delayLine[maxDelayLineSize] = {0};

	float prevTap = 0;
	int tapCounter = 0;

	int index = 0;
	float_4 in = 0;
	int inN = 0;
	float_4 out = 0;
	float_4 lastOut = 0;

	int oversamplingRate = 8;

	double phasor = 0;

	static constexpr float minCutoff = 200.f; // Hz
	static constexpr float maxCutoff = 10000.f; // Hz

	// input filter
	musx::TFourPole<float_4> inFilter;

	// output filter
	musx::TFourPole<float_4> outFilter;

	// compressor
	musx::TCompander<float_4> compander;

	// DC block
	musx::TOnePole<float_4> dcBlocker;

	dsp::ClockDivider lightDivider;
	dsp::ClockDivider knobDivider;

	double tapLightPhasor = 0;
	musx::TOnePole<float_4> lightFilter;

	Delay() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		delayTimeQty = configParam(TIME_PARAM, 0.f, 1.f, 0.65f, "Delay time", " ms", maxDelayTime/minDelayTime, minDelayTime);
		configParam(FEEDBACK_PARAM, 0.f, 3.0f, 0.5f, "Feedback", " %", 0, 100);

		configSwitch(TAP_PARAM, 0, 1, 0, "Tap tempo");


		configParam(CUTOFF_PARAM, 0.f, 1.f, 0.65f, "Low pass filter cutoff frequency", " Hz", maxCutoff/minCutoff, minCutoff);
		configParam(RESONANCE_PARAM, 0.f, 0.625f, 0.3125f, "Low pass filter resonance", " %", 0, 160);

		configParam(NOISE_PARAM, 0.f, 0.25f, 0.025f, "Noise level", " %", 0, 400);
		configParam(BBD_SIZE_PARAM, 8, 14, 12, "BBD delay line size", " buckets", 2);
		getParamQuantity(BBD_SIZE_PARAM)->snapEnabled = true;
		getParamQuantity(BBD_SIZE_PARAM)->smoothEnabled = false;

		configParam(POLES_PARAM, 0, 3, 1, "Low pass filter order", " dB/Oct", 0, 6, 6);
		getParamQuantity(POLES_PARAM)->snapEnabled = true;
		getParamQuantity(POLES_PARAM)->smoothEnabled = false;
		configParam(COMPANDER_PARAM, .1f, 40.f, 20.f, "Compander low pass filter frequency", " Hz");

		configParam(INPUT_PARAM, 0.f, 4.f, 1.f, "Delay input level", " %", 0, 100);
		configParam(STEREO_WIDTH_PARAM, 0.f, 1.f, 0.5f, "Stereo width", " %", 0, 100);
		configSwitch(INVERT_PARAM, 0, 1, 0, "R wet signal = - L wet signal (Chorus mode)");
		configParam(MIX_PARAM, 0.f, 1.f, 0.5f, "Dry-wet mix", " %", 0, 100);

		configInput(TIME_CV_INPUT, "Delay time CV");
		configInput(FEEDBACK_CV_INPUT, "Feedback CV");
		configInput(L_INPUT, "Left / Mono");
		configInput(R_INPUT, "Right");

		configOutput(L_OUTPUT, "Left");
		configOutput(R_OUTPUT, "Right");

		lightDivider.setDivision(256);
		knobDivider.setDivision(16);

		configBypass(L_INPUT, L_OUTPUT);
		configBypass(R_INPUT, R_OUTPUT);
	}

	void onSampleRateChange(const SampleRateChangeEvent& e) override {
		compander.setCompressorCutoffFreq(params[COMPANDER_PARAM].getValue()/e.sampleRate);
		compander.setExpanderCutoffFreq(params[COMPANDER_PARAM].getValue()/e.sampleRate);
		dcBlocker.setCutoffFreq(20.f/e.sampleRate);
		lightFilter.setCutoffFreq(5.f/e.sampleRate*lightDivider.getDivision());

		int factor = std::round(std::log2(46050/e.sampleRate));
		oversamplingRate = 8 * pow(2, factor);
		oversamplingRate = std::fmax(1, oversamplingRate);
	}

	void process(const ProcessArgs& args) override {
		if (knobDivider.process())
		{
			int newDelayLineSize = std::pow(2, params[BBD_SIZE_PARAM].getValue());
			if (newDelayLineSize != delayLineSize)
			{
				delayLineSize = newDelayLineSize;
				minDelayTime = 1.f * delayLineSize / 256;
				maxDelayTime = 100.f * delayLineSize / 256;
				logMaxOverMin = std::log(maxDelayTime/minDelayTime);
			}

			delayTimeQty->ParamQuantity::displayBase = maxDelayTime/minDelayTime;
			delayTimeQty->ParamQuantity::displayMultiplier = minDelayTime;

			// calculate frequencies for anti-aliasing- and reconstruction-filter
			float_4 cutoffFreq = std::pow(maxCutoff/minCutoff, params[CUTOFF_PARAM].getValue()) * minCutoff / args.sampleRate; // f_c / f_s

			inFilter.setCutoffFreq(cutoffFreq);
			outFilter.setCutoffFreq(cutoffFreq);

			inFilter.setResonance(params[RESONANCE_PARAM].getValue());
			outFilter.setResonance(params[RESONANCE_PARAM].getValue());

			// tap tempo
			++tapCounter;
			if (!prevTap && params[TAP_PARAM].getValue())
			{
				float delayTime = tapCounter * knobDivider.getDivision() / args.sampleRate * 1000.f; // ms
				float param = std::log(delayTime/minDelayTime) / logMaxOverMin;
				if (param < 1.f)
				{
					params[TIME_PARAM].setValue(param);
				}
				tapCounter = 0;
			}
			prevTap = params[TAP_PARAM].getValue();

			compander.setCompressorCutoffFreq(params[COMPANDER_PARAM].getValue()/args.sampleRate);
			compander.setExpanderCutoffFreq(params[COMPANDER_PARAM].getValue()/args.sampleRate);
		}

		// calculate frequency for BBD clock
		// pow(a, b) = exp(b * log(a))
		float delayTime = std::exp(logMaxOverMin * simd::clamp(params[TIME_PARAM].getValue() + 0.1f * inputs[TIME_CV_INPUT].getVoltageSum(), 0.f, 1.f)) * minDelayTime; // [ms]
		float freq = 1.f/delayTime * 1000.f; // [Hz]
		double phaseInc = 1.f / args.sampleRate * freq / oversamplingRate * 2 * delayLineSize;

		// inputs
		float inL = inputs[L_INPUT].getVoltageSum();
		float inR = inputs[R_INPUT].isConnected() ? inputs[R_INPUT].getVoltageSum() : inL;

		float_4 inMono;
		inMono[0] = 0.5f * (inL + inR) * params[INPUT_PARAM].getValue();

		// feedback
		if (params[INVERT_PARAM].getValue())
		{
			// chorus mode: feedback from delay line 1
			inMono[0] += (params[FEEDBACK_PARAM].getValue() + 0.1f * inputs[FEEDBACK_CV_INPUT].getVoltageSum()) * lastOut[0];
		}
		else
		{
			// output of delay line 0 (l) is is input of delay line 1 (r)
			inMono[0] += (params[FEEDBACK_PARAM].getValue() + 0.1f * inputs[FEEDBACK_CV_INPUT].getVoltageSum()) * lastOut[1];
			inMono[1] = (params[FEEDBACK_PARAM].getValue() + 0.1f * inputs[FEEDBACK_CV_INPUT].getVoltageSum()) * lastOut[0];
		}

		// saturate
		inMono = musx::tanh(inMono / 10.f) * 10.f; // +-10V

		// compressor
		inMono = compander.compress(inMono);

		// anti-aliasing filter
		inFilter.process(inMono);
		inMono = inFilter.lowpassN(params[POLES_PARAM].getValue());

		out = 0;
		// oversampled BBD simulation
		for (int i = 0; i < oversamplingRate; ++i)
		{
			// simple average over input
			in += inMono;
			++inN;

			// readout
			float_4 readout = delayLine[index];

			// add noise
			readout += params[NOISE_PARAM].getValue() * random::normal();

			// nonlinearity
			readout = musx::waveshape(readout/5.f)*5.f;

			out += readout;

			// bbd
			if (phasor + phaseInc > 1.f)
			{
				// fill bucket
				delayLine[index] = in/inN;

				// reset input averager
				in = 0;
				inN = 0;

				// advance BBD delay line
				++index;
				index &= delayLineSize-1;

				phasor -= 2.f;
			}

			phasor += phaseInc;
		}

		// simple average over output
		out /= oversamplingRate;

		// DC blocker
		dcBlocker.process(out);
		out = dcBlocker.highpass();

		// reconstruction filter
		outFilter.process(out);
		out = outFilter.lowpassN(params[POLES_PARAM].getValue());

		// expander
		out = compander.expand(out);

		// saturate
		out = musx::tanh(out / 10.f) * 10.f; // +-10V

		lastOut = out;

		// L R
		float outMono = 0.5f * (out[0] + out[1]);
		float outL = out[0] * params[STEREO_WIDTH_PARAM].getValue() + (1. - params[STEREO_WIDTH_PARAM].getValue()) * outMono;
		float outR = out[1] * params[STEREO_WIDTH_PARAM].getValue() + (1. - params[STEREO_WIDTH_PARAM].getValue()) * outMono;

		outputs[L_OUTPUT].setVoltage(std::min(1.f, (2.f - 2.f * params[MIX_PARAM].getValue())) * inL +
				std::min(1.f, 2.f * params[MIX_PARAM].getValue()) * outL);

		if (params[INVERT_PARAM].getValue())
		{
			// chorus mode
			outputs[R_OUTPUT].setVoltage(std::min(1.f, (2.f - 2.f * params[MIX_PARAM].getValue())) * inR -
								std::min(1.f, 2.f * params[MIX_PARAM].getValue()) * outL);
		}
		else
		{
			outputs[R_OUTPUT].setVoltage(std::min(1.f, (2.f - 2.f * params[MIX_PARAM].getValue())) * inR +
					std::min(1.f, 2.f * params[MIX_PARAM].getValue()) * outR);
		}

		// Light
		if (lightDivider.process()) {
			double tapPhaseInc = 1.f / args.sampleRate * freq * 2 * lightDivider.getDivision();
			tapLightPhasor += tapPhaseInc;
			tapLightPhasor = tapLightPhasor > 1.f ? tapLightPhasor - 2.f : tapLightPhasor;
			float_4 lightSignal = {
					float(tapLightPhasor > 0.f),
					compander.compressorAmplitude()[0],
					0, 0};
			lightFilter.process(lightSignal);

			float tapBrightness = 0.5;
			if (freq < 60.f)
			{
				tapBrightness = lightFilter.lowpass()[0];
			}
			lights[TAP_LIGHT].setBrightness(tapBrightness);

			float overloadBrightness = 0;
			if (lightFilter.lowpass()[1] > 2.f)
			{
				overloadBrightness = lightFilter.lowpass()[1] - 2.f;
			}
			lights[OVERLOAD_LIGHT].setBrightness(overloadBrightness);

			lights[INVERT_LIGHT].setBrightness(params[INVERT_PARAM].getValue());
		}
	}
};


struct DelayWidget : ModuleWidget {
	DelayWidget(Delay* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/Delay.svg"), asset::plugin(pluginInstance, "res/Delay-dark.svg")));

		addChild(createWidget<ThemedScrew>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ThemedScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ThemedScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ThemedScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<RoundBigBlackKnob>(mm2px(Vec(15.24, 24.094)), module, Delay::TIME_PARAM));
		addParam(createParamCentered<RoundBigBlackKnob>(mm2px(Vec(45.72, 24.094)), module, Delay::FEEDBACK_PARAM));
		auto tapParam = createLightParamCentered<VCVLightLatch<MediumSimpleLight<BlueLight>>>(mm2px(Vec(28.36, 32.125)), module, Delay::TAP_PARAM, Delay::TAP_LIGHT);
		tapParam->momentary = true;
		addParam(tapParam);

		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(7.62, 64.25)), module, Delay::CUTOFF_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(22.86, 64.25)), module, Delay::RESONANCE_PARAM));

		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(38.1, 56.219)), module, Delay::NOISE_PARAM));
		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(53.34, 56.219)), module, Delay::BBD_SIZE_PARAM));
		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(38.1, 72.281)), module, Delay::POLES_PARAM));
		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(53.34, 72.281)), module, Delay::COMPANDER_PARAM));

		addChild(createLight<MediumLight<RedLight>>(mm2px(Vec(3., 80.0)), module, Delay::OVERLOAD_LIGHT));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(7.62, 88.344)), module, Delay::INPUT_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(22.86, 88.344)), module, Delay::STEREO_WIDTH_PARAM));
		addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<WhiteLight>>>(mm2px(Vec(38.1, 88.344)), module, Delay::INVERT_PARAM, Delay::INVERT_LIGHT));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(53.34, 88.344)), module, Delay::MIX_PARAM));

		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(15.24, 40.156)), module, Delay::TIME_CV_INPUT));
		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(45.72, 40.156)), module, Delay::FEEDBACK_CV_INPUT));
		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(7.62, 112.438)), module, Delay::L_INPUT));
		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(22.86, 112.438)), module, Delay::R_INPUT));

		addOutput(createOutputCentered<ThemedPJ301MPort>(mm2px(Vec(38.312, 112.438)), module, Delay::L_OUTPUT));
		addOutput(createOutputCentered<ThemedPJ301MPort>(mm2px(Vec(53.552, 112.438)), module, Delay::R_OUTPUT));
	}
};


Model* modelDelay = createModel<Delay, DelayWidget>("Delay");

}
