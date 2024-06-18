#include "plugin.hpp"
#include "components/componentLibrary.hpp"

namespace musx {

using namespace rack;

struct Synth : Module {
	enum ParamId {
		// mod in
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

		// env
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

		// lfo
		LFO1_FREQ_PARAM,
		LFO1_SHAPE_PARAM,
		LFO1_AMOUNT_PARAM,
		LFO1_MODE_PARAM,
		LFO1_UNIPOLAR_ASSIGN_PARAM,
		LFO1_BIPOLAR_ASSIGN_PARAM,

		LFO2_FREQ_PARAM,
		LFO2_SHAPE_PARAM,
		LFO2_AMOUNT_PARAM,
		LFO2_MODE_PARAM,
		LFO2_UNIPOLAR_ASSIGN_PARAM,
		LFO2_BIPOLAR_ASSIGN_PARAM,

		GLOBAL_LFO_FREQ_PARAM,
		GLOBAL_LFO_AMT_PARAM,
		GLOBAL_LFO_ASSIGN_PARAM,

		DRIFT_RATE_PARAM,
		DRIFT_BALANCE_PARAM,
		DRIFT_1_ASSIGN_PARAM,
		DRIFT_2_ASSIGN_PARAM,

		// mod out
		INDIVIDUAL_MOD_OUT_1_PARAM,
		INDIVIDUAL_MOD_OUT_2_PARAM,
		INDIVIDUAL_MOD_OUT_3_PARAM,
		INDIVIDUAL_MOD_OUT_4_PARAM,

		// osc
		OSC1_GLIDE_PARAM,
		OSC1_OCT_PARAM,
		OSC1_SEMI_PARAM,
		OSC1_FINE_PARAM,
		OSC1_SHAPE_PARAM,
		OSC1_PW_PARAM,
		OSC1_VOL_PARAM,
		OSC1_SUB_VOL_PARAM,

		OSC2_GLIDE_PARAM,
		OSC2_OCT_PARAM,
		OSC2_SEMI_PARAM,
		OSC2_FINE_PARAM,
		OSC2_SHAPE_PARAM,
		OSC2_PW_PARAM,
		OSC2_VOL_PARAM,

		OSC_SYNC_PARAM,
		OSC_FM_PARAM,
		OSC_RM_PARAM,

		OSC_NOISE_PARAM,
		OSC_EXT_VOL_PARAM,

		// filter
		FILTER1_INPUT_ASSIGN_PARAM,
		FILTER1_CUTOFF_PARAM,
		FILTER1_RESONANCE_PARAM,
		FILTER1_FM_ASSIGN_PARAM,
		FILTER1_FM_PARAM,
		FILTER1_TYPE_PARAM,
		FILTER1_VOL_PARAM,

		FILTER2_INPUT_ASSIGN_PARAM,
		FILTER2_CUTOFF_PARAM,
		FILTER2_RESONANCE_PARAM,
		FILTER2_FM_ASSIGN_PARAM,
		FILTER2_FM_PARAM,
		FILTER2_TYPE_PARAM,
		FILTER2_VOL_PARAM,

		// amp
		AMP_INPUT_ASSIGN_PARAM,
		AMP_VOL_PARAM,
		AMP_AM_ASSIGN_PARAM,
		AMP_AM_PARAM,
		AMP_PAN_ASSIGN_PARAM,
		AMP_PAN_PARAM,

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

