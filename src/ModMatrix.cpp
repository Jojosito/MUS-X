#include "plugin.hpp"

namespace musx {

struct ModMatrix : Module {
	enum ParamId {
		CTRL1_PARAM,
		CTRL2_PARAM,
		CTRL3_PARAM,
		CTRL4_PARAM,
		CTRL5_PARAM,
		CTRL6_PARAM,
		CTRL7_PARAM,
		CTRL8_PARAM,
		CTRL9_PARAM,
		CTRL10_PARAM,
		CTRL11_PARAM,
		CTRL12_PARAM,
		CTRL13_PARAM,
		CTRL14_PARAM,
		CTRL15_PARAM,
		CTRL16_PARAM,
		_1_1_PARAM,
		_1_2_PARAM,
		_1_3_PARAM,
		_1_4_PARAM,
		_1_5_PARAM,
		_1_6_PARAM,
		_1_7_PARAM,
		_1_8_PARAM,
		_1_9_PARAM,
		_1_10_PARAM,
		_1_11_PARAM,
		_1_12_PARAM,
		_1_13_PARAM,
		_1_14_PARAM,
		_1_15_PARAM,
		_1_16_PARAM,
		SEL1_PARAM,
		_2_1_PARAM,
		_2_2_PARAM,
		_2_3_PARAM,
		_2_4_PARAM,
		_2_5_PARAM,
		_2_6_PARAM,
		_2_7_PARAM,
		_2_8_PARAM,
		_2_9_PARAM,
		_2_10_PARAM,
		_2_11_PARAM,
		_2_12_PARAM,
		_2_13_PARAM,
		_2_14_PARAM,
		_2_15_PARAM,
		_2_16_PARAM,
		SEL2_PARAM,
		_3_1_PARAM,
		_3_2_PARAM,
		_3_3_PARAM,
		_3_4_PARAM,
		_3_5_PARAM,
		_3_6_PARAM,
		_3_7_PARAM,
		_3_8_PARAM,
		_3_9_PARAM,
		_3_10_PARAM,
		_3_11_PARAM,
		_3_12_PARAM,
		_3_13_PARAM,
		_3_14_PARAM,
		_3_15_PARAM,
		_3_16_PARAM,
		SEL3_PARAM,
		_4_1_PARAM,
		_4_2_PARAM,
		_4_3_PARAM,
		_4_4_PARAM,
		_4_5_PARAM,
		_4_6_PARAM,
		_4_7_PARAM,
		_4_8_PARAM,
		_4_9_PARAM,
		_4_10_PARAM,
		_4_11_PARAM,
		_4_12_PARAM,
		_4_13_PARAM,
		_4_14_PARAM,
		_4_15_PARAM,
		_4_16_PARAM,
		SEL4_PARAM,
		_5_1_PARAM,
		_5_2_PARAM,
		_5_3_PARAM,
		_5_4_PARAM,
		_5_5_PARAM,
		_5_6_PARAM,
		_5_7_PARAM,
		_5_8_PARAM,
		_5_9_PARAM,
		_5_10_PARAM,
		_5_11_PARAM,
		_5_12_PARAM,
		_5_13_PARAM,
		_5_14_PARAM,
		_5_15_PARAM,
		_5_16_PARAM,
		SEL5_PARAM,
		_6_1_PARAM,
		_6_2_PARAM,
		_6_3_PARAM,
		_6_4_PARAM,
		_6_5_PARAM,
		_6_6_PARAM,
		_6_7_PARAM,
		_6_8_PARAM,
		_6_9_PARAM,
		_6_10_PARAM,
		_6_11_PARAM,
		_6_12_PARAM,
		_6_13_PARAM,
		_6_14_PARAM,
		_6_15_PARAM,
		_6_16_PARAM,
		SEL6_PARAM,
		_7_1_PARAM,
		_7_2_PARAM,
		_7_3_PARAM,
		_7_4_PARAM,
		_7_5_PARAM,
		_7_6_PARAM,
		_7_7_PARAM,
		_7_8_PARAM,
		_7_9_PARAM,
		_7_10_PARAM,
		_7_11_PARAM,
		_7_12_PARAM,
		_7_13_PARAM,
		_7_14_PARAM,
		_7_15_PARAM,
		_7_16_PARAM,
		SEL7_PARAM,
		_8_1_PARAM,
		_8_2_PARAM,
		_8_3_PARAM,
		_8_4_PARAM,
		_8_5_PARAM,
		_8_6_PARAM,
		_8_7_PARAM,
		_8_8_PARAM,
		_8_9_PARAM,
		_8_10_PARAM,
		_8_11_PARAM,
		_8_12_PARAM,
		_8_13_PARAM,
		_8_14_PARAM,
		_8_15_PARAM,
		_8_16_PARAM,
		SEL8_PARAM,
		_9_1_PARAM,
		_9_2_PARAM,
		_9_3_PARAM,
		_9_4_PARAM,
		_9_5_PARAM,
		_9_6_PARAM,
		_9_7_PARAM,
		_9_8_PARAM,
		_9_9_PARAM,
		_9_10_PARAM,
		_9_11_PARAM,
		_9_12_PARAM,
		_9_13_PARAM,
		_9_14_PARAM,
		_9_15_PARAM,
		_9_16_PARAM,
		SEL9_PARAM,
		_10_1_PARAM,
		_10_2_PARAM,
		_10_3_PARAM,
		_10_4_PARAM,
		_10_5_PARAM,
		_10_6_PARAM,
		_10_7_PARAM,
		_10_8_PARAM,
		_10_9_PARAM,
		_10_10_PARAM,
		_10_11_PARAM,
		_10_12_PARAM,
		_10_13_PARAM,
		_10_14_PARAM,
		_10_15_PARAM,
		_10_16_PARAM,
		SEL10_PARAM,
		_11_1_PARAM,
		_11_2_PARAM,
		_11_3_PARAM,
		_11_4_PARAM,
		_11_5_PARAM,
		_11_6_PARAM,
		_11_7_PARAM,
		_11_8_PARAM,
		_11_9_PARAM,
		_11_10_PARAM,
		_11_11_PARAM,
		_11_12_PARAM,
		_11_13_PARAM,
		_11_14_PARAM,
		_11_15_PARAM,
		_11_16_PARAM,
		SEL11_PARAM,
		_12_1_PARAM,
		_12_2_PARAM,
		_12_3_PARAM,
		_12_4_PARAM,
		_12_5_PARAM,
		_12_6_PARAM,
		_12_7_PARAM,
		_12_8_PARAM,
		_12_9_PARAM,
		_12_10_PARAM,
		_12_11_PARAM,
		_12_12_PARAM,
		_12_13_PARAM,
		_12_14_PARAM,
		_12_15_PARAM,
		_12_16_PARAM,
		SEL12_PARAM,
		PARAMS_LEN
	};
	enum InputId {
		_0_INPUT,
		_1_INPUT,
		_2_INPUT,
		_3_INPUT,
		_4_INPUT,
		_5_INPUT,
		_6_INPUT,
		_7_INPUT,
		_8_INPUT,
		_9_INPUT,
		_10_INPUT,
		_11_INPUT,
		_12_INPUT,
		INPUTS_LEN
	};
	enum OutputId {
		_1_OUTPUT,
		_2_OUTPUT,
		_3_OUTPUT,
		_4_OUTPUT,
		_5_OUTPUT,
		_6_OUTPUT,
		_7_OUTPUT,
		_8_OUTPUT,
		_9_OUTPUT,
		_10_OUTPUT,
		_11_OUTPUT,
		_12_OUTPUT,
		_13_OUTPUT,
		_14_OUTPUT,
		_15_OUTPUT,
		_16_OUTPUT,
		OUTPUTS_LEN
	};
	enum LightId {
		LIGHTS_LEN
	};

