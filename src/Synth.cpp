#include "plugin.hpp"
#include "components/componentLibrary.hpp"

#include "blocks/ADSRBlock.hpp"
#include "blocks/FilterBlock.hpp"
#include "blocks/OscillatorsBlock.hpp"

namespace musx {

using namespace rack;

struct Synth : Module {
	enum ParamId {
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
		ENV1_A_PARAM,
		ENV1_D_PARAM,
		ENV1_S_PARAM,
		ENV1_R_PARAM,
		ENV1_VEL_PARAM,
		ENV1_ASSIGN_PARAM,
		ENV2_A_PARAM,
		ENV2_D_PARAM,
		ENV2_S_PARAM,
		ENV2_R_PARAM,
		ENV2_VEL_PARAM,
		ENV2_ASSIGN_PARAM,
		LFO1_UNIPOLAR_ASSIGN_PARAM,
		LFO1_FREQ_PARAM,
		LFO1_SHAPE_PARAM,
		LFO1_AMOUNT_PARAM,
		LFO1_MODE_PARAM,
		LFO1_BIPOLAR_ASSIGN_PARAM,
		LFO2_UNIPOLAR_ASSIGN_PARAM,
		LFO2_FREQ_PARAM,
		LFO2_SHAPE_PARAM,
		LFO2_AMOUNT_PARAM,
		LFO2_MODE_PARAM,
		LFO2_BIPOLAR_ASSIGN_PARAM,
		GLOBAL_LFO_FREQ_PARAM,
		GLOBAL_LFO_AMT_PARAM,
		GLOBAL_LFO_ASSIGN_PARAM,
		DRIFT_1_ASSIGN_PARAM,
		DRIFT_RATE_PARAM,
		DRIFT_BALANCE_PARAM,
		DRIFT_2_ASSIGN_PARAM,
		INDIVIDUAL_MOD_OUT_1_PARAM,
		INDIVIDUAL_MOD_OUT_2_PARAM,
		INDIVIDUAL_MOD_OUT_3_PARAM,
		INDIVIDUAL_MOD_OUT_4_PARAM,
		OSC1_TUNE_OCT_PARAM,
		OSC1_TUNE_SEMI_PARAM,
		OSC1_TUNE_FINE_PARAM,
		OSC1_SHAPE_PARAM,
		OSC1_PW_PARAM,
		OSC1_VOL_PARAM,
		OSC1_SUB_VOL_PARAM,
		OSC1_TUNE_GLIDE_PARAM,
		OSC2_TUNE_GLIDE_PARAM,
		OSC2_TUNE_OCT_PARAM,
		OSC2_TUNE_SEMI_PARAM,
		OSC2_TUNE_FINE_PARAM,
		OSC2_SHAPE_PARAM,
		OSC2_PW_PARAM,
		OSC2_VOL_PARAM,
		OSC_MIX_ROUTE_PARAM,
		OSC_SYNC_PARAM,
		OSC_FM_AMOUNT_PARAM,
		OSC_RM_VOL_PARAM,
		OSC_NOISE_VOL_PARAM,
		OSC_EXT_VOL_PARAM,
		FILTER1_CUTOFF_PARAM,
		FILTER1_RESONANCE_PARAM,
		FILTER1_TYPE_PARAM,
		FILTER1_PAN_PARAM,
		FILTER2_CUTOFF_MODE_PARAM,
		FILTER2_CUTOFF_PARAM,
		FILTER2_RESONANCE_PARAM,
		FILTER2_TYPE_PARAM,
		FILTER2_PAN_PARAM,
		FILTER_SERIAL_PARALLEL_PARAM,
		AMP_VOL_PARAM,
		DELAY_TAP_PARAM,
		DELAY_TIME_PARAM,
		DELAY_FEEDBACK_PARAM,
		DELAY_MIX_PARAM,
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
		LIGHTS_LEN
	};

