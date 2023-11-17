#include "plugin.hpp"
#include <svf.hpp>


struct Delay : Module {
	enum ParamId {
		TIME_PARAM,
		FEEDBACK_PARAM,
		CUTOFF_PARAM,
		RESONANCE_PARAM,
		NOISE_PARAM,
		BBD_SIZE_PARAM,
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
		INVERT_LIGHT,
		LIGHTS_LEN
	};

	static constexpr float minDelayTime = 10.f; // ms
	static constexpr float maxDelayTime = 10000.f; // ms
	static const int maxDelayLineSize = 32768;
	int delayLineSize = 4096;
	// TODO stereo delay with simd
	float delayLine1[maxDelayLineSize] = {0};
	//float delayLine2[delayLineSize] = {0};
	int index = 0;
	float in = 0;
	int inN = 0;
	float out = 0;
	float lastOut = 0;

	int oversamplingRate = 16; // TODO check how much oversampling is necessary for min delay time

	double phasor = 0;

	static constexpr float minCutoff = 200.f; // Hz
	static constexpr float maxCutoff = 15000.f; // Hz
	// TODO combine filters with simd
	// input filter
	SVF inFilter;

	// output filter
	SVF outFilter;

	// compressor
	dsp::TRCFilter<float> compAmplitude;

	// expander
	dsp::TRCFilter<float> expAmplitude;

	// DC block
	dsp::TRCFilter<float> dcBlocker;

	dsp::ClockDivider lightDivider;
	dsp::ClockDivider knobDivider;

	Delay() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configParam(TIME_PARAM, 0.f, 1.f, 0.5f, "Delay time", " ms", maxDelayTime/minDelayTime, minDelayTime);
		configParam(FEEDBACK_PARAM, 0.f, 2.0f, 0.2f, "Feedback", " %", 0, 100);

		configParam(CUTOFF_PARAM, 0.f, 1.f, 0.5f, "Low pass filter cutoff frequency", " Hz", maxCutoff/minCutoff, minCutoff);
		configParam(RESONANCE_PARAM, 0.f, 0.5f, 0.f, "Low pass filter resonance", " %", 0, 200);
		configParam(NOISE_PARAM, 0.f, 10.f, 2.5f, "Noise level", " %", 0, 10);
		configParam(BBD_SIZE_PARAM, 8, 15, 12, "BBD delay line size", " buckets", 2);
		getParamQuantity(BBD_SIZE_PARAM)->snapEnabled = true;
		getParamQuantity(BBD_SIZE_PARAM)->smoothEnabled = false;

		configParam(INPUT_PARAM, 0.f, 2.f, 1.f, "Delay input level", " %", 0, 100);
		configParam(STEREO_WIDTH_PARAM, 0.f, 1.f, 0.05f, "Stereo width", " %", 0, 100);
		configSwitch(INVERT_PARAM, 0, 1, 0, "R wet signal = - L wet signal (Chorus mode)");
		configParam(MIX_PARAM, 0.f, 1.f, 0.5f, "Dry-wet mix", " %", 0, 100);

		configInput(TIME_CV_INPUT, "Delay time CV");
		configInput(FEEDBACK_CV_INPUT, "Feedback CV");
		configInput(L_INPUT, "Left / Mono");
		configInput(R_INPUT, "Right");

		configOutput(L_OUTPUT, "Left");
		configOutput(R_OUTPUT, "Right");

		lightDivider.setDivision(128);
		knobDivider.setDivision(8);