	ModMatrix() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configParam(CTRL1_PARAM, 0.f, 1.f, 0.f, "");
		configParam(CTRL2_PARAM, 0.f, 1.f, 0.f, "");
		configParam(CTRL3_PARAM, 0.f, 1.f, 0.f, "");
		configParam(CTRL4_PARAM, 0.f, 1.f, 0.f, "");
		configParam(CTRL5_PARAM, 0.f, 1.f, 0.f, "");
		configParam(CTRL6_PARAM, 0.f, 1.f, 0.f, "");
		configParam(CTRL7_PARAM, 0.f, 1.f, 0.f, "");
		configParam(CTRL8_PARAM, 0.f, 1.f, 0.f, "");
		configParam(CTRL9_PARAM, 0.f, 1.f, 0.f, "");
		configParam(CTRL10_PARAM, 0.f, 1.f, 0.f, "");
		configParam(CTRL11_PARAM, 0.f, 1.f, 0.f, "");
		configParam(CTRL12_PARAM, 0.f, 1.f, 0.f, "");
		configParam(CTRL13_PARAM, 0.f, 1.f, 0.f, "");
		configParam(CTRL14_PARAM, 0.f, 1.f, 0.f, "");
		configParam(CTRL15_PARAM, 0.f, 1.f, 0.f, "");
		configParam(CTRL16_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_1_1_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_1_2_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_1_3_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_1_4_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_1_5_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_1_6_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_1_7_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_1_8_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_1_9_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_1_10_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_1_11_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_1_12_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_1_13_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_1_14_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_1_15_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_1_16_PARAM, 0.f, 1.f, 0.f, "");
		configParam(SEL1_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_2_1_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_2_2_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_2_3_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_2_4_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_2_5_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_2_6_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_2_7_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_2_8_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_2_9_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_2_10_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_2_11_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_2_12_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_2_13_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_2_14_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_2_15_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_2_16_PARAM, 0.f, 1.f, 0.f, "");
		configParam(SEL2_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_3_1_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_3_2_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_3_3_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_3_4_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_3_5_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_3_6_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_3_7_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_3_8_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_3_9_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_3_10_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_3_11_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_3_12_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_3_13_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_3_14_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_3_15_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_3_16_PARAM, 0.f, 1.f, 0.f, "");
		configParam(SEL3_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_4_1_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_4_2_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_4_3_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_4_4_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_4_5_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_4_6_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_4_7_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_4_8_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_4_9_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_4_10_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_4_11_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_4_12_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_4_13_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_4_14_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_4_15_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_4_16_PARAM, 0.f, 1.f, 0.f, "");
		configParam(SEL4_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_5_1_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_5_2_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_5_3_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_5_4_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_5_5_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_5_6_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_5_7_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_5_8_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_5_9_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_5_10_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_5_11_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_5_12_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_5_13_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_5_14_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_5_15_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_5_16_PARAM, 0.f, 1.f, 0.f, "");
		configParam(SEL5_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_6_1_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_6_2_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_6_3_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_6_4_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_6_5_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_6_6_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_6_7_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_6_8_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_6_9_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_6_10_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_6_11_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_6_12_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_6_13_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_6_14_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_6_15_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_6_16_PARAM, 0.f, 1.f, 0.f, "");
		configParam(SEL6_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_7_1_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_7_2_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_7_3_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_7_4_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_7_5_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_7_6_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_7_7_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_7_8_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_7_9_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_7_10_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_7_11_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_7_12_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_7_13_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_7_14_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_7_15_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_7_16_PARAM, 0.f, 1.f, 0.f, "");
		configParam(SEL7_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_8_1_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_8_2_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_8_3_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_8_4_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_8_5_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_8_6_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_8_7_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_8_8_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_8_9_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_8_10_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_8_11_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_8_12_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_8_13_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_8_14_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_8_15_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_8_16_PARAM, 0.f, 1.f, 0.f, "");
		configParam(SEL8_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_9_1_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_9_2_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_9_3_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_9_4_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_9_5_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_9_6_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_9_7_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_9_8_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_9_9_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_9_10_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_9_11_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_9_12_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_9_13_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_9_14_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_9_15_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_9_16_PARAM, 0.f, 1.f, 0.f, "");
		configParam(SEL9_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_10_1_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_10_2_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_10_3_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_10_4_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_10_5_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_10_6_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_10_7_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_10_8_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_10_9_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_10_10_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_10_11_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_10_12_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_10_13_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_10_14_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_10_15_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_10_16_PARAM, 0.f, 1.f, 0.f, "");
		configParam(SEL10_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_11_1_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_11_2_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_11_3_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_11_4_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_11_5_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_11_6_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_11_7_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_11_8_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_11_9_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_11_10_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_11_11_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_11_12_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_11_13_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_11_14_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_11_15_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_11_16_PARAM, 0.f, 1.f, 0.f, "");
		configParam(SEL11_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_12_1_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_12_2_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_12_3_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_12_4_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_12_5_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_12_6_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_12_7_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_12_8_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_12_9_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_12_10_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_12_11_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_12_12_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_12_13_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_12_14_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_12_15_PARAM, 0.f, 1.f, 0.f, "");
		configParam(_12_16_PARAM, 0.f, 1.f, 0.f, "");
		configParam(SEL12_PARAM, 0.f, 1.f, 0.f, "");
		configInput(_0_INPUT, "");
		configInput(_1_INPUT, "");
		configInput(_2_INPUT, "");
		configInput(_3_INPUT, "");
		configInput(_4_INPUT, "");
		configInput(_5_INPUT, "");
		configInput(_6_INPUT, "");
		configInput(_7_INPUT, "");
		configInput(_8_INPUT, "");
		configInput(_9_INPUT, "");
		configInput(_10_INPUT, "");
		configInput(_11_INPUT, "");
		configInput(_12_INPUT, "");
		configOutput(_1_OUTPUT, "");
		configOutput(_2_OUTPUT, "");
		configOutput(_3_OUTPUT, "");
		configOutput(_4_OUTPUT, "");
		configOutput(_5_OUTPUT, "");
		configOutput(_6_OUTPUT, "");
		configOutput(_7_OUTPUT, "");
		configOutput(_8_OUTPUT, "");
		configOutput(_9_OUTPUT, "");
		configOutput(_10_OUTPUT, "");
		configOutput(_11_OUTPUT, "");
		configOutput(_12_OUTPUT, "");
		configOutput(_13_OUTPUT, "");
		configOutput(_14_OUTPUT, "");
		configOutput(_15_OUTPUT, "");
		configOutput(_16_OUTPUT, "");
	}