	//
	int channels = 1;
	BipolarColorParamQuantity* modulatableParamQtys[PARAMS_LEN];

	// over/-undersampling
	static const size_t maxOversamplingRate = 8;

	dsp::ClockDivider uiDivider;
	dsp::ClockDivider modDivider;

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
		configParam(VOCT_ASSIGN_PARAM, 0.f, 1.f, 0.f, "");
		configParam(GATE_ASSIGN_PARAM, 0.f, 1.f, 0.f, "");
		configParam(VELOCITY_ASSIGN_PARAM, 0.f, 1.f, 0.f, "");
		configParam(AFTERTOUCH_ASSIGN_PARAM, 0.f, 1.f, 0.f, "");
		configParam(PITCH_WHEEL_ASSIGN_PARAM, 0.f, 1.f, 0.f, "");
		configParam(MOD_WHEEL_ASSIGN_PARAM, 0.f, 1.f, 0.f, "");
		configParam(EXPRESSION_ASSIGN_PARAM, 0.f, 1.f, 0.f, "");
		configParam(INDIVIDUAL_MOD_1_ASSIGN_PARAM, 0.f, 1.f, 0.f, "");
		configParam(INDIVIDUAL_MOD_2_ASSIGN_PARAM, 0.f, 1.f, 0.f, "");
		configParam(VOICE_NR_ASSIGN_PARAM, 0.f, 1.f, 0.f, "");
		configParam(RANDOM_ASSIGN_PARAM, 0.f, 1.f, 0.f, "");
		modulatableParamQtys[ENV1_A_PARAM] = configParam<BipolarColorParamQuantity>(ENV1_A_PARAM, 0.f, 1.f, 0.f, "");
		modulatableParamQtys[ENV1_D_PARAM] = configParam<BipolarColorParamQuantity>(ENV1_D_PARAM, 0.f, 1.f, 0.f, "");
		modulatableParamQtys[ENV1_S_PARAM] = configParam<BipolarColorParamQuantity>(ENV1_S_PARAM, 0.f, 1.f, 0.f, "");
		modulatableParamQtys[ENV1_R_PARAM] = configParam<BipolarColorParamQuantity>(ENV1_R_PARAM, 0.f, 1.f, 0.f, "");
		configParam(ENV1_VEL_PARAM, 0.f, 1.f, 0.f, "");
		configParam(ENV1_ASSIGN_PARAM, 0.f, 1.f, 0.f, "");
		modulatableParamQtys[ENV2_A_PARAM] = configParam<BipolarColorParamQuantity>(ENV2_A_PARAM, 0.f, 1.f, 0.f, "");
		modulatableParamQtys[ENV2_D_PARAM] = configParam<BipolarColorParamQuantity>(ENV2_D_PARAM, 0.f, 1.f, 0.f, "");
		modulatableParamQtys[ENV2_S_PARAM] = configParam<BipolarColorParamQuantity>(ENV2_S_PARAM, 0.f, 1.f, 0.f, "");
		modulatableParamQtys[ENV2_R_PARAM] = configParam<BipolarColorParamQuantity>(ENV2_R_PARAM, 0.f, 1.f, 0.f, "");
		configParam(ENV2_VEL_PARAM, 0.f, 1.f, 0.f, "");
		configParam(ENV2_ASSIGN_PARAM, 0.f, 1.f, 0.f, "");
		configParam(LFO1_UNIPOLAR_ASSIGN_PARAM, 0.f, 1.f, 0.f, "");
		modulatableParamQtys[LFO1_FREQ_PARAM] = configParam<BipolarColorParamQuantity>(LFO1_FREQ_PARAM, 0.f, 1.f, 0.f, "");
		configParam(LFO1_SHAPE_PARAM, 0.f, 1.f, 0.f, "");
		modulatableParamQtys[LFO1_AMOUNT_PARAM] = configParam<BipolarColorParamQuantity>(LFO1_AMOUNT_PARAM, 0.f, 1.f, 0.f, "");
		configParam(LFO1_MODE_PARAM, 0.f, 1.f, 0.f, "");
		configParam(LFO1_BIPOLAR_ASSIGN_PARAM, 0.f, 1.f, 0.f, "");
		configParam(LFO2_UNIPOLAR_ASSIGN_PARAM, 0.f, 1.f, 0.f, "");
		modulatableParamQtys[LFO2_FREQ_PARAM] = configParam<BipolarColorParamQuantity>(LFO2_FREQ_PARAM, 0.f, 1.f, 0.f, "");
		configParam(LFO2_SHAPE_PARAM, 0.f, 1.f, 0.f, "");
		modulatableParamQtys[LFO2_AMOUNT_PARAM] = configParam<BipolarColorParamQuantity>(LFO2_AMOUNT_PARAM, 0.f, 1.f, 0.f, "");
		configParam(LFO2_MODE_PARAM, 0.f, 1.f, 0.f, "");
		configParam(LFO2_BIPOLAR_ASSIGN_PARAM, 0.f, 1.f, 0.f, "");
		modulatableParamQtys[GLOBAL_LFO_FREQ_PARAM] = configParam<BipolarColorParamQuantity>(GLOBAL_LFO_FREQ_PARAM, 0.f, 1.f, 0.f, "");
		modulatableParamQtys[GLOBAL_LFO_AMT_PARAM] = configParam<BipolarColorParamQuantity>(GLOBAL_LFO_AMT_PARAM, 0.f, 1.f, 0.f, "");
		configParam(GLOBAL_LFO_ASSIGN_PARAM, 0.f, 1.f, 0.f, "");
		configParam(DRIFT_1_ASSIGN_PARAM, 0.f, 1.f, 0.f, "");
		configParam(DRIFT_RATE_PARAM, 0.f, 1.f, 0.f, "");
		configParam(DRIFT_BALANCE_PARAM, 0.f, 1.f, 0.f, "");
		configParam(DRIFT_2_ASSIGN_PARAM, 0.f, 1.f, 0.f, "");
		modulatableParamQtys[INDIVIDUAL_MOD_OUT_1_PARAM] = configParam<BipolarColorParamQuantity>(INDIVIDUAL_MOD_OUT_1_PARAM, 0.f, 1.f, 0.f, "");
		modulatableParamQtys[INDIVIDUAL_MOD_OUT_2_PARAM] = configParam<BipolarColorParamQuantity>(INDIVIDUAL_MOD_OUT_2_PARAM, 0.f, 1.f, 0.f, "");
		modulatableParamQtys[INDIVIDUAL_MOD_OUT_3_PARAM] = configParam<BipolarColorParamQuantity>(INDIVIDUAL_MOD_OUT_3_PARAM, 0.f, 1.f, 0.f, "");
		modulatableParamQtys[INDIVIDUAL_MOD_OUT_4_PARAM] = configParam<BipolarColorParamQuantity>(INDIVIDUAL_MOD_OUT_4_PARAM, 0.f, 1.f, 0.f, "");
		configParam(OSC1_TUNE_OCT_PARAM, 0.f, 1.f, 0.f, "");
		modulatableParamQtys[OSC1_TUNE_SEMI_PARAM] = configParam<BipolarColorParamQuantity>(OSC1_TUNE_SEMI_PARAM, 0.f, 1.f, 0.f, "");
		modulatableParamQtys[OSC1_TUNE_FINE_PARAM] = configParam<BipolarColorParamQuantity>(OSC1_TUNE_FINE_PARAM, 0.f, 1.f, 0.f, "");
		modulatableParamQtys[OSC1_SHAPE_PARAM] = configParam<BipolarColorParamQuantity>(OSC1_SHAPE_PARAM, 0.f, 1.f, 0.f, "");
		modulatableParamQtys[OSC1_PW_PARAM] = configParam<BipolarColorParamQuantity>(OSC1_PW_PARAM, 0.f, 1.f, 0.f, "");
		modulatableParamQtys[OSC1_VOL_PARAM] = configParam<BipolarColorParamQuantity>(OSC1_VOL_PARAM, 0.f, 1.f, 0.f, "");
		modulatableParamQtys[OSC1_SUB_VOL_PARAM] = configParam<BipolarColorParamQuantity>(OSC1_SUB_VOL_PARAM, 0.f, 1.f, 0.f, "");
		modulatableParamQtys[OSC1_TUNE_GLIDE_PARAM] = configParam<BipolarColorParamQuantity>(OSC1_TUNE_GLIDE_PARAM, 0.f, 1.f, 0.f, "");
		modulatableParamQtys[OSC2_TUNE_GLIDE_PARAM] = configParam<BipolarColorParamQuantity>(OSC2_TUNE_GLIDE_PARAM, 0.f, 1.f, 0.f, "");
		configParam(OSC2_TUNE_OCT_PARAM, 0.f, 1.f, 0.f, "");
		modulatableParamQtys[OSC2_TUNE_SEMI_PARAM] = configParam<BipolarColorParamQuantity>(OSC2_TUNE_SEMI_PARAM, 0.f, 1.f, 0.f, "");
		modulatableParamQtys[OSC2_TUNE_FINE_PARAM] = configParam<BipolarColorParamQuantity>(OSC2_TUNE_FINE_PARAM, 0.f, 1.f, 0.f, "");
		modulatableParamQtys[OSC2_SHAPE_PARAM] = configParam<BipolarColorParamQuantity>(OSC2_SHAPE_PARAM, 0.f, 1.f, 0.f, "");
		modulatableParamQtys[OSC2_PW_PARAM] = configParam<BipolarColorParamQuantity>(OSC2_PW_PARAM, 0.f, 1.f, 0.f, "");
		modulatableParamQtys[OSC2_VOL_PARAM] = configParam<BipolarColorParamQuantity>(OSC2_VOL_PARAM, 0.f, 1.f, 0.f, "");
		configParam(OSC_SYNC_PARAM, 0.f, 1.f, 0.f, "");
		modulatableParamQtys[OSC_FM_AMOUNT_PARAM] = configParam<BipolarColorParamQuantity>(OSC_FM_AMOUNT_PARAM, 0.f, 1.f, 0.f, "");
		modulatableParamQtys[OSC_RM_VOL_PARAM] = configParam<BipolarColorParamQuantity>(OSC_RM_VOL_PARAM, 0.f, 1.f, 0.f, "");
		modulatableParamQtys[OSC_NOISE_VOL_PARAM] = configParam<BipolarColorParamQuantity>(OSC_NOISE_VOL_PARAM, 0.f, 1.f, 0.f, "");
		modulatableParamQtys[OSC_EXT_VOL_PARAM] = configParam<BipolarColorParamQuantity>(OSC_EXT_VOL_PARAM, 0.f, 1.f, 0.f, "");
		configSwitch(OSC_MIX_ROUTE_PARAM, 0.f, 1.f, 0.f, "");
		modulatableParamQtys[FILTER1_CUTOFF_PARAM] = configParam<BipolarColorParamQuantity>(FILTER1_CUTOFF_PARAM, 0.f, 1.f, 0.f, "");
		modulatableParamQtys[FILTER1_RESONANCE_PARAM] = configParam<BipolarColorParamQuantity>(FILTER1_RESONANCE_PARAM, 0.f, 1.f, 0.f, "");
		configParam(FILTER1_TYPE_PARAM, 0.f, 1.f, 0.f, "");
		modulatableParamQtys[FILTER1_PAN_PARAM] = configParam<BipolarColorParamQuantity>(FILTER1_PAN_PARAM, 0.f, 1.f, 0.f, "");
		configParam(FILTER2_CUTOFF_MODE_PARAM, 0.f, 1.f, 0.f, "");
		modulatableParamQtys[FILTER2_CUTOFF_PARAM] = configParam<BipolarColorParamQuantity>(FILTER2_CUTOFF_PARAM, 0.f, 1.f, 0.f, "");
		modulatableParamQtys[FILTER2_RESONANCE_PARAM] = configParam<BipolarColorParamQuantity>(FILTER2_RESONANCE_PARAM, 0.f, 1.f, 0.f, "");
		configParam(FILTER2_TYPE_PARAM, 0.f, 1.f, 0.f, "");
		modulatableParamQtys[FILTER2_PAN_PARAM] = configParam<BipolarColorParamQuantity>(FILTER2_PAN_PARAM, 0.f, 1.f, 0.f, "");
		modulatableParamQtys[FILTER_SERIAL_PARALLEL_PARAM] = configParam<BipolarColorParamQuantity>(FILTER_SERIAL_PARALLEL_PARAM, 0.f, 1.f, 0.f, "");
		modulatableParamQtys[AMP_VOL_PARAM] = configParam<BipolarColorParamQuantity>(AMP_VOL_PARAM, 0.f, 1.f, 0.f, "");
		configParam(DELAY_TAP_PARAM, 0.f, 1.f, 0.f, "");
		modulatableParamQtys[DELAY_TIME_PARAM] = configParam<BipolarColorParamQuantity>(DELAY_TIME_PARAM, 0.f, 1.f, 0.f, "");
		modulatableParamQtys[DELAY_FEEDBACK_PARAM] = configParam<BipolarColorParamQuantity>(DELAY_FEEDBACK_PARAM, 0.f, 1.f, 0.f, "");
		modulatableParamQtys[DELAY_MIX_PARAM] = configParam<BipolarColorParamQuantity>(DELAY_MIX_PARAM, 0.f, 1.f, 0.f, "");
		configInput(VOCT_INPUT, "");
		configInput(GATE_INPUT, "");
		configInput(VELOCITY_INPUT, "");
		configInput(AFTERTOUCH_INPUT, "");
		configInput(PITCH_WHEEL_INPUT, "");
		configInput(MOD_WHEEL_INPUT, "");
		configInput(EXPRESSION_INPUT, "");
		configInput(INDIVIDUAL_MOD_1_INPUT, "");
		configInput(INDIVIDUAL_MOD_2_INPUT, "");
		configInput(RETRIGGER_INPUT, "");
		configInput(EXT_INPUT, "");
		configOutput(INDIVIDUAL_MOD_1_OUTPUT, "");
		configOutput(INDIVIDUAL_MOD_2_OUTPUT, "");
		configOutput(INDIVIDUAL_MOD_3_OUTPUT, "");
		configOutput(INDIVIDUAL_MOD_4_OUTPUT, "");
		configOutput(OUT_L_OUTPUT, "");
		configOutput(OUT_R_OUTPUT, "");