		configBypass(L_INPUT, L_OUTPUT);
		configBypass(R_INPUT, R_OUTPUT);
	}

	void onSampleRateChange(const SampleRateChangeEvent& e) override {
		compAmplitude.setCutoffFreq(5.01f/e.sampleRate);
		expAmplitude.setCutoffFreq(4.964f/e.sampleRate);
		dcBlocker.setCutoffFreq(20.f/e.sampleRate);
	}

	void process(const ProcessArgs& args) override {
		if (knobDivider.process())
		{
			delayLineSize = std::pow(2, params[BBD_SIZE_PARAM].getValue());

			// calculate frequencies for anti-aliasing- and reconstruction-filter
			float filterFreq = std::pow(maxCutoff/minCutoff, params[CUTOFF_PARAM].getValue()) * minCutoff / args.sampleRate; // f_c / f_s
			inFilter.setCutoffFreq(filterFreq);
			outFilter.setCutoffFreq(filterFreq*1.054);

			// set filter q
			inFilter.setResonance(params[RESONANCE_PARAM].getValue());
			outFilter.setResonance(params[RESONANCE_PARAM].getValue()*0.984);

		}

		// inputs
		float inL = inputs[L_INPUT].getVoltageSum();
		float inR = inL;
		if (inputs[R_INPUT].isConnected())
		{
			inR = inputs[R_INPUT].getVoltageSum();
		}

		float inMono = 0.5f * (inL + inR) * params[INPUT_PARAM].getValue();

		// calculate frequency for BBD clock
		float delayTime = std::pow(maxDelayTime/minDelayTime, params[TIME_PARAM].getValue() + 0.1f * inputs[TIME_CV_INPUT].getVoltageSum()) * minDelayTime; // [ms]
		float freq = 1.f/delayTime * 1000.f; // [Hz]
		double phaseInc = 1.f / args.sampleRate * freq / oversamplingRate * 2 * delayLineSize;


		// feedback
		inMono += (params[FEEDBACK_PARAM].getValue() + 0.1f * inputs[FEEDBACK_CV_INPUT].getVoltageSum()) * lastOut;

		// saturate
		inMono = tanh(inMono / 10.f) * 10.f;

		// compressor
		inMono = compress(inMono);

		// anti-aliasing filter
		inFilter.process(inMono);
		inMono = inFilter.lowpass();

		out = 0;
		// oversampled BBD simulation
		for (int i = 0; i < oversamplingRate; ++i)
		{
			// simple average over input
			in += inMono;
			++inN;

			// readout
			float readout = delayLine1[index];

			// add delay-time-dependent noise
			readout += params[NOISE_PARAM].getValue() * rack::random::normal() / freq;

			// nonlinearity
			readout = waveshape(readout/5.f)*5.f;

			out += readout;

			if (phasor + phaseInc > 1.f)
			{
				// fill bucket
				delayLine1[index] = in/inN;

				// reset input averager
				in = 0;
				inN = 0;

				// advance BBD delay line
				++index;
				index &= delayLineSize-1;
			}

			phasor += phaseInc;
			phasor = phasor > 1.f ? phasor - 2.f : phasor;
		}

		// simple average over output
		out /= oversamplingRate;

		// DC blocker
		dcBlocker.process(out);
		out = dcBlocker.highpass();


		// reconstruction filter
		outFilter.process(out);
		out = outFilter.lowpass();

		// expander
		out = expand(out);

		lastOut = out;

		outputs[L_OUTPUT].setVoltage(std::min(1.f, (2.f - 2.f * params[MIX_PARAM].getValue())) * inL +
				std::min(1.f, 2.f * params[MIX_PARAM].getValue()) * out);

		if (params[INVERT_PARAM].getValue())
		{
			// chorus mode
			outputs[R_OUTPUT].setVoltage(std::min(1.f, (2.f - 2.f * params[MIX_PARAM].getValue())) * inR -
								std::min(1.f, 2.f * params[MIX_PARAM].getValue()) * out);
		}
		else
		{
			outputs[R_OUTPUT].setVoltage(std::min(1.f, (2.f - 2.f * params[MIX_PARAM].getValue())) * inR +
					std::min(1.f, 2.f * params[MIX_PARAM].getValue()) * out);
		}

		// Light
		if (lightDivider.process()) {
			lights[INVERT_LIGHT].setBrightness(params[INVERT_PARAM].getValue());
		}
	}

	float compress(float in)
	{
		float gain = 1. / std::max(compAmplitude.lowpass(), 0.1f);
		float out = gain * in;
		compAmplitude.process(std::abs(out));
		return out;

		/*
		compAmplitude.process(std::abs(in));
		float amp = compAmplitude.lowpass();
		float gain = 1.f;
		if (amp > 1.f)
		{
			gain = (0.5f + 0.5f * amp) / amp;
		}
		return 1.7f * gain*in;
		*/
	}

	float expand(float in)
	{
		expAmplitude.process(std::abs(in));
		float gain = std::min(expAmplitude.lowpass(), 10.f);
		return gain * in;

		/*
		expAmplitude.process(std::abs(in));
		float amp = expAmplitude.lowpass();
		float gain = 1.f;
		if (amp > 1.f)
		{
			gain = amp / (0.5f + 0.5f * amp);
		}
		return gain*in / 1.7f;
		*/
	}


	float waveshape(float in)
	{
		static const float a = {1.f/8.f};
		static const float b = {1.f/18.f};

		if (in > 1.f)
		{
			return 1.f - a - b;
		}
		if (in > -1.f)
		{
			return in - a*in*in - b*in*in*in + a;
		}
		return -1.f - a - b;




		// clips at +-5.443V
		// 1. * x - 0.5 * x *x * x
		in *= 0.1;
		in = std::fmax(std::fmin(in, 0.5443f), -0.5443f);
		in -= 0.5f * in*in*in;
		return 10.f*in;
	}

	float tanh(float x)
	{
		return std::tanh(x);

		// TODO Pade approximant of tanh
		x = simd::clamp(x, -3.f, 3.f);
		return x * (27 + x * x) / (27 + 9 * x * x);
	}
};


struct DelayWidget : ModuleWidget {
	DelayWidget(Delay* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/Delay.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<RoundBigBlackKnob>(mm2px(Vec(15.24, 24.094)), module, Delay::TIME_PARAM));
		addParam(createParamCentered<RoundBigBlackKnob>(mm2px(Vec(45.72, 24.094)), module, Delay::FEEDBACK_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(7.62, 64.25)), module, Delay::CUTOFF_PARAM));
		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(22.86, 64.25)), module, Delay::RESONANCE_PARAM));
		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(38.1, 64.25)), module, Delay::NOISE_PARAM));
		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(53.34, 64.25)), module, Delay::BBD_SIZE_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(7.62, 88.344)), module, Delay::INPUT_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(22.86, 88.344)), module, Delay::STEREO_WIDTH_PARAM));
		addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<WhiteLight>>>(mm2px(Vec(38.1, 88.344)), module, Delay::INVERT_PARAM, Delay::INVERT_LIGHT));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(53.34, 88.344)), module, Delay::MIX_PARAM));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(15.24, 40.156)), module, Delay::TIME_CV_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(45.72, 40.156)), module, Delay::FEEDBACK_CV_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.62, 112.438)), module, Delay::L_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(22.86, 112.438)), module, Delay::R_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(38.312, 112.438)), module, Delay::L_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(53.552, 112.438)), module, Delay::R_OUTPUT));
	}
};


Model* modelDelay = createModel<Delay, DelayWidget>("Delay");