	void process(const ProcessArgs& args) override {
	}
};


struct ModMatrixWidget : ModuleWidget {
	ModMatrixWidget(ModMatrix* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/ModMatrix.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(22.86, 8.031)), module, ModMatrix::CTRL1_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(30.48, 8.031)), module, ModMatrix::CTRL2_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(38.1, 8.031)), module, ModMatrix::CTRL3_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(45.72, 8.031)), module, ModMatrix::CTRL4_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(53.34, 8.031)), module, ModMatrix::CTRL5_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(60.96, 8.031)), module, ModMatrix::CTRL6_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(68.58, 8.031)), module, ModMatrix::CTRL7_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(76.2, 8.031)), module, ModMatrix::CTRL8_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(83.82, 8.031)), module, ModMatrix::CTRL9_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(91.44, 8.031)), module, ModMatrix::CTRL10_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(99.06, 8.031)), module, ModMatrix::CTRL11_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(106.68, 8.031)), module, ModMatrix::CTRL12_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(114.3, 8.031)), module, ModMatrix::CTRL13_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(121.92, 8.031)), module, ModMatrix::CTRL14_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(129.54, 8.031)), module, ModMatrix::CTRL15_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(137.16, 8.031)), module, ModMatrix::CTRL16_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(22.86, 20.078)), module, ModMatrix::_1_1_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(30.48, 20.078)), module, ModMatrix::_1_2_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(38.1, 20.078)), module, ModMatrix::_1_3_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(45.72, 20.078)), module, ModMatrix::_1_4_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(53.34, 20.078)), module, ModMatrix::_1_5_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(60.96, 20.078)), module, ModMatrix::_1_6_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(68.58, 20.078)), module, ModMatrix::_1_7_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(76.2, 20.078)), module, ModMatrix::_1_8_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(83.82, 20.078)), module, ModMatrix::_1_9_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(91.44, 20.078)), module, ModMatrix::_1_10_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(99.06, 20.078)), module, ModMatrix::_1_11_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(106.68, 20.078)), module, ModMatrix::_1_12_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(114.3, 20.077)), module, ModMatrix::_1_13_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(121.92, 20.078)), module, ModMatrix::_1_14_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(129.54, 20.078)), module, ModMatrix::_1_15_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(137.16, 20.078)), module, ModMatrix::_1_16_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(144.78, 20.077)), module, ModMatrix::SEL1_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(22.86, 28.109)), module, ModMatrix::_2_1_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(30.48, 28.11)), module, ModMatrix::_2_2_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(38.1, 28.109)), module, ModMatrix::_2_3_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(45.72, 28.11)), module, ModMatrix::_2_4_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(53.34, 28.11)), module, ModMatrix::_2_5_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(60.96, 28.11)), module, ModMatrix::_2_6_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(68.58, 28.11)), module, ModMatrix::_2_7_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(76.2, 28.11)), module, ModMatrix::_2_8_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(83.82, 28.11)), module, ModMatrix::_2_9_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(91.44, 28.109)), module, ModMatrix::_2_10_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(99.06, 28.11)), module, ModMatrix::_2_11_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(106.68, 28.11)), module, ModMatrix::_2_12_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(114.3, 28.109)), module, ModMatrix::_2_13_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(121.92, 28.11)), module, ModMatrix::_2_14_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(129.54, 28.11)), module, ModMatrix::_2_15_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(137.16, 28.11)), module, ModMatrix::_2_16_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(144.78, 28.109)), module, ModMatrix::SEL2_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(22.86, 36.141)), module, ModMatrix::_3_1_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(30.48, 36.141)), module, ModMatrix::_3_2_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(38.1, 36.14)), module, ModMatrix::_3_3_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(45.72, 36.141)), module, ModMatrix::_3_4_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(53.34, 36.141)), module, ModMatrix::_3_5_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(60.96, 36.141)), module, ModMatrix::_3_6_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(68.58, 36.141)), module, ModMatrix::_3_7_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(76.2, 36.141)), module, ModMatrix::_3_8_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(83.82, 36.141)), module, ModMatrix::_3_9_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(91.44, 36.141)), module, ModMatrix::_3_10_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(99.06, 36.141)), module, ModMatrix::_3_11_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(106.68, 36.141)), module, ModMatrix::_3_12_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(114.3, 36.14)), module, ModMatrix::_3_13_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(121.92, 36.141)), module, ModMatrix::_3_14_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(129.54, 36.141)), module, ModMatrix::_3_15_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(137.16, 36.141)), module, ModMatrix::_3_16_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(144.78, 36.14)), module, ModMatrix::SEL3_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(22.86, 44.172)), module, ModMatrix::_4_1_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(30.48, 44.173)), module, ModMatrix::_4_2_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(38.1, 44.172)), module, ModMatrix::_4_3_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(45.72, 44.173)), module, ModMatrix::_4_4_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(53.34, 44.173)), module, ModMatrix::_4_5_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(60.96, 44.173)), module, ModMatrix::_4_6_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(68.58, 44.173)), module, ModMatrix::_4_7_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(76.2, 44.173)), module, ModMatrix::_4_8_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(83.82, 44.173)), module, ModMatrix::_4_9_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(91.44, 44.173)), module, ModMatrix::_4_10_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(99.06, 44.173)), module, ModMatrix::_4_11_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(106.68, 44.173)), module, ModMatrix::_4_12_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(114.3, 44.172)), module, ModMatrix::_4_13_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(121.92, 44.173)), module, ModMatrix::_4_14_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(129.54, 44.173)), module, ModMatrix::_4_15_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(137.16, 44.173)), module, ModMatrix::_4_16_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(144.78, 44.172)), module, ModMatrix::SEL4_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(22.86, 52.203)), module, ModMatrix::_5_1_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(30.48, 52.203)), module, ModMatrix::_5_2_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(38.1, 52.203)), module, ModMatrix::_5_3_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(45.72, 52.204)), module, ModMatrix::_5_4_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(53.34, 52.204)), module, ModMatrix::_5_5_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(60.96, 52.204)), module, ModMatrix::_5_6_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(68.58, 52.204)), module, ModMatrix::_5_7_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(76.2, 52.204)), module, ModMatrix::_5_8_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(83.82, 52.204)), module, ModMatrix::_5_9_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(91.44, 52.204)), module, ModMatrix::_5_10_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(99.06, 52.204)), module, ModMatrix::_5_11_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(106.68, 52.204)), module, ModMatrix::_5_12_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(114.3, 52.203)), module, ModMatrix::_5_13_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(121.92, 52.204)), module, ModMatrix::_5_14_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(129.54, 52.204)), module, ModMatrix::_5_15_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(137.16, 52.204)), module, ModMatrix::_5_16_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(144.78, 52.202)), module, ModMatrix::SEL5_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(22.86, 60.236)), module, ModMatrix::_6_1_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(30.48, 60.236)), module, ModMatrix::_6_2_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(38.1, 60.235)), module, ModMatrix::_6_3_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(45.72, 60.236)), module, ModMatrix::_6_4_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(53.34, 60.236)), module, ModMatrix::_6_5_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(60.96, 60.236)), module, ModMatrix::_6_6_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(68.58, 60.236)), module, ModMatrix::_6_7_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(76.2, 60.236)), module, ModMatrix::_6_8_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(83.82, 60.236)), module, ModMatrix::_6_9_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(91.44, 60.236)), module, ModMatrix::_6_10_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(99.06, 60.236)), module, ModMatrix::_6_11_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(106.68, 60.236)), module, ModMatrix::_6_12_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(114.3, 60.235)), module, ModMatrix::_6_13_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(121.92, 60.236)), module, ModMatrix::_6_14_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(129.54, 60.236)), module, ModMatrix::_6_15_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(137.16, 60.236)), module, ModMatrix::_6_16_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(144.78, 60.234)), module, ModMatrix::SEL6_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(22.86, 68.266)), module, ModMatrix::_7_1_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(30.48, 68.266)), module, ModMatrix::_7_2_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(38.1, 68.265)), module, ModMatrix::_7_3_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(45.72, 68.266)), module, ModMatrix::_7_4_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(53.34, 68.266)), module, ModMatrix::_7_5_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(60.96, 68.266)), module, ModMatrix::_7_6_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(68.58, 68.266)), module, ModMatrix::_7_7_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(76.2, 68.266)), module, ModMatrix::_7_8_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(83.82, 68.266)), module, ModMatrix::_7_9_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(91.44, 68.266)), module, ModMatrix::_7_10_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(99.06, 68.266)), module, ModMatrix::_7_11_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(106.68, 68.266)), module, ModMatrix::_7_12_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(114.3, 68.265)), module, ModMatrix::_7_13_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(121.92, 68.266)), module, ModMatrix::_7_14_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(129.54, 68.266)), module, ModMatrix::_7_15_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(137.16, 68.266)), module, ModMatrix::_7_16_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(144.78, 68.265)), module, ModMatrix::SEL7_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(22.86, 76.298)), module, ModMatrix::_8_1_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(30.48, 76.298)), module, ModMatrix::_8_2_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(38.1, 76.297)), module, ModMatrix::_8_3_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(45.72, 76.298)), module, ModMatrix::_8_4_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(53.34, 76.298)), module, ModMatrix::_8_5_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(60.96, 76.298)), module, ModMatrix::_8_6_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(68.58, 76.298)), module, ModMatrix::_8_7_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(76.2, 76.298)), module, ModMatrix::_8_8_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(83.82, 76.298)), module, ModMatrix::_8_9_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(91.44, 76.298)), module, ModMatrix::_8_10_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(99.06, 76.298)), module, ModMatrix::_8_11_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(106.68, 76.298)), module, ModMatrix::_8_12_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(114.3, 76.297)), module, ModMatrix::_8_13_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(121.92, 76.298)), module, ModMatrix::_8_14_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(129.54, 76.298)), module, ModMatrix::_8_15_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(137.16, 76.298)), module, ModMatrix::_8_16_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(144.78, 76.297)), module, ModMatrix::SEL8_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(22.86, 84.329)), module, ModMatrix::_9_1_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(30.48, 84.329)), module, ModMatrix::_9_2_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(38.1, 84.328)), module, ModMatrix::_9_3_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(45.72, 84.329)), module, ModMatrix::_9_4_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(53.34, 84.329)), module, ModMatrix::_9_5_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(60.96, 84.329)), module, ModMatrix::_9_6_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(68.58, 84.329)), module, ModMatrix::_9_7_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(76.2, 84.329)), module, ModMatrix::_9_8_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(83.82, 84.329)), module, ModMatrix::_9_9_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(91.44, 84.329)), module, ModMatrix::_9_10_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(99.06, 84.329)), module, ModMatrix::_9_11_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(106.68, 84.329)), module, ModMatrix::_9_12_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(114.3, 84.328)), module, ModMatrix::_9_13_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(121.92, 84.329)), module, ModMatrix::_9_14_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(129.54, 84.329)), module, ModMatrix::_9_15_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(137.16, 84.329)), module, ModMatrix::_9_16_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(144.78, 84.327)), module, ModMatrix::SEL9_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(22.86, 92.359)), module, ModMatrix::_10_1_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(30.48, 92.361)), module, ModMatrix::_10_2_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(38.1, 92.36)), module, ModMatrix::_10_3_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(45.72, 92.361)), module, ModMatrix::_10_4_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(53.34, 92.361)), module, ModMatrix::_10_5_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(60.96, 92.361)), module, ModMatrix::_10_6_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(68.58, 92.361)), module, ModMatrix::_10_7_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(76.2, 92.361)), module, ModMatrix::_10_8_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(83.82, 92.361)), module, ModMatrix::_10_9_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(91.44, 92.361)), module, ModMatrix::_10_10_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(99.06, 92.361)), module, ModMatrix::_10_11_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(106.68, 92.361)), module, ModMatrix::_10_12_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(114.3, 92.36)), module, ModMatrix::_10_13_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(121.92, 92.361)), module, ModMatrix::_10_14_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(129.54, 92.361)), module, ModMatrix::_10_15_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(137.16, 92.361)), module, ModMatrix::_10_16_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(144.78, 92.359)), module, ModMatrix::SEL10_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(22.86, 100.391)), module, ModMatrix::_11_1_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(30.48, 100.391)), module, ModMatrix::_11_2_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(38.1, 100.39)), module, ModMatrix::_11_3_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(45.72, 100.391)), module, ModMatrix::_11_4_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(53.34, 100.391)), module, ModMatrix::_11_5_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(60.96, 100.391)), module, ModMatrix::_11_6_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(68.58, 100.391)), module, ModMatrix::_11_7_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(76.2, 100.391)), module, ModMatrix::_11_8_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(83.82, 100.391)), module, ModMatrix::_11_9_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(91.44, 100.391)), module, ModMatrix::_11_10_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(99.06, 100.391)), module, ModMatrix::_11_11_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(106.68, 100.391)), module, ModMatrix::_11_12_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(114.3, 100.39)), module, ModMatrix::_11_13_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(121.92, 100.391)), module, ModMatrix::_11_14_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(129.54, 100.391)), module, ModMatrix::_11_15_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(137.16, 100.391)), module, ModMatrix::_11_16_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(144.78, 100.39)), module, ModMatrix::SEL11_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(22.86, 108.423)), module, ModMatrix::_12_1_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(30.48, 108.423)), module, ModMatrix::_12_2_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(38.1, 108.422)), module, ModMatrix::_12_3_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(45.72, 108.423)), module, ModMatrix::_12_4_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(53.34, 108.422)), module, ModMatrix::_12_5_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(60.96, 108.422)), module, ModMatrix::_12_6_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(68.58, 108.423)), module, ModMatrix::_12_7_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(76.2, 108.423)), module, ModMatrix::_12_8_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(83.82, 108.423)), module, ModMatrix::_12_9_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(91.44, 108.423)), module, ModMatrix::_12_10_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(99.06, 108.423)), module, ModMatrix::_12_11_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(106.68, 108.423)), module, ModMatrix::_12_12_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(114.3, 108.422)), module, ModMatrix::_12_13_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(121.92, 108.423)), module, ModMatrix::_12_14_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(129.54, 108.423)), module, ModMatrix::_12_15_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(137.16, 108.423)), module, ModMatrix::_12_16_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(144.78, 108.422)), module, ModMatrix::SEL12_PARAM));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(15.24, 8.031)), module, ModMatrix::_0_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(15.24, 20.078)), module, ModMatrix::_1_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(15.24, 28.11)), module, ModMatrix::_2_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(15.24, 36.14)), module, ModMatrix::_3_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(15.24, 44.172)), module, ModMatrix::_4_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(15.24, 52.203)), module, ModMatrix::_5_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(15.24, 60.235)), module, ModMatrix::_6_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(15.24, 68.266)), module, ModMatrix::_7_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(15.24, 76.298)), module, ModMatrix::_8_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(15.24, 84.329)), module, ModMatrix::_9_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(15.24, 92.361)), module, ModMatrix::_10_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(15.24, 100.391)), module, ModMatrix::_11_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(15.24, 108.422)), module, ModMatrix::_12_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(22.86, 120.469)), module, ModMatrix::_1_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(30.48, 120.469)), module, ModMatrix::_2_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(38.1, 120.469)), module, ModMatrix::_3_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(45.72, 120.469)), module, ModMatrix::_4_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(53.34, 120.469)), module, ModMatrix::_5_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(60.96, 120.469)), module, ModMatrix::_6_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(68.58, 120.469)), module, ModMatrix::_7_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(76.2, 120.469)), module, ModMatrix::_8_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(83.82, 120.469)), module, ModMatrix::_9_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(91.44, 120.469)), module, ModMatrix::_10_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(99.06, 120.469)), module, ModMatrix::_11_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(106.68, 120.469)), module, ModMatrix::_12_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(114.3, 120.469)), module, ModMatrix::_13_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(121.92, 120.469)), module, ModMatrix::_14_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(129.54, 120.469)), module, ModMatrix::_15_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(137.16, 120.469)), module, ModMatrix::_16_OUTPUT));
	}
};


Model* modelModMatrix = createModel<ModMatrix, ModMatrixWidget>("ModMatrix");

}
