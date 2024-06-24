#include "plugin.hpp"
#include "components/componentLibrary.hpp"

#include "blocks/ADSRBlock.hpp"
#include "blocks/FilterBlock.hpp"
#include "blocks/OscillatorsBlock.hpp"

namespace musx {

using namespace rack;

struct Synth : Module {
	enum ParamId {
		// assign params
		VOCT_ASSIGN_PARAM,
		GATE_ASSIGN_PARAM,
		VELOCITY_ASSIGN_PARAM,
		AFTERTOUCH_ASSIGN_PARAM,
		PITCH_WHEEL_ASSIGN_PARAM,
		MOD_WHEEL_ASSIGN_PARAM,
		EXPRESSION_ASSIGN_PARAM,
		INDIVIDUAL_MOD_1_ASSIGN_PARAM,
		INDIVIDUAL_MOD_2_ASSIGN_PARAM,
		VOICE_NR_ASSIGN_PARAM,
		RANDOM_ASSIGN_PARAM,

		ENV1_ASSIGN_PARAM,
		ENV2_ASSIGN_PARAM,
		LFO1_UNIPOLAR_ASSIGN_PARAM,
		LFO1_BIPOLAR_ASSIGN_PARAM,
		LFO2_UNIPOLAR_ASSIGN_PARAM,
		LFO2_BIPOLAR_ASSIGN_PARAM,
		GLOBAL_LFO_ASSIGN_PARAM,
		DRIFT_1_ASSIGN_PARAM,
		DRIFT_2_ASSIGN_PARAM,

		// modulatable params
		ENV1_A_PARAM,
		ENV1_D_PARAM,
		ENV1_S_PARAM,
		ENV1_R_PARAM,

		ENV2_A_PARAM,
		ENV2_D_PARAM,
		ENV2_S_PARAM,
		ENV2_R_PARAM,

		LFO1_FREQ_PARAM,
		LFO1_AMOUNT_PARAM,

		LFO2_FREQ_PARAM,
		LFO2_AMOUNT_PARAM,

		GLOBAL_LFO_FREQ_PARAM,
		GLOBAL_LFO_AMT_PARAM,

		INDIVIDUAL_MOD_OUT_1_PARAM,
		INDIVIDUAL_MOD_OUT_2_PARAM,
		INDIVIDUAL_MOD_OUT_3_PARAM,
		INDIVIDUAL_MOD_OUT_4_PARAM,

		OSC1_TUNE_GLIDE_PARAM,
		OSC1_TUNE_SEMI_PARAM,
		OSC1_TUNE_FINE_PARAM,
		OSC1_SHAPE_PARAM,
		OSC1_PW_PARAM,

		OSC2_TUNE_GLIDE_PARAM,
		OSC2_TUNE_SEMI_PARAM,
		OSC2_TUNE_FINE_PARAM,
		OSC2_SHAPE_PARAM,
		OSC2_PW_PARAM,

		OSC_FM_AMOUNT_PARAM,

		FILTER1_CUTOFF_PARAM,
		FILTER1_RESONANCE_PARAM,
		FILTER1_PAN_PARAM,

		FILTER2_CUTOFF_PARAM,
		FILTER2_RESONANCE_PARAM,
		FILTER2_PAN_PARAM,

		FILTER_SERIAL_PARALLEL_PARAM,

		AMP_VOL_PARAM,

		DELAY_TIME_PARAM,
		DELAY_FEEDBACK_PARAM,
		DELAY_MIX_PARAM,

		// mix params
		OSC1_VOL_PARAM,
		OSC1_SUB_VOL_PARAM,
		OSC_RM_VOL_PARAM,
		OSC_NOISE_VOL_PARAM,
		OSC2_VOL_PARAM,
		OSC_EXT_VOL_PARAM,

		// non modulatable params
		ENV1_VEL_PARAM,
		ENV2_VEL_PARAM,
		LFO1_SHAPE_PARAM,
		LFO1_MODE_PARAM,
		LFO2_SHAPE_PARAM,
		LFO2_MODE_PARAM,
		DRIFT_RATE_PARAM,
		DRIFT_BALANCE_PARAM,

		OSC1_TUNE_OCT_PARAM,
		OSC2_TUNE_OCT_PARAM,
		OSC_MIX_ROUTE_PARAM,
		OSC_SYNC_PARAM,

		FILTER1_TYPE_PARAM,
		FILTER2_CUTOFF_MODE_PARAM,
		FILTER2_TYPE_PARAM,

		DELAY_TAP_PARAM,