	Synth() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configParam(VOCT_ASSIGN_PARAM, 0.f, 1.f, 0.f, "");
		configParam<BipolarColorParamQuantity>(ENV1_A_PARAM, 0.f, 1.f, 0.f, "");
		configParam<BipolarColorParamQuantity>(ENV1_D_PARAM, 0.f, 1.f, 0.f, "");
		configParam<BipolarColorParamQuantity>(ENV1_S_PARAM, 0.f, 1.f, 0.f, "");
		configParam<BipolarColorParamQuantity>(ENV1_R_PARAM, 0.f, 1.f, 0.f, "");
		configParam(ENV1_VEL_PARAM, 0.f, 1.f, 0.f, "");
		configParam(ENV1_ASSIGN_PARAM, 0.f, 1.f, 0.f, "");
		configParam<BipolarColorParamQuantity>(LFO1_FREQ_PARAM, 0.f, 1.f, 0.f, "");
		configParam(LFO1_SHAPE_PARAM, 0.f, 1.f, 0.f, "");
		configParam<BipolarColorParamQuantity>(LFO1_AMOUNT_PARAM, 0.f, 1.f, 0.f, "");
		configParam(LFO1_MODE_PARAM, 0.f, 1.f, 0.f, "");
		configParam(LFO1_UNIPOLAR_ASSIGN_PARAM, 0.f, 1.f, 0.f, "");
		configParam(LFO1_BIPOLAR_ASSIGN_PARAM, 0.f, 1.f, 0.f, "");
		configParam<BipolarColorParamQuantity>(GLOBAL_LFO_FREQ_PARAM, 0.f, 1.f, 0.f, "");
		configParam<BipolarColorParamQuantity>(GLOBAL_LFO_AMT_PARAM, 0.f, 1.f, 0.f, "");
		configParam(GLOBAL_LFO_ASSIGN_PARAM, 0.f, 1.f, 0.f, "");
		configParam<BipolarColorParamQuantity>(INDIVIDUAL_MOD_OUT_1_PARAM, 0.f, 1.f, 0.f, "");
		configParam(GATE_ASSIGN_PARAM, 0.f, 1.f, 0.f, "");
		configParam(VELOCITY_ASSIGN_PARAM, 0.f, 1.f, 0.f, "");
		configParam(DRIFT_1_ASSIGN_PARAM, 0.f, 1.f, 0.f, "");
		configParam<BipolarColorParamQuantity>(ENV2_A_PARAM, 0.f, 1.f, 0.f, "");
		configParam<BipolarColorParamQuantity>(ENV2_D_PARAM, 0.f, 1.f, 0.f, "");
		configParam<BipolarColorParamQuantity>(ENV2_S_PARAM, 0.f, 1.f, 0.f, "");
		configParam<BipolarColorParamQuantity>(ENV2_R_PARAM, 0.f, 1.f, 0.f, "");
		configParam(ENV2_VEL_PARAM, 0.f, 1.f, 0.f, "");
		configParam(ENV2_ASSIGN_PARAM, 0.f, 1.f, 0.f, "");
		configParam<BipolarColorParamQuantity>(LFO2_FREQ_PARAM, 0.f, 1.f, 0.f, "");
		configParam(LFO2_SHAPE_PARAM, 0.f, 1.f, 0.f, "");
		configParam<BipolarColorParamQuantity>(LFO2_AMOUNT_PARAM, 0.f, 1.f, 0.f, "");
		configParam(LFO2_MODE_PARAM, 0.f, 1.f, 0.f, "");
		configParam(LFO2_UNIPOLAR_ASSIGN_PARAM, 0.f, 1.f, 0.f, "");
		configParam(LFO2_BIPOLAR_ASSIGN_PARAM, 0.f, 1.f, 0.f, "");
		configParam(DRIFT_RATE_PARAM, 0.f, 1.f, 0.f, "");
		configParam(AFTERTOUCH_ASSIGN_PARAM, 0.f, 1.f, 0.f, "");
		configParam(DRIFT_BALANCE_PARAM, 0.f, 1.f, 0.f, "");
		configParam<BipolarColorParamQuantity>(INDIVIDUAL_MOD_OUT_2_PARAM, 0.f, 1.f, 0.f, "");
		configParam(DRIFT_2_ASSIGN_PARAM, 0.f, 1.f, 0.f, "");
		configParam(PITCH_WHEEL_ASSIGN_PARAM, 0.f, 1.f, 0.f, "");
		configParam(MOD_WHEEL_ASSIGN_PARAM, 0.f, 1.f, 0.f, "");
		configParam(OSC1_OCT_PARAM, 0.f, 1.f, 0.f, "");
		configParam<BipolarColorParamQuantity>(OSC1_SEMI_PARAM, 0.f, 1.f, 0.f, "");
		configParam<BipolarColorParamQuantity>(OSC1_FINE_PARAM, 0.f, 1.f, 0.f, "");
		configParam<BipolarColorParamQuantity>(OSC1_SHAPE_PARAM, 0.f, 1.f, 0.f, "");
		configParam<BipolarColorParamQuantity>(OSC1_PW_PARAM, 0.f, 1.f, 0.f, "");
		configParam<BipolarColorParamQuantity>(OSC1_VOL_PARAM, 0.f, 1.f, 0.f, "");
		configParam<BipolarColorParamQuantity>(OSC1_SUB_VOL_PARAM, 0.f, 1.f, 0.f, "");
		configParam(FILTER1_INPUT_ASSIGN_PARAM, 0.f, 1.f, 0.f, "");
		configParam<BipolarColorParamQuantity>(FILTER1_CUTOFF_PARAM, 0.f, 1.f, 0.f, "");
		configParam<BipolarColorParamQuantity>(FILTER1_RESONANCE_PARAM, 0.f, 1.f, 0.f, "");
		configParam(FILTER1_FM_ASSIGN_PARAM, 0.f, 1.f, 0.f, "");
		configParam<BipolarColorParamQuantity>(FILTER1_FM_PARAM, 0.f, 1.f, 0.f, "");
		configParam(FILTER1_TYPE_PARAM, 0.f, 1.f, 0.f, "");
		configParam<BipolarColorParamQuantity>(FILTER1_VOL_PARAM, 0.f, 1.f, 0.f, "");
		configParam<BipolarColorParamQuantity>(INDIVIDUAL_MOD_OUT_3_PARAM, 0.f, 1.f, 0.f, "");
		configParam(EXPRESSION_ASSIGN_PARAM, 0.f, 1.f, 0.f, "");
		configParam(INDIVIDUAL_MOD_1_ASSIGN_PARAM, 0.f, 1.f, 0.f, "");
		configParam(INDIVIDUAL_MOD_2_ASSIGN_PARAM, 0.f, 1.f, 0.f, "");
		configParam<BipolarColorParamQuantity>(OSC1_GLIDE_PARAM, 0.f, 1.f, 0.f, "");
		configParam<BipolarColorParamQuantity>(OSC2_GLIDE_PARAM, 0.f, 1.f, 0.f, "");
		configParam(OSC_SYNC_PARAM, 0.f, 1.f, 0.f, "");
		configParam<BipolarColorParamQuantity>(OSC_FM_PARAM, 0.f, 1.f, 0.f, "");
		configParam<BipolarColorParamQuantity>(OSC_RM_PARAM, 0.f, 1.f, 0.f, "");
		configParam<BipolarColorParamQuantity>(OSC_NOISE_PARAM, 0.f, 1.f, 0.f, "");
		configParam(FILTER2_INPUT_ASSIGN_PARAM, 0.f, 1.f, 0.f, "");
		configParam<BipolarColorParamQuantity>(FILTER2_CUTOFF_PARAM, 0.f, 1.f, 0.f, "");
		configParam<BipolarColorParamQuantity>(FILTER2_RESONANCE_PARAM, 0.f, 1.f, 0.f, "");
		configParam(FILTER2_FM_ASSIGN_PARAM, 0.f, 1.f, 0.f, "");
		configParam<BipolarColorParamQuantity>(FILTER2_FM_PARAM, 0.f, 1.f, 0.f, "");
		configParam<BipolarColorParamQuantity>(FILTER2_TYPE_PARAM, 0.f, 1.f, 0.f, "");
		configParam<BipolarColorParamQuantity>(FILTER2_VOL_PARAM, 0.f, 1.f, 0.f, "");
		configParam<BipolarColorParamQuantity>(INDIVIDUAL_MOD_OUT_4_PARAM, 0.f, 1.f, 0.f, "");
		configParam(VOICE_NR_ASSIGN_PARAM, 0.f, 1.f, 0.f, "");
		configParam(RANDOM_ASSIGN_PARAM, 0.f, 1.f, 0.f, "");
		configParam(OSC2_OCT_PARAM, 0.f, 1.f, 0.f, "");
		configParam<BipolarColorParamQuantity>(OSC2_SEMI_PARAM, 0.f, 1.f, 0.f, "");
		configParam<BipolarColorParamQuantity>(OSC2_FINE_PARAM, 0.f, 1.f, 0.f, "");
		configParam<BipolarColorParamQuantity>(OSC2_SHAPE_PARAM, 0.f, 1.f, 0.f, "");
		configParam<BipolarColorParamQuantity>(OSC2_PW_PARAM, 0.f, 1.f, 0.f, "");
		configParam<BipolarColorParamQuantity>(OSC2_VOL_PARAM, 0.f, 1.f, 0.f, "");
		configParam<BipolarColorParamQuantity>(OSC_EXT_VOL_PARAM, 0.f, 1.f, 0.f, "");
		configParam(AMP_INPUT_ASSIGN_PARAM, 0.f, 1.f, 0.f, "");
		configParam<BipolarColorParamQuantity>(AMP_VOL_PARAM, 0.f, 1.f, 0.f, "");
		configParam(AMP_AM_ASSIGN_PARAM, 0.f, 1.f, 0.f, "");
		configParam<BipolarColorParamQuantity>(AMP_AM_PARAM, 0.f, 1.f, 0.f, "");
		configParam(AMP_PAN_ASSIGN_PARAM, 0.f, 1.f, 0.f, "");
		configParam<BipolarColorParamQuantity>(AMP_PAN_PARAM, 0.f, 1.f, 0.f, "");
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
	}

