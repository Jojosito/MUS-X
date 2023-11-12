#include "plugin.hpp"


struct Delay : Module {
	enum ParamId {
		TIME_PARAM,
		FEEDBACK_PARAM,
		NOISE_PARAM,
		LP_PARAM,
		STEREO_WIDTH_PARAM,
		MIX_PARAM,
		PARAMS_LEN
	};
	enum InputId {
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
		LIGHTS_LEN
	};

	static const int delayLineSize = 4096;
	float delayLine1[delayLineSize] = {0};
	//float delayLine2[delayLineSize] = {0};
	int index = 0;
	float in = 0;
	int inN = 0;
	float out = 0;
	float lastOut = 0;

	int oversamplingRate = 16;

	double phasor = 0;

	// input filter
	dsp::TRCFilter<float> inFilter;

	// output filter
	dsp::TRCFilter<float> outFilter;

	// compressor
	dsp::TRCFilter<float> compAmplitude;

	// expander
	dsp::TRCFilter<float> expAmplitude;


	Delay() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configParam(TIME_PARAM, 10.f, 1000.f, 200.f, "Delay time", " ms");
		configParam(FEEDBACK_PARAM, 0.f, 2.0f, 0.f, "Feedback", " %", 0, 100);
		configParam(NOISE_PARAM, 0.f, 1.f, 0.f, "Noise level");
		configParam(LP_PARAM, 0.25f, 2.f, 1.f, "Low pass offset");
		configParam(STEREO_WIDTH_PARAM, 0.f, 1.f, 1.f, "Stereo width", " %");
		configParam(MIX_PARAM, 0.f, 1.f, 0.5f, "Dry-wet mix");
		configInput(L_INPUT, "Left / Mono");
		configInput(R_INPUT, "Right");
		configOutput(L_OUTPUT, "Left");
		configOutput(R_OUTPUT, "Right");

		compAmplitude.setCutoffFreq(3.f/48000.f); // TODO use actual sample rate
		expAmplitude.setCutoffFreq(3.f/48000.f); // TODO use actual sample rate
	}

	void process(const ProcessArgs& args) override {
		float inL = inputs[L_INPUT].getVoltageSum();
		float inR = inL;
		if (inputs[R_INPUT].isConnected())
		{
			inR = inputs[R_INPUT].getVoltageSum();
		}

		float inMono = 0.5f * (inL + inR);


		float delayTime = params[TIME_PARAM].getValue(); // [ms]
		float freq = 1.f/delayTime * 1000.f; // [Hz]
		double phaseInc = 1.f / args.sampleRate * freq / oversamplingRate * 2 * delayLineSize;

		float filterFreq = params[LP_PARAM].getValue() * freq*delayLineSize / args.sampleRate;
		inFilter.setCutoffFreq(filterFreq);
		outFilter.setCutoffFreq(filterFreq);

		// compress
		inMono = compress(inMono + params[FEEDBACK_PARAM].getValue() * lastOut);

		// filter
		inFilter.process(inMono);
		inMono = inFilter.lowpass();

		out = 0;
		for (int i = 0; i < oversamplingRate; ++i)
		{
			// average over input
			in += inMono;
			++inN;

			// readout
			float readout = delayLine1[index];

			// add noise & nonlin
			readout += params[NOISE_PARAM].getValue() * rack::random::normal() / freq;
			readout = waveshape(readout);

			out += readout;

			if (phasor + phaseInc > 1.f)
			{
				// fill bucket
				delayLine1[index] = in/inN;

				// reset input averager
				in = 0;
				inN = 0;

				// advance delay line
				++index;
				index = (index >= delayLineSize) ? 0 : index;
			}

			phasor += phaseInc;
			phasor = phasor > 1.f ? phasor - 2.f : phasor;
		}

		// simple average
		out /= oversamplingRate;

		// filter
		outFilter.process(out);
		out = outFilter.lowpass();

		// expand
		out = expand(out);

		lastOut = out;

		outputs[L_OUTPUT].setVoltage(std::min(1.f, (2.f - 2.f * params[MIX_PARAM].getValue())) * inL +
				std::min(1.f, 2.f * params[MIX_PARAM].getValue()) * out);

		outputs[R_OUTPUT].setVoltage(std::min(1.f, (2.f - 2.f * params[MIX_PARAM].getValue())) * inR +
				std::min(1.f, 2.f * params[MIX_PARAM].getValue()) * out);
	}

	float compress(float in)
	{
		compAmplitude.process(std::abs(in));
		float amp = compAmplitude.lowpass();
		float gain = 1.f;
		if (amp > 1.f)
		{
			gain -= 0.25f * (amp - 1.f);
		}
		return gain*in;
	}

	float expand(float in)
	{
		expAmplitude.process(std::abs(in));
		float amp = expAmplitude.lowpass();
		float gain = 1.f;
		if (amp > 1.f)
		{
			gain += 0.25f * (amp - 1.f);
		}
		return gain*in;
	}

	float waveshape(float in)
	{
		// 1. * x - 0.5 * x *x * x
		in *= 0.1;
		in = std::fmax(std::fmin(in, 0.5443f), -0.5443f);
		in -= 0.5f * in*in*in;
		return 10.f*in;
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
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(15.24, 48.188)), module, Delay::FEEDBACK_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(7.62, 64.25)), module, Delay::NOISE_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(22.86, 64.25)), module, Delay::LP_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(7.62, 80.313)), module, Delay::STEREO_WIDTH_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(22.86, 80.313)), module, Delay::MIX_PARAM));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.62, 96.375)), module, Delay::L_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(22.86, 96.375)), module, Delay::R_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(7.62, 112.438)), module, Delay::L_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(22.86, 112.438)), module, Delay::R_OUTPUT));
	}
};


Model* modelDelay = createModel<Delay, DelayWidget>("Delay");