		PARAMS_LEN
	};
	enum InputId {
		VOCT_INPUT,
		GATE_INPUT,
		VELOCITY_INPUT,
		AFTERTOUCH_INPUT,
		PITCH_WHEEL_INPUT,
		MOD_WHEEL_INPUT,
		EXPRESSION_INPUT,
		INDIVIDUAL_MOD_1_INPUT,
		INDIVIDUAL_MOD_2_INPUT,
		RETRIGGER_INPUT,
		EXT_INPUT,
		INPUTS_LEN
	};
	enum OutputId {
		INDIVIDUAL_MOD_1_OUTPUT,
		INDIVIDUAL_MOD_2_OUTPUT,
		INDIVIDUAL_MOD_3_OUTPUT,
		INDIVIDUAL_MOD_4_OUTPUT,
		OUT_L_OUTPUT,
		OUT_R_OUTPUT,
		OUTPUTS_LEN
	};
	enum LightId {
		// assign lights
		VOCT_ASSIGN_LIGHT,
		GATE_ASSIGN_LIGHT,
		VELOCITY_ASSIGN_LIGHT,
		AFTERTOUCH_ASSIGN_LIGHT,
		PITCH_WHEEL_ASSIGN_LIGHT,
		MOD_WHEEL_ASSIGN_LIGHT,
		EXPRESSION_ASSIGN_LIGHT,
		INDIVIDUAL_MOD_1_ASSIGN_LIGHT,
		INDIVIDUAL_MOD_2_ASSIGN_LIGHT,
		VOICE_NR_ASSIGN_LIGHT,
		RANDOM_ASSIGN_LIGHT,

		ENV1_ASSIGN_LIGHT,
		ENV2_ASSIGN_LIGHT,
		LFO1_UNIPOLAR_ASSIGN_LIGHT,
		LFO1_BIPOLAR_ASSIGN_LIGHT,
		LFO2_UNIPOLAR_ASSIGN_LIGHT,
		LFO2_BIPOLAR_ASSIGN_LIGHT,
		GLOBAL_LFO_ASSIGN_LIGHT,
		DRIFT_1_ASSIGN_LIGHT,
		DRIFT_2_ASSIGN_LIGHT,

		// mix route light
		OSC_MIX_ROUTE_LIGHT,

		LIGHTS_LEN
	};

	//
	int channels = 1;

	// over/-undersampling
	static const size_t maxOversamplingRate = 8;

	dsp::ClockDivider uiDivider;
	dsp::ClockDivider modDivider;

	// mod matrix
	static constexpr size_t nSources = ENV1_A_PARAM; // number of modulation sources, + 1 for base vale
	static constexpr size_t nMixChannels = 6;
	static constexpr size_t nDestinations = ENV1_VEL_PARAM - ENV1_A_PARAM + nMixChannels; // number of modulation destinations, additional 6 for mix to filter balance

	size_t activeSourceAssign = 0; // index of active mod source assign button. 0 = base value / no button active

	bool oscMixRouteActive = false; // is the OSC_MIX_ROUTE_PARAM button pressed?
	float mixLevels[nMixChannels] = {0.};
	float mixFilterBalances[nMixChannels] = {0.};

	float_4 modMatrixInputs[nSources][4] = {0};
	float_4 modMatrixOutputs[nDestinations][4] = {0};

	float modMatrix[nSources][nDestinations] = {0}; // the mod matrix

	bool mustCalculateDestination[nDestinations] = {false}; // false if all but the first entry of the mod matrix column are 0

	// modulation blocks
	static constexpr float MIN_TIME = 1e-3f;
	static constexpr float MAX_TIME = 10.f;
	static constexpr float LAMBDA_BASE = MAX_TIME / MIN_TIME;
	static constexpr float ATT_TARGET = 1.2f;
	musx::ADSRBlock env1[4] = {musx::ADSRBlock(MIN_TIME, MAX_TIME, ATT_TARGET)};
	musx::ADSRBlock env2[4] = {musx::ADSRBlock(MIN_TIME, MAX_TIME, ATT_TARGET)};

	// audio blocks
	OscillatorsBlock<maxOversamplingRate> oscillators[4];

	musx::FilterBlock filter1[4];
	musx::FilterBlock filter2[4];