		uiDivider.setDivision(32);
		modDivider.setDivision(2);
	}

	void process(const ProcessArgs& args) override {
		if (uiDivider.process())
		{
			channels = std::max(1, inputs[VOCT_INPUT].getChannels());

			// adapt UI if OSC_MIX_ROUTE_PARAM button is pressed
			modulatableParamQtys[OSC1_VOL_PARAM]->bipolar = params[OSC_MIX_ROUTE_PARAM].getValue() > 0.5;
			modulatableParamQtys[OSC1_VOL_PARAM]->color = params[OSC_MIX_ROUTE_PARAM].getValue() > 0.5 ? nvgRGB(255, 0, 0) : nvgRGB(0, 255, 0);


			// adapt UI if assign button is pressed

			// update mod matrix elements

			// set non-modulatable parameters
		}

		if (modDivider.process())
		{
			// process modulation blocks

			// matrix multiplication

			// set modulated parameters

		}
	}
};


struct SynthWidget : ModuleWidget {
	SynthWidget(Synth* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/Synth.svg"), asset::plugin(pluginInstance, "res/Synth-dark.svg")));

	    addParam(createParamCentered<VCVLightLatch<MediumSimpleLight<BlueLight>>>(mm2px(Vec(27.0, 8.557)), module, Synth::VOCT_ASSIGN_PARAM));
	    addParam(createParamCentered<VCVLightLatch<MediumSimpleLight<BlueLight>>>(mm2px(Vec(27.0, 18.182)), module, Synth::GATE_ASSIGN_PARAM));
	    addParam(createParamCentered<VCVLightLatch<MediumSimpleLight<BlueLight>>>(mm2px(Vec(27.0, 27.807)), module, Synth::VELOCITY_ASSIGN_PARAM));
	    addParam(createParamCentered<VCVLightLatch<MediumSimpleLight<BlueLight>>>(mm2px(Vec(27.0, 37.433)), module, Synth::AFTERTOUCH_ASSIGN_PARAM));
	    addParam(createParamCentered<VCVLightLatch<MediumSimpleLight<BlueLight>>>(mm2px(Vec(27.0, 47.058)), module, Synth::PITCH_WHEEL_ASSIGN_PARAM));
	    addParam(createParamCentered<VCVLightLatch<MediumSimpleLight<BlueLight>>>(mm2px(Vec(27.0, 56.683)), module, Synth::MOD_WHEEL_ASSIGN_PARAM));
	    addParam(createParamCentered<VCVLightLatch<MediumSimpleLight<BlueLight>>>(mm2px(Vec(27.0, 66.309)), module, Synth::EXPRESSION_ASSIGN_PARAM));
	    addParam(createParamCentered<VCVLightLatch<MediumSimpleLight<BlueLight>>>(mm2px(Vec(27.0, 75.934)), module, Synth::INDIVIDUAL_MOD_1_ASSIGN_PARAM));
	    addParam(createParamCentered<VCVLightLatch<MediumSimpleLight<BlueLight>>>(mm2px(Vec(27.0, 85.56)), module, Synth::INDIVIDUAL_MOD_2_ASSIGN_PARAM));
	    addParam(createParamCentered<VCVLightLatch<MediumSimpleLight<BlueLight>>>(mm2px(Vec(27.0, 95.185)), module, Synth::VOICE_NR_ASSIGN_PARAM));
	    addParam(createParamCentered<VCVLightLatch<MediumSimpleLight<BlueLight>>>(mm2px(Vec(27.0, 104.81)), module, Synth::RANDOM_ASSIGN_PARAM));
	    addParam(createParamCentered<RoundBlackKnobWithArc>(mm2px(Vec(47.029, 12.088)), module, Synth::ENV1_A_PARAM));
	    addParam(createParamCentered<RoundBlackKnobWithArc>(mm2px(Vec(64.029, 12.088)), module, Synth::ENV1_D_PARAM));
	    addParam(createParamCentered<RoundBlackKnobWithArc>(mm2px(Vec(81.029, 12.088)), module, Synth::ENV1_S_PARAM));
	    addParam(createParamCentered<RoundBlackKnobWithArc>(mm2px(Vec(98.029, 12.088)), module, Synth::ENV1_R_PARAM));
	    addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(115.029, 12.088)), module, Synth::ENV1_VEL_PARAM));
	    addParam(createParamCentered<VCVLightLatch<MediumSimpleLight<BlueLight>>>(mm2px(Vec(127.5, 12.088)), module, Synth::ENV1_ASSIGN_PARAM));
	    addParam(createParamCentered<RoundBlackKnobWithArc>(mm2px(Vec(47.029, 37.188)), module, Synth::ENV2_A_PARAM));
	    addParam(createParamCentered<RoundBlackKnobWithArc>(mm2px(Vec(64.029, 37.188)), module, Synth::ENV2_D_PARAM));
	    addParam(createParamCentered<RoundBlackKnobWithArc>(mm2px(Vec(81.029, 37.188)), module, Synth::ENV2_S_PARAM));
	    addParam(createParamCentered<RoundBlackKnobWithArc>(mm2px(Vec(98.029, 37.188)), module, Synth::ENV2_R_PARAM));
	    addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(115.029, 37.188)), module, Synth::ENV2_VEL_PARAM));
	    addParam(createParamCentered<VCVLightLatch<MediumSimpleLight<BlueLight>>>(mm2px(Vec(127.5, 37.188)), module, Synth::ENV2_ASSIGN_PARAM));
	    addParam(createParamCentered<VCVLightLatch<MediumSimpleLight<BlueLight>>>(mm2px(Vec(210.0, 6.101)), module, Synth::LFO1_UNIPOLAR_ASSIGN_PARAM));
	    addParam(createParamCentered<RoundBlackKnobWithArc>(mm2px(Vec(149.0, 12.088)), module, Synth::LFO1_FREQ_PARAM));
	    addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(166.0, 12.088)), module, Synth::LFO1_SHAPE_PARAM));
	    addParam(createParamCentered<RoundBlackKnobWithArc>(mm2px(Vec(183.0, 12.088)), module, Synth::LFO1_AMOUNT_PARAM));
	    addParam(createParamCentered<NKK>(mm2px(Vec(198.0, 12.088)), module, Synth::LFO1_MODE_PARAM));
	    addParam(createParamCentered<VCVLightLatch<MediumSimpleLight<BlueLight>>>(mm2px(Vec(210.0, 17.662)), module, Synth::LFO1_BIPOLAR_ASSIGN_PARAM));
	    addParam(createParamCentered<VCVLightLatch<MediumSimpleLight<BlueLight>>>(mm2px(Vec(210.0, 31.201)), module, Synth::LFO2_UNIPOLAR_ASSIGN_PARAM));
	    addParam(createParamCentered<RoundBlackKnobWithArc>(mm2px(Vec(149.0, 37.188)), module, Synth::LFO2_FREQ_PARAM));
	    addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(166.0, 37.188)), module, Synth::LFO2_SHAPE_PARAM));
	    addParam(createParamCentered<RoundBlackKnobWithArc>(mm2px(Vec(183.0, 37.188)), module, Synth::LFO2_AMOUNT_PARAM));
	    addParam(createParamCentered<NKK>(mm2px(Vec(198.0, 37.188)), module, Synth::LFO2_MODE_PARAM));
	    addParam(createParamCentered<VCVLightLatch<MediumSimpleLight<BlueLight>>>(mm2px(Vec(210.0, 42.762)), module, Synth::LFO2_BIPOLAR_ASSIGN_PARAM));
	    addParam(createParamCentered<RoundBlackKnobWithArc>(mm2px(Vec(231.0, 12.088)), module, Synth::GLOBAL_LFO_FREQ_PARAM));
	    addParam(createParamCentered<RoundBlackKnobWithArc>(mm2px(Vec(248.0, 12.088)), module, Synth::GLOBAL_LFO_AMT_PARAM));
	    addParam(createParamCentered<VCVLightLatch<MediumSimpleLight<BlueLight>>>(mm2px(Vec(263.0, 12.088)), module, Synth::GLOBAL_LFO_ASSIGN_PARAM));
	    addParam(createParamCentered<VCVLightLatch<MediumSimpleLight<BlueLight>>>(mm2px(Vec(263.0, 33.843)), module, Synth::DRIFT_1_ASSIGN_PARAM));
	    addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(231.0, 37.188)), module, Synth::DRIFT_RATE_PARAM));
	    addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(248.0, 37.188)), module, Synth::DRIFT_BALANCE_PARAM));
	    addParam(createParamCentered<VCVLightLatch<MediumSimpleLight<BlueLight>>>(mm2px(Vec(263.0, 44.742)), module, Synth::DRIFT_2_ASSIGN_PARAM));
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