	void process(const ProcessArgs& args) override {
	}
};


struct SynthWidget : ModuleWidget {
	SynthWidget(Synth* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/Synth.svg"), asset::plugin(pluginInstance, "res/Synth-dark.svg")));

//		addChild(createWidget<ThemedScrew>(Vec(0, 0)));
//		addChild(createWidget<ThemedScrew>(Vec(box.size.x - RACK_GRID_WIDTH, 0)));
//		addChild(createWidget<ThemedScrew>(Vec(0, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
//		addChild(createWidget<ThemedScrew>(Vec(box.size.x - RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		// mod in
		addParam(createParamCentered<VCVLightLatch<MediumSimpleLight<BlueLight>>>(mm2px(Vec(27.0, 10.874)), module, Synth::VOCT_ASSIGN_PARAM));
		addParam(createParamCentered<VCVLightLatch<MediumSimpleLight<BlueLight>>>(mm2px(Vec(27.0, 20.399)), module, Synth::GATE_ASSIGN_PARAM));
		addParam(createParamCentered<VCVLightLatch<MediumSimpleLight<BlueLight>>>(mm2px(Vec(27.0, 29.924)), module, Synth::VELOCITY_ASSIGN_PARAM));
		addParam(createParamCentered<VCVLightLatch<MediumSimpleLight<BlueLight>>>(mm2px(Vec(27.0, 39.449)), module, Synth::AFTERTOUCH_ASSIGN_PARAM));
		addParam(createParamCentered<VCVLightLatch<MediumSimpleLight<BlueLight>>>(mm2px(Vec(27.0, 48.974)), module, Synth::PITCH_WHEEL_ASSIGN_PARAM));
		addParam(createParamCentered<VCVLightLatch<MediumSimpleLight<BlueLight>>>(mm2px(Vec(27.0, 58.499)), module, Synth::MOD_WHEEL_ASSIGN_PARAM));
		addParam(createParamCentered<VCVLightLatch<MediumSimpleLight<BlueLight>>>(mm2px(Vec(27.0, 68.024)), module, Synth::EXPRESSION_ASSIGN_PARAM));
		addParam(createParamCentered<VCVLightLatch<MediumSimpleLight<BlueLight>>>(mm2px(Vec(27.0, 77.549)), module, Synth::INDIVIDUAL_MOD_1_ASSIGN_PARAM));
		addParam(createParamCentered<VCVLightLatch<MediumSimpleLight<BlueLight>>>(mm2px(Vec(27.0, 87.074)), module, Synth::INDIVIDUAL_MOD_2_ASSIGN_PARAM));
		addParam(createParamCentered<VCVLightLatch<MediumSimpleLight<BlueLight>>>(mm2px(Vec(27.0, 96.599)), module, Synth::VOICE_NR_ASSIGN_PARAM));
		addParam(createParamCentered<VCVLightLatch<MediumSimpleLight<BlueLight>>>(mm2px(Vec(27.0, 106.124)), module, Synth::RANDOM_ASSIGN_PARAM));

		// env
		addParam(createParamCentered<RoundBlackKnobWithArc>(mm2px(Vec(46.5, 12.088)), module, Synth::ENV1_A_PARAM));
		addParam(createParamCentered<RoundBlackKnobWithArc>(mm2px(Vec(63.5, 12.088)), module, Synth::ENV1_D_PARAM));
		addParam(createParamCentered<RoundBlackKnobWithArc>(mm2px(Vec(80.5, 12.088)), module, Synth::ENV1_S_PARAM));
		addParam(createParamCentered<RoundBlackKnobWithArc>(mm2px(Vec(97.5, 12.088)), module, Synth::ENV1_R_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(114.5, 12.088)), module, Synth::ENV1_VEL_PARAM));
		addParam(createParamCentered<VCVLightLatch<MediumSimpleLight<BlueLight>>>(mm2px(Vec(127.5, 12.088)), module, Synth::ENV1_ASSIGN_PARAM));