	Synth() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configParam(ENV1_VEL_PARAM, 0.f, 1.f, 0.f, "Envelope 1 velocity scaling", " %", 0, 100);
		configParam(ENV2_VEL_PARAM, 0.f, 1.f, 0.f, "Envelope 2 velocity scaling", " %", 0, 100);
		configParam(LFO1_SHAPE_PARAM, 0.f, 1.f, 0.f, "LFO 1 shape");
		configSwitch(LFO1_MODE_PARAM, 0, 2, 0, "LFO 1 mode", {"free running", "retrigger", "retrigger, single cycle"});
		configParam(LFO2_SHAPE_PARAM, 0.f, 1.f, 0.f, "LFO 2 shape");
		configSwitch(LFO2_MODE_PARAM, 0, 2, 0, "LFO 2 mode", {"free running", "retrigger", "retrigger, single cycle"});
		configParam(DRIFT_RATE_PARAM, 0.f, 1.f, 0.f, "Drift rate", " Hz");
		configParam(DRIFT_BALANCE_PARAM, 0.f, 1.f, 0.f, "Random constant offset / drift balance");
		configParam(OSC1_TUNE_OCT_PARAM, 0.f, 1.f, 0.f, "Oscillator 1 octave");
		configParam(OSC2_TUNE_OCT_PARAM, 0.f, 1.f, 0.f, "Oscillator 2 octave");
		configSwitch(OSC_SYNC_PARAM, 0,   1,   0,  "Sync", {"Off", "Sync oscillator 2 to oscillator 1"});
		configSwitch(OSC_MIX_ROUTE_PARAM, 0, 1, 0, "Adjust filter 1 / filter 2 input balance", {"", "active"});
		configSwitch(FILTER1_TYPE_PARAM, 0, FilterBlock::getModeLabels().size() - 1, 8, "Filter 1 type", FilterBlock::getModeLabels());
		configSwitch(FILTER2_CUTOFF_MODE_PARAM, 0, 2, 0, "Filter 2 cutoff mode", {"individual", "offset", "space"});
		configSwitch(FILTER2_TYPE_PARAM, 0, FilterBlock::getModeLabels().size() - 1, 8, "Filter 2 type", FilterBlock::getModeLabels());
		configSwitch(DELAY_TAP_PARAM, 0, 1, 0, "Delay tap tempo");
		configInput(VOCT_INPUT, "V/Oct");
		configInput(GATE_INPUT, "Gate");
		configInput(VELOCITY_INPUT, "Velocity");
		configInput(AFTERTOUCH_INPUT, "Aftertouch");
		configInput(PITCH_WHEEL_INPUT, "Pitch wheel");
		configInput(MOD_WHEEL_INPUT, "Mod wheel");
		configInput(EXPRESSION_INPUT, "Expression");
		configInput(INDIVIDUAL_MOD_1_INPUT, "Indvidual modulation 1");
		configInput(INDIVIDUAL_MOD_2_INPUT, "Indvidual modulation 2");
		configInput(RETRIGGER_INPUT, "Retrigger");
		configInput(EXT_INPUT, "External audio");
		configOutput(INDIVIDUAL_MOD_1_OUTPUT, "Indvidual modulation 1");
		configOutput(INDIVIDUAL_MOD_2_OUTPUT, "Indvidual modulation 2");
		configOutput(INDIVIDUAL_MOD_3_OUTPUT, "Indvidual modulation 3");
		configOutput(INDIVIDUAL_MOD_4_OUTPUT, "Indvidual modulation 4");
		configOutput(OUT_L_OUTPUT, "Left/Mono");
		configOutput(OUT_R_OUTPUT, "Right");

		const auto& sourceLabels = getSourceLabels();
		for (size_t i = 0; i < sourceLabels.size(); i++)
		{
			configSwitch(i, 0, 1, 0, "Assign " + sourceLabels[i], {"", "active"});
		}

		configureUi();

		uiDivider.setDivision(128);
		modDivider.setDivision(2);
	}

	static const std::array<std::string, nSources>& getSourceLabels()
	{
		static const std::array<std::string, nSources> sourceLabelMap = {
			"V/Oct",
			"gate",
			"velocity",
			"aftertouch",
			"pitch wheel",
			"mod wheel",
			"expression pedal",
			"indvidual modulation 1",
			"indvidual modulation 2",
			"voice number",
			"random",

			"envelope 1",
			"envelope 2",
			"LFO 1 (unipolar)",
			"LFO 1 (bipolar)",
			"LFO 2 (unipolar)",
			"LFO 2 (bipolar)",
			"global LFO (bipolar, monophonic)",
			"drift 1",
			"drift 2",
		};

		return sourceLabelMap;
	}

	static const std::array<std::string, nDestinations>& getDestinationLabels()
	{
		static const std::array<std::string, nDestinations> destinationLabelMap = {
			"envelope 1 Attack",
			"envelope 1 Decay",
			"envelope 1 Sustain",
			"envelope 1 Release",

			"envelope 2 Attack",
			"envelope 2 Decay",
			"envelope 2 Sustain",
			"envelope 2 Release",

			"LFO 1 frequency",
			"LFO 1 amount",

			"LFO 2 frequency",
			"LFO 2 amount",

			"global LFO frequency",
			"global LFO amount",

			"individual modulation 1",
			"individual modulation 2",
			"individual modulation 3",
			"individual modulation 4",

			"oscillator 1 glide",
			"oscillator 1 semitones",
			"oscillator 1 fine tune",
			"oscillator 1 shape",
			"oscillator 1 triangle phase / pulse width",

			"oscillator 2 glide offset",
			"oscillator 2 semitones",
			"oscillator 2 fine tune",
			"oscillator 2 shape",
			"oscillator 2 triangle phase / pulse width",

			"oscillator 1 to oscillator 2 FM amount",

			"filter 1 cutoff frequency",
			"filter 1 resonance",
			"filter 1 pan",

			"filter 2 cutoff frequency",
			"filter 2 resonance",
			"filter 2 pan",

			"filter routing: serial / parallel",

			"amp volume",

			"delay time",
			"delay feedback",
			"delay dry-wet mix",

			"oscillator 1",
			"oscillator 1 sub-oscillator",
			"ring modulator",
			"Noise",
			"oscillator 2",
			"external audio input",
		};

		return destinationLabelMap;
	}

	void configureUi()
	{
		const auto& sourceLabels = getSourceLabels();
		const auto& destinationLabels = getDestinationLabels();


		for (size_t i = 0; i < destinationLabels.size() - 2 * nMixChannels; i++)
		{
			if (activeSourceAssign)
			{
				std::string sourceLabel = sourceLabels[activeSourceAssign - 1];

				BipolarColorParamQuantity* param = configParam<BipolarColorParamQuantity>(ENV1_A_PARAM + i, -1.f, 1.f, 0.f,
						"Assign " + sourceLabel + " to " + destinationLabels[i],
						" %", 0, 100.);

				param->bipolar = true;
				param->color = SCHEME_BLUE;
			}
			else
			{
				std::string destinationLabel = destinationLabels[i];
				destinationLabel[0] = toupper(destinationLabel[0]);
				BipolarColorParamQuantity* param = configParam<BipolarColorParamQuantity>(ENV1_A_PARAM + i, 0.f, 1.f, 0.f, destinationLabel); // TODO unit label

				param->bipolar = false; // TODO must be true for some params
				param->color = SCHEME_GREEN;
			}
//			params[ENV1_A_PARAM + i].setValue(modMatrix[activeSourceAssign][ENV1_A_PARAM + i]);
		}

		for (size_t i = destinationLabels.size() - 2 * nMixChannels; i < destinationLabels.size() - nMixChannels; i++)
		{
			if (oscMixRouteActive)
			{
				if (activeSourceAssign)
				{
					std::string sourceLabel = sourceLabels[activeSourceAssign - 1];

					BipolarColorParamQuantity* param = configParam<BipolarColorParamQuantity>(ENV1_A_PARAM + i, -1.f, 1.f, 0.f,
							"Assign " + sourceLabel + " to " + destinationLabels[i] + " routing (filter 1 / filter 2)",
							" %", 0, 100.);

					param->bipolar = true;
					param->color = SCHEME_PURPLE;
				}
				else
				{
					std::string destinationLabel = destinationLabels[i];
					destinationLabel[0] = toupper(destinationLabel[0]);
					BipolarColorParamQuantity* param = configParam<BipolarColorParamQuantity>(ENV1_A_PARAM + i, -1.f, 1.f, 0.f,
							destinationLabel + " routing (filter 1 / filter 2)",
							" %", 0, 100.);

					param->bipolar = true;
					param->color = SCHEME_RED;
				}
//				params[ENV1_A_PARAM + i].setValue(modMatrix[activeSourceAssign][ENV1_A_PARAM + nMixChannels + i]);
			}
			else
			{
				if (activeSourceAssign)
				{
					std::string sourceLabel = sourceLabels[activeSourceAssign - 1];

					BipolarColorParamQuantity* param = configParam<BipolarColorParamQuantity>(ENV1_A_PARAM + i, -1.f, 1.f, 0.f,
							"Assign " + sourceLabel + " to " + destinationLabels[i] + " volume",
							" %", 0, 100.);

					param->bipolar = true;
					param->color = SCHEME_BLUE;
				}
				else
				{
					std::string destinationLabel = destinationLabels[i];
					destinationLabel[0] = toupper(destinationLabel[0]);
					BipolarColorParamQuantity* param = configParam<BipolarColorParamQuantity>(ENV1_A_PARAM + i, 0.f, 1.f, 0.f,
							destinationLabel + " volume",
							" %", 0, 100.);

					param->bipolar = false;
					param->color = SCHEME_GREEN;
				}
//				params[ENV1_A_PARAM + i].setValue(modMatrix[activeSourceAssign][ENV1_A_PARAM + i]);
			}
		}

		// set lights
		for (size_t i = 0; i < OSC_MIX_ROUTE_LIGHT; i++)
		{
			lights[i].setBrightness(i == activeSourceAssign - 1);
		}

		lights[OSC_MIX_ROUTE_LIGHT].setBrightness(oscMixRouteActive);
	}