		addParam(createParamCentered<RoundBlackKnobWithArc>(mm2px(Vec(46.5, 37.188)), module, Synth::ENV2_A_PARAM));
		addParam(createParamCentered<RoundBlackKnobWithArc>(mm2px(Vec(63.5, 37.188)), module, Synth::ENV2_D_PARAM));
		addParam(createParamCentered<RoundBlackKnobWithArc>(mm2px(Vec(80.5, 37.188)), module, Synth::ENV2_S_PARAM));
		addParam(createParamCentered<RoundBlackKnobWithArc>(mm2px(Vec(97.5, 37.188)), module, Synth::ENV2_R_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(114.5, 37.188)), module, Synth::ENV2_VEL_PARAM));
		addParam(createParamCentered<VCVLightLatch<MediumSimpleLight<BlueLight>>>(mm2px(Vec(127.5, 37.188)), module, Synth::ENV2_ASSIGN_PARAM));

		// lfo
		addParam(createParamCentered<RoundBlackKnobWithArc>(mm2px(Vec(149.0, 12.088)), module, Synth::LFO1_FREQ_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(166.0, 12.088)), module, Synth::LFO1_SHAPE_PARAM));
		addParam(createParamCentered<RoundBlackKnobWithArc>(mm2px(Vec(183.0, 12.088)), module, Synth::LFO1_AMOUNT_PARAM));
		addParam(createParamCentered<NKK>(mm2px(Vec(198.0, 12.088)), module, Synth::LFO1_MODE_PARAM));
		addParam(createParamCentered<VCVLightLatch<MediumSimpleLight<BlueLight>>>(mm2px(Vec(210.0, 12.088)), module, Synth::LFO1_UNIPOLAR_ASSIGN_PARAM));
		addParam(createParamCentered<VCVLightLatch<MediumSimpleLight<BlueLight>>>(mm2px(Vec(222.0, 12.088)), module, Synth::LFO1_BIPOLAR_ASSIGN_PARAM));

		addParam(createParamCentered<RoundBlackKnobWithArc>(mm2px(Vec(149.0, 37.188)), module, Synth::LFO2_FREQ_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(166.0, 37.188)), module, Synth::LFO2_SHAPE_PARAM));
		addParam(createParamCentered<RoundBlackKnobWithArc>(mm2px(Vec(183.0, 37.188)), module, Synth::LFO2_AMOUNT_PARAM));
		addParam(createParamCentered<NKK>(mm2px(Vec(198.0, 37.188)), module, Synth::LFO2_MODE_PARAM));
		addParam(createParamCentered<VCVLightLatch<MediumSimpleLight<BlueLight>>>(mm2px(Vec(210.0, 37.188)), module, Synth::LFO2_UNIPOLAR_ASSIGN_PARAM));
		addParam(createParamCentered<VCVLightLatch<MediumSimpleLight<BlueLight>>>(mm2px(Vec(222.0, 37.188)), module, Synth::LFO2_BIPOLAR_ASSIGN_PARAM));

		addParam(createParamCentered<RoundBlackKnobWithArc>(mm2px(Vec(243.0, 12.088)), module, Synth::GLOBAL_LFO_FREQ_PARAM));
		addParam(createParamCentered<RoundBlackKnobWithArc>(mm2px(Vec(260.0, 12.088)), module, Synth::GLOBAL_LFO_AMT_PARAM));
		addParam(createParamCentered<VCVLightLatch<MediumSimpleLight<BlueLight>>>(mm2px(Vec(275.0, 12.088)), module, Synth::GLOBAL_LFO_ASSIGN_PARAM));

		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(243.0, 37.188)), module, Synth::DRIFT_RATE_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(260.0, 37.188)), module, Synth::DRIFT_BALANCE_PARAM));
		addParam(createParamCentered<VCVLightLatch<MediumSimpleLight<BlueLight>>>(mm2px(Vec(275.0, 33.843)), module, Synth::DRIFT_1_ASSIGN_PARAM));
		addParam(createParamCentered<VCVLightLatch<MediumSimpleLight<BlueLight>>>(mm2px(Vec(275.0, 44.742)), module, Synth::DRIFT_2_ASSIGN_PARAM));

		// mod out
		addParam(createParamCentered<RoundBlackKnobWithArc>(mm2px(Vec(293.0, 15.088)), module, Synth::INDIVIDUAL_MOD_OUT_1_PARAM));
		addParam(createParamCentered<RoundBlackKnobWithArc>(mm2px(Vec(293.0, 39.188)), module, Synth::INDIVIDUAL_MOD_OUT_2_PARAM));
		addParam(createParamCentered<RoundBlackKnobWithArc>(mm2px(Vec(293.0, 63.288)), module, Synth::INDIVIDUAL_MOD_OUT_3_PARAM));
		addParam(createParamCentered<RoundBlackKnobWithArc>(mm2px(Vec(293.0, 87.388)), module, Synth::INDIVIDUAL_MOD_OUT_4_PARAM));

		// osc
		addParam(createParamCentered<RoundBlackKnobWithArc>(mm2px(Vec(55.0, 87.388)), module, Synth::OSC1_GLIDE_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(46.5, 62.288)), module, Synth::OSC1_OCT_PARAM));
		addParam(createParamCentered<RoundBlackKnobWithArc>(mm2px(Vec(63.5, 62.288)), module, Synth::OSC1_SEMI_PARAM));
		addParam(createParamCentered<RoundBlackKnobWithArc>(mm2px(Vec(80.5, 62.288)), module, Synth::OSC1_FINE_PARAM));
		addParam(createParamCentered<RoundBlackKnobWithArc>(mm2px(Vec(100.5, 62.288)), module, Synth::OSC1_SHAPE_PARAM));
		addParam(createParamCentered<RoundBlackKnobWithArc>(mm2px(Vec(117.5, 62.288)), module, Synth::OSC1_PW_PARAM));
		addParam(createParamCentered<RoundBlackKnobWithArc>(mm2px(Vec(137.5, 62.288)), module, Synth::OSC1_VOL_PARAM));
		addParam(createParamCentered<RoundBlackKnobWithArc>(mm2px(Vec(154.5, 62.288)), module, Synth::OSC1_SUB_VOL_PARAM));

		addParam(createParamCentered<RoundBlackKnobWithArc>(mm2px(Vec(72.0, 87.388)), module, Synth::OSC2_GLIDE_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(46.5, 112.488)), module, Synth::OSC2_OCT_PARAM));
		addParam(createParamCentered<RoundBlackKnobWithArc>(mm2px(Vec(63.5, 112.488)), module, Synth::OSC2_SEMI_PARAM));
		addParam(createParamCentered<RoundBlackKnobWithArc>(mm2px(Vec(80.5, 112.488)), module, Synth::OSC2_FINE_PARAM));
		addParam(createParamCentered<RoundBlackKnobWithArc>(mm2px(Vec(100.5, 112.488)), module, Synth::OSC2_SHAPE_PARAM));
		addParam(createParamCentered<RoundBlackKnobWithArc>(mm2px(Vec(117.5, 112.488)), module, Synth::OSC2_PW_PARAM));
		addParam(createParamCentered<RoundBlackKnobWithArc>(mm2px(Vec(137.5, 112.488)), module, Synth::OSC2_VOL_PARAM));

		addParam(createParamCentered<VCVLightLatch<MediumSimpleLight<WhiteLight>>>(mm2px(Vec(100.5, 87.388)), module, Synth::OSC_SYNC_PARAM));
		addParam(createParamCentered<RoundBlackKnobWithArc>(mm2px(Vec(117.5, 87.388)), module, Synth::OSC_FM_PARAM));
		addParam(createParamCentered<RoundBlackKnobWithArc>(mm2px(Vec(137.5, 87.388)), module, Synth::OSC_RM_PARAM));

		addParam(createParamCentered<RoundBlackKnobWithArc>(mm2px(Vec(154.5, 87.388)), module, Synth::OSC_NOISE_PARAM));
		addParam(createParamCentered<RoundBlackKnobWithArc>(mm2px(Vec(154.5, 112.488)), module, Synth::OSC_EXT_VOL_PARAM));

		// filter
		addParam(createParamCentered<VCVLightLatch<MediumSimpleLight<RedLight>>>(mm2px(Vec(176.5, 62.288)), module, Synth::FILTER1_INPUT_ASSIGN_PARAM));
		addParam(createParamCentered<RoundBlackKnobWithArc>(mm2px(Vec(191.5, 62.288)), module, Synth::FILTER1_CUTOFF_PARAM));
		addParam(createParamCentered<RoundBlackKnobWithArc>(mm2px(Vec(208.5, 62.288)), module, Synth::FILTER1_RESONANCE_PARAM));
		addParam(createParamCentered<VCVLightLatch<MediumSimpleLight<RedLight>>>(mm2px(Vec(223.0, 62.288)), module, Synth::FILTER1_FM_ASSIGN_PARAM));
		addParam(createParamCentered<RoundBlackKnobWithArc>(mm2px(Vec(238.0, 62.288)), module, Synth::FILTER1_FM_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(255.0, 62.288)), module, Synth::FILTER1_TYPE_PARAM));
		addParam(createParamCentered<RoundBlackKnobWithArc>(mm2px(Vec(272.0, 62.288)), module, Synth::FILTER1_VOL_PARAM));

		addParam(createParamCentered<VCVLightLatch<MediumSimpleLight<RedLight>>>(mm2px(Vec(176.5, 87.388)), module, Synth::FILTER2_INPUT_ASSIGN_PARAM));
		addParam(createParamCentered<RoundBlackKnobWithArc>(mm2px(Vec(191.5, 87.388)), module, Synth::FILTER2_CUTOFF_PARAM));
		addParam(createParamCentered<RoundBlackKnobWithArc>(mm2px(Vec(208.5, 87.388)), module, Synth::FILTER2_RESONANCE_PARAM));
		addParam(createParamCentered<VCVLightLatch<MediumSimpleLight<RedLight>>>(mm2px(Vec(223.0, 87.388)), module, Synth::FILTER2_FM_ASSIGN_PARAM));
		addParam(createParamCentered<RoundBlackKnobWithArc>(mm2px(Vec(238.0, 87.388)), module, Synth::FILTER2_FM_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(255.0, 87.388)), module, Synth::FILTER2_TYPE_PARAM));
		addParam(createParamCentered<RoundBlackKnobWithArc>(mm2px(Vec(272.0, 87.388)), module, Synth::FILTER2_VOL_PARAM));

		// amp
		addParam(createParamCentered<VCVLightLatch<MediumSimpleLight<RedLight>>>(mm2px(Vec(176.464, 112.488)), module, Synth::AMP_INPUT_ASSIGN_PARAM));
		addParam(createParamCentered<RoundBlackKnobWithArc>(mm2px(Vec(191.5, 112.488)), module, Synth::AMP_VOL_PARAM));
		addParam(createParamCentered<VCVLightLatch<MediumSimpleLight<RedLight>>>(mm2px(Vec(216.745, 112.488)), module, Synth::AMP_AM_ASSIGN_PARAM));
		addParam(createParamCentered<RoundBlackKnobWithArc>(mm2px(Vec(231.745, 112.488)), module, Synth::AMP_AM_PARAM));
		addParam(createParamCentered<VCVLightLatch<MediumSimpleLight<RedLight>>>(mm2px(Vec(257.0, 112.488)), module, Synth::AMP_PAN_ASSIGN_PARAM));
		addParam(createParamCentered<RoundBlackKnobWithArc>(mm2px(Vec(272.0, 112.488)), module, Synth::AMP_PAN_PARAM));


		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(7.0, 10.874)), module, Synth::VOCT_INPUT));
		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(7.0, 20.399)), module, Synth::GATE_INPUT));
		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(7.0, 29.924)), module, Synth::VELOCITY_INPUT));
		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(7.0, 39.449)), module, Synth::AFTERTOUCH_INPUT));
		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(7.0, 48.974)), module, Synth::PITCH_WHEEL_INPUT));
		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(7.0, 58.499)), module, Synth::MOD_WHEEL_INPUT));
		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(7.0, 68.024)), module, Synth::EXPRESSION_INPUT));
		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(7.0, 77.549)), module, Synth::INDIVIDUAL_MOD_1_INPUT));
		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(7.0, 87.074)), module, Synth::INDIVIDUAL_MOD_2_INPUT));
		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(8.983, 116.799)), module, Synth::RETRIGGER_INPUT));
		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(24.823, 116.799)), module, Synth::EXT_INPUT));

		addOutput(createOutputCentered<ThemedPJ301MPort>(mm2px(Vec(308.0, 15.088)), module, Synth::INDIVIDUAL_MOD_1_OUTPUT));
		addOutput(createOutputCentered<ThemedPJ301MPort>(mm2px(Vec(308.0, 39.188)), module, Synth::INDIVIDUAL_MOD_2_OUTPUT));
		addOutput(createOutputCentered<ThemedPJ301MPort>(mm2px(Vec(308.0, 63.288)), module, Synth::INDIVIDUAL_MOD_3_OUTPUT));
		addOutput(createOutputCentered<ThemedPJ301MPort>(mm2px(Vec(308.0, 87.388)), module, Synth::INDIVIDUAL_MOD_4_OUTPUT));
		addOutput(createOutputCentered<ThemedPJ301MPort>(mm2px(Vec(290.525, 112.488)), module, Synth::OUT_L_OUTPUT));
		addOutput(createOutputCentered<ThemedPJ301MPort>(mm2px(Vec(305.679, 112.488)), module, Synth::OUT_R_OUTPUT));
	}
};


Model* modelSynth = createModel<Synth, SynthWidget>("Synth");

}