	void process(const ProcessArgs& args) override {

		if (uiDivider.process())
		{
			channels = std::max(1, inputs[VOCT_INPUT].getChannels());

			// update activeSourceAssign and oscMixRouteActive
			size_t newActiveSourceAssign = 0;
			for (size_t i = 0; i < ENV1_A_PARAM; i++)
			{
				if (params[i].getValue())
				{
					newActiveSourceAssign = i + 1;
					break;
				}
			}

			bool newOscMixRouteActive = params[OSC_MIX_ROUTE_PARAM].getValue() > 0.5f;

			// adapt UI if  activeSourceAssign or oscMixRouteActive have changed
			if (activeSourceAssign != newActiveSourceAssign || oscMixRouteActive != newOscMixRouteActive)
			{
				activeSourceAssign = newActiveSourceAssign;
				oscMixRouteActive = newOscMixRouteActive;
				configureUi();
			}

			// update mod matrix elements
//			for (size_t i = 0; i < nDestinations - 2 * nMixChannels; i++)
//			{
//				modMatrix[activeSourceAssign][i] = params[ENV1_A_PARAM + i].getValue();
//			}
//
//			if (oscMixRouteActive)
//			{
//				for (size_t i = nDestinations - 2 * nMixChannels; i < nDestinations - nMixChannels; i++)
//				{
//					modMatrix[activeSourceAssign][i + nMixChannels] = params[ENV1_A_PARAM + i].getValue();
//				}
//			}
//			else
//			{
//				for (size_t i = nDestinations - 2 * nMixChannels; i < nDestinations - nMixChannels; i++)
//				{
//					modMatrix[activeSourceAssign][i] = params[ENV1_A_PARAM + i].getValue();
//				}
//			}


			// set non-modulatable parameters
		}

		if (modDivider.process())
		{
			// process modulation blocks

			// matrix multiplication

			// set modulated parameters

		}

		outputs[OUT_L_OUTPUT].setVoltage(oscMixRouteActive);
		outputs[OUT_R_OUTPUT].setVoltage(activeSourceAssign);
	}

//	json_t* dataToJson() override {
//		// TODO store mod matrix, mixLevels, mixFilterBalances
//	}
//
//	void dataFromJson(json_t* rootJ) override {
//		// TODO load mod matrix, mixLevels, mixFilterBalances
//	}
};


struct SynthWidget : ModuleWidget {
	SynthWidget(Synth* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/Synth.svg"), asset::plugin(pluginInstance, "res/Synth-dark.svg")));

	    addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<BlueLight>>>(mm2px(Vec(27.0, 8.557)), module, Synth::VOCT_ASSIGN_PARAM, Synth::VOCT_ASSIGN_LIGHT));
	    addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<BlueLight>>>(mm2px(Vec(27.0, 18.182)), module, Synth::GATE_ASSIGN_PARAM, Synth::GATE_ASSIGN_LIGHT));
	    addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<BlueLight>>>(mm2px(Vec(27.0, 27.807)), module, Synth::VELOCITY_ASSIGN_PARAM, Synth::VELOCITY_ASSIGN_LIGHT));
	    addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<BlueLight>>>(mm2px(Vec(27.0, 37.433)), module, Synth::AFTERTOUCH_ASSIGN_PARAM, Synth::AFTERTOUCH_ASSIGN_LIGHT));
	    addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<BlueLight>>>(mm2px(Vec(27.0, 47.058)), module, Synth::PITCH_WHEEL_ASSIGN_PARAM, Synth::PITCH_WHEEL_ASSIGN_LIGHT));
	    addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<BlueLight>>>(mm2px(Vec(27.0, 56.683)), module, Synth::MOD_WHEEL_ASSIGN_PARAM, Synth::MOD_WHEEL_ASSIGN_LIGHT));
	    addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<BlueLight>>>(mm2px(Vec(27.0, 66.309)), module, Synth::EXPRESSION_ASSIGN_PARAM, Synth::EXPRESSION_ASSIGN_LIGHT));
	    addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<BlueLight>>>(mm2px(Vec(27.0, 75.934)), module, Synth::INDIVIDUAL_MOD_1_ASSIGN_PARAM, Synth::INDIVIDUAL_MOD_1_ASSIGN_LIGHT));
	    addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<BlueLight>>>(mm2px(Vec(27.0, 85.56)), module, Synth::INDIVIDUAL_MOD_2_ASSIGN_PARAM, Synth::INDIVIDUAL_MOD_2_ASSIGN_LIGHT));
	    addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<BlueLight>>>(mm2px(Vec(27.0, 95.185)), module, Synth::VOICE_NR_ASSIGN_PARAM, Synth::VOICE_NR_ASSIGN_LIGHT));
	    addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<BlueLight>>>(mm2px(Vec(27.0, 104.81)), module, Synth::RANDOM_ASSIGN_PARAM, Synth::RANDOM_ASSIGN_LIGHT));

	    addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<BlueLight>>>(mm2px(Vec(127.5, 12.088)), module, Synth::ENV1_ASSIGN_PARAM, Synth::ENV1_ASSIGN_LIGHT));
	    addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<BlueLight>>>(mm2px(Vec(127.5, 37.188)), module, Synth::ENV2_ASSIGN_PARAM, Synth::ENV2_ASSIGN_LIGHT));
	    addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<BlueLight>>>(mm2px(Vec(210.0, 6.101)), module, Synth::LFO1_UNIPOLAR_ASSIGN_PARAM, Synth::LFO1_UNIPOLAR_ASSIGN_LIGHT));
	    addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<BlueLight>>>(mm2px(Vec(210.0, 17.662)), module, Synth::LFO1_BIPOLAR_ASSIGN_PARAM, Synth::LFO1_BIPOLAR_ASSIGN_LIGHT));
	    addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<BlueLight>>>(mm2px(Vec(210.0, 31.201)), module, Synth::LFO2_UNIPOLAR_ASSIGN_PARAM, Synth::LFO2_UNIPOLAR_ASSIGN_LIGHT));
	    addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<BlueLight>>>(mm2px(Vec(210.0, 42.762)), module, Synth::LFO2_BIPOLAR_ASSIGN_PARAM, Synth::LFO2_BIPOLAR_ASSIGN_LIGHT));
	    addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<BlueLight>>>(mm2px(Vec(263.0, 12.088)), module, Synth::GLOBAL_LFO_ASSIGN_PARAM, Synth::GLOBAL_LFO_ASSIGN_LIGHT));
	    addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<BlueLight>>>(mm2px(Vec(263.0, 33.843)), module, Synth::DRIFT_1_ASSIGN_PARAM, Synth::DRIFT_1_ASSIGN_LIGHT));
	    addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<BlueLight>>>(mm2px(Vec(263.0, 44.742)), module, Synth::DRIFT_2_ASSIGN_PARAM, Synth::DRIFT_2_ASSIGN_LIGHT));

	    addParam(createParamCentered<RoundBlackKnobWithArc>(mm2px(Vec(47.029, 12.088)), module, Synth::ENV1_A_PARAM));
	    addParam(createParamCentered<RoundBlackKnobWithArc>(mm2px(Vec(64.029, 12.088)), module, Synth::ENV1_D_PARAM));
	    addParam(createParamCentered<RoundBlackKnobWithArc>(mm2px(Vec(81.029, 12.088)), module, Synth::ENV1_S_PARAM));
	    addParam(createParamCentered<RoundBlackKnobWithArc>(mm2px(Vec(98.029, 12.088)), module, Synth::ENV1_R_PARAM));
	    addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(115.029, 12.088)), module, Synth::ENV1_VEL_PARAM));
	    addParam(createParamCentered<RoundBlackKnobWithArc>(mm2px(Vec(47.029, 37.188)), module, Synth::ENV2_A_PARAM));
	    addParam(createParamCentered<RoundBlackKnobWithArc>(mm2px(Vec(64.029, 37.188)), module, Synth::ENV2_D_PARAM));
	    addParam(createParamCentered<RoundBlackKnobWithArc>(mm2px(Vec(81.029, 37.188)), module, Synth::ENV2_S_PARAM));
	    addParam(createParamCentered<RoundBlackKnobWithArc>(mm2px(Vec(98.029, 37.188)), module, Synth::ENV2_R_PARAM));
	    addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(115.029, 37.188)), module, Synth::ENV2_VEL_PARAM));
	    addParam(createParamCentered<RoundBlackKnobWithArc>(mm2px(Vec(149.0, 12.088)), module, Synth::LFO1_FREQ_PARAM));
	    addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(166.0, 12.088)), module, Synth::LFO1_SHAPE_PARAM));
	    addParam(createParamCentered<RoundBlackKnobWithArc>(mm2px(Vec(183.0, 12.088)), module, Synth::LFO1_AMOUNT_PARAM));
	    addParam(createParamCentered<NKK>(mm2px(Vec(198.0, 12.088)), module, Synth::LFO1_MODE_PARAM));
	    addParam(createParamCentered<RoundBlackKnobWithArc>(mm2px(Vec(149.0, 37.188)), module, Synth::LFO2_FREQ_PARAM));
	    addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(166.0, 37.188)), module, Synth::LFO2_SHAPE_PARAM));
	    addParam(createParamCentered<RoundBlackKnobWithArc>(mm2px(Vec(183.0, 37.188)), module, Synth::LFO2_AMOUNT_PARAM));
	    addParam(createParamCentered<NKK>(mm2px(Vec(198.0, 37.188)), module, Synth::LFO2_MODE_PARAM));
	    addParam(createParamCentered<RoundBlackKnobWithArc>(mm2px(Vec(231.0, 12.088)), module, Synth::GLOBAL_LFO_FREQ_PARAM));
	    addParam(createParamCentered<RoundBlackKnobWithArc>(mm2px(Vec(248.0, 12.088)), module, Synth::GLOBAL_LFO_AMT_PARAM));
	    addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(231.0, 37.188)), module, Synth::DRIFT_RATE_PARAM));
	    addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(248.0, 37.188)), module, Synth::DRIFT_BALANCE_PARAM));
	    addParam(createParamCentered<RoundBlackKnobWithArc>(mm2px(Vec(284.578, 12.088)), module, Synth::INDIVIDUAL_MOD_OUT_1_PARAM));
	    addParam(createParamCentered<RoundBlackKnobWithArc>(mm2px(Vec(284.578, 37.188)), module, Synth::INDIVIDUAL_MOD_OUT_2_PARAM));
	    addParam(createParamCentered<RoundBlackKnobWithArc>(mm2px(Vec(284.578, 62.288)), module, Synth::INDIVIDUAL_MOD_OUT_3_PARAM));
	    addParam(createParamCentered<RoundBlackKnobWithArc>(mm2px(Vec(284.577, 87.388)), module, Synth::INDIVIDUAL_MOD_OUT_4_PARAM));
	    addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(44.864, 62.288)), module, Synth::OSC1_TUNE_OCT_PARAM));
	    addParam(createParamCentered<RoundBlackKnobWithArc>(mm2px(Vec(60.276, 62.288)), module, Synth::OSC1_TUNE_SEMI_PARAM));
	    addParam(createParamCentered<RoundBlackKnobWithArc>(mm2px(Vec(77.276, 62.288)), module, Synth::OSC1_TUNE_FINE_PARAM));
	    addParam(createParamCentered<RoundBlackKnobWithArc>(mm2px(Vec(98.335, 62.288)), module, Synth::OSC1_SHAPE_PARAM));
	    addParam(createParamCentered<RoundBlackKnobWithArc>(mm2px(Vec(119.039, 62.288)), module, Synth::OSC1_PW_PARAM));
	    addParam(createParamCentered<RoundBlackKnobWithArc>(mm2px(Vec(140.626, 62.288)), module, Synth::OSC1_VOL_PARAM));
	    addParam(createParamCentered<RoundBlackKnobWithArc>(mm2px(Vec(157.626, 62.288)), module, Synth::OSC1_SUB_VOL_PARAM));
	    addParam(createParamCentered<RoundBlackKnobWithArc>(mm2px(Vec(52.835, 87.388)), module, Synth::OSC1_TUNE_GLIDE_PARAM));
	    addParam(createParamCentered<RoundBlackKnobWithArc>(mm2px(Vec(69.835, 87.388)), module, Synth::OSC2_TUNE_GLIDE_PARAM));
	    addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(44.864, 112.488)), module, Synth::OSC2_TUNE_OCT_PARAM));
	    addParam(createParamCentered<RoundBlackKnobWithArc>(mm2px(Vec(60.276, 112.488)), module, Synth::OSC2_TUNE_SEMI_PARAM));
	    addParam(createParamCentered<RoundBlackKnobWithArc>(mm2px(Vec(77.276, 112.488)), module, Synth::OSC2_TUNE_FINE_PARAM));
	    addParam(createParamCentered<RoundBlackKnobWithArc>(mm2px(Vec(98.335, 112.488)), module, Synth::OSC2_SHAPE_PARAM));
	    addParam(createParamCentered<RoundBlackKnobWithArc>(mm2px(Vec(119.039, 112.488)), module, Synth::OSC2_PW_PARAM));
	    addParam(createParamCentered<RoundBlackKnobWithArc>(mm2px(Vec(140.626, 112.488)), module, Synth::OSC2_VOL_PARAM));
	    addParam(createParamCentered<VCVLightLatch<MediumSimpleLight<WhiteLight>>>(mm2px(Vec(95.441, 87.388)), module, Synth::OSC_SYNC_PARAM));
	    addParam(createParamCentered<RoundBlackKnobWithArc>(mm2px(Vec(109.004, 87.388)), module, Synth::OSC_FM_AMOUNT_PARAM));
	    addParam(createParamCentered<RoundBlackKnobWithArc>(mm2px(Vec(140.626, 87.388)), module, Synth::OSC_RM_VOL_PARAM));
	    addParam(createParamCentered<RoundBlackKnobWithArc>(mm2px(Vec(157.626, 87.388)), module, Synth::OSC_NOISE_VOL_PARAM));
	    addParam(createParamCentered<RoundBlackKnobWithArc>(mm2px(Vec(157.626, 112.488)), module, Synth::OSC_EXT_VOL_PARAM));
	    addParam(createParamCentered<VCVLightLatch<MediumSimpleLight<RedLight>>>(mm2px(Vec(126.9, 87.388)), module, Synth::OSC_MIX_ROUTE_PARAM));
	    addParam(createParamCentered<RoundBlackKnobWithArc>(mm2px(Vec(194.912, 62.288)), module, Synth::FILTER1_CUTOFF_PARAM));
	    addParam(createParamCentered<RoundBlackKnobWithArc>(mm2px(Vec(211.912, 62.288)), module, Synth::FILTER1_RESONANCE_PARAM));
	    addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(227.325, 62.288)), module, Synth::FILTER1_TYPE_PARAM));
	    addParam(createParamCentered<RoundBlackKnobWithArc>(mm2px(Vec(259.737, 62.288)), module, Synth::FILTER1_PAN_PARAM));
	    addParam(createParamCentered<NKK>(mm2px(Vec(180.413, 75.237)), module, Synth::FILTER2_CUTOFF_MODE_PARAM));
	    addParam(createParamCentered<RoundBlackKnobWithArc>(mm2px(Vec(194.912, 87.388)), module, Synth::FILTER2_CUTOFF_PARAM));
	    addParam(createParamCentered<RoundBlackKnobWithArc>(mm2px(Vec(211.912, 87.388)), module, Synth::FILTER2_RESONANCE_PARAM));
	    addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(227.325, 87.388)), module, Synth::FILTER2_TYPE_PARAM));
	    addParam(createParamCentered<RoundBlackKnobWithArc>(mm2px(Vec(259.737, 87.388)), module, Synth::FILTER2_PAN_PARAM));
	    addParam(createParamCentered<RoundBlackKnobWithArc>(mm2px(Vec(242.737, 75.237)), module, Synth::FILTER_SERIAL_PARALLEL_PARAM));
	    addParam(createParamCentered<RoundBlackKnobWithArc>(mm2px(Vec(183.587, 112.488)), module, Synth::AMP_VOL_PARAM));
	    addParam(createParamCentered<VCVLightLatch<MediumSimpleLight<WhiteLight>>>(mm2px(Vec(209.587, 112.488)), module, Synth::DELAY_TAP_PARAM));
	    addParam(createParamCentered<RoundBlackKnobWithArc>(mm2px(Vec(225.65, 112.626)), module, Synth::DELAY_TIME_PARAM));
	    addParam(createParamCentered<RoundBlackKnobWithArc>(mm2px(Vec(242.65, 112.626)), module, Synth::DELAY_FEEDBACK_PARAM));
	    addParam(createParamCentered<RoundBlackKnobWithArc>(mm2px(Vec(259.737, 112.557)), module, Synth::DELAY_MIX_PARAM));

	    addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(7.0, 8.557)), module, Synth::VOCT_INPUT));
	    addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(7.0, 18.182)), module, Synth::GATE_INPUT));
	    addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(7.0, 27.807)), module, Synth::VELOCITY_INPUT));
	    addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(7.0, 37.433)), module, Synth::AFTERTOUCH_INPUT));
	    addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(7.0, 47.058)), module, Synth::PITCH_WHEEL_INPUT));
	    addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(7.0, 56.683)), module, Synth::MOD_WHEEL_INPUT));
	    addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(7.0, 66.309)), module, Synth::EXPRESSION_INPUT));
	    addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(7.0, 75.934)), module, Synth::INDIVIDUAL_MOD_1_INPUT));
	    addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(7.0, 85.56)), module, Synth::INDIVIDUAL_MOD_2_INPUT));
	    addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(8.983, 116.799)), module, Synth::RETRIGGER_INPUT));
	    addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(24.823, 116.799)), module, Synth::EXT_INPUT));

		addOutput(createOutputCentered<ThemedPJ301MPort>(mm2px(Vec(298.976, 12.088)), module, Synth::INDIVIDUAL_MOD_1_OUTPUT));
		addOutput(createOutputCentered<ThemedPJ301MPort>(mm2px(Vec(298.976, 37.188)), module, Synth::INDIVIDUAL_MOD_2_OUTPUT));
		addOutput(createOutputCentered<ThemedPJ301MPort>(mm2px(Vec(298.976, 62.288)), module, Synth::INDIVIDUAL_MOD_3_OUTPUT));
		addOutput(createOutputCentered<ThemedPJ301MPort>(mm2px(Vec(298.975, 87.388)), module, Synth::INDIVIDUAL_MOD_4_OUTPUT));
		addOutput(createOutputCentered<ThemedPJ301MPort>(mm2px(Vec(279.323, 112.557)), module, Synth::OUT_L_OUTPUT));
		addOutput(createOutputCentered<ThemedPJ301MPort>(mm2px(Vec(294.477, 112.557)), module, Synth::OUT_R_OUTPUT));
	}

	void appendContextMenu(Menu* menu) override {
		Synth* module = getModule<Synth>();

		menu->addChild(new MenuSeparator);

//		menu->addChild(createIndexSubmenuItem("Quality", {"draft", "ok", "good", "ultra"},
//			[=]() {
//				// TODO
//			},
//			[=](int mode) {
//				// TODO
//			}
//		));

		// latch buttons
	}
};


Model* modelSynth = createModel<Synth, SynthWidget>("Synth");

}
