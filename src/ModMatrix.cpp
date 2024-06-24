#include "plugin.hpp"

namespace musx {

using namespace rack;
using simd::float_4;

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
		SEL1_PARAM,
		SEL2_PARAM,
		SEL3_PARAM,
		SEL4_PARAM,
		SEL5_PARAM,
		SEL6_PARAM,
		SEL7_PARAM,
		SEL8_PARAM,
		SEL9_PARAM,
		SEL10_PARAM,
		SEL11_PARAM,
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
		_KNOB_BASE_VALUES_OUTPUT,
		_KNOB_CURRENT_VALUES_OUTPUT,
		OUTPUTS_LEN
	};
	enum LightId {
		SEL1_LIGHT,
		SEL2_LIGHT,
		SEL3_LIGHT,
		SEL4_LIGHT,
		SEL5_LIGHT,
		SEL6_LIGHT,
		SEL7_LIGHT,
		SEL8_LIGHT,
		SEL9_LIGHT,
		SEL10_LIGHT,
		SEL11_LIGHT,
		SEL12_LIGHT,
		LIGHTS_LEN
	};

	static constexpr size_t rows = 12;
	static constexpr size_t columns = 16;
	int channels = 1;

	std::vector<Input*> ins;
	std::vector<std::vector<Param*>> matrix;
	std::vector<Output*> outs;

	std::vector<Param*> controlKnobs;
	std::vector<float> controlKnobBaseValues; // 'base' values of the control knobs when not controlling other rows
	std::vector<float> currentControlKnobValues;
	std::vector<float> previousControlKnobValues;
	std::vector<float> midiControlKnobValues;
	std::vector<float> previousMidiControlKnobValues;
	std::vector<Param*> controlSelectors;
	size_t prevSelectedControl = 0;

	int sampleRateReduction = 1;
	bool latchButtons = false;
	bool bipolar = true;
	bool relative = false; // relative midi control mode

	dsp::ClockDivider controlDivider;
	dsp::ClockDivider matrixDivider;

	ModMatrix() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);

		for (size_t j = 0; j < columns; j++)
		{
			configParam(CTRL1_PARAM + j, -1.f * bipolar, 1.f, 0.f, "Control " + std::to_string(j+1), " %", 0.f, 100.f);
			getParamQuantity(CTRL1_PARAM + j)->smoothEnabled = false;

			controlKnobs.push_back(&params[CTRL1_PARAM + j]);
			controlKnobBaseValues.push_back(params[CTRL1_PARAM + j].getValue());
			currentControlKnobValues.push_back(params[CTRL1_PARAM + j].getValue());
			previousControlKnobValues.push_back(params[CTRL1_PARAM + j].getValue());
			midiControlKnobValues.push_back(-2);
			previousMidiControlKnobValues.push_back(-2);
		}

		for (size_t i = 0; i < rows; i++)
		{
			std::vector<Param*> row;
			for (size_t j = 0; j < columns; j++)
			{
				size_t iParam = _1_1_PARAM + i*columns + j;
				configParam(iParam, -1.f * bipolar, 1.f, 0.f, "Input " + std::to_string(i+1) + " to Mix " + std::to_string(j+1), " %", 0.f, 100.f);
				row.push_back(&params[iParam]);
			}
			matrix.push_back(row);
		}
		setPolarity();

		for (size_t i = 0; i < rows; i++)
		{
			configParam(SEL1_PARAM + i,  0.f, 1.f, 0.f, "Select Row " + std::to_string(i+1) + " for control");
			getParamQuantity(SEL1_PARAM + i)->snapEnabled = true;
			getParamQuantity(SEL1_PARAM + i)->randomizeEnabled = false;
			controlSelectors.push_back(&params[SEL1_PARAM + i]);

			configInput(_1_INPUT + i, "Signal " + std::to_string(i+1));
			ins.push_back(&inputs[_1_INPUT + i]);
		}

		for (size_t j = 0; j < columns; j++)
		{
			configOutput(_1_OUTPUT + j, "Mix " + std::to_string(j+1));
			outs.push_back(&outputs[_1_OUTPUT + j]);
		}

		configOutput(_KNOB_BASE_VALUES_OUTPUT, "Base control knob values");
		configOutput(_KNOB_CURRENT_VALUES_OUTPUT, "Current control knob values");

		controlDivider.setDivision(1);
		matrixDivider.setDivision(1);
	}

	void setPolarity()
	{
		if (bipolar)
		{
			configInput(_0_INPUT, "Control knob base values (normalled to 5V)");
		}
		else
		{
			configInput(_0_INPUT, "Control knob base values (normalled to 10V)");
		}

		for (size_t j = 0; j < columns; j++)
		{
			for (size_t i = 0; i < rows + 1; i++)
			{
				size_t iParam = CTRL1_PARAM + i*columns + j;
				ParamQuantity* qty = paramQuantities[iParam];
				qty->minValue = -1.f * bipolar;
				params[iParam].setValue(std::max(qty->minValue, qty->getValue()));
			}
		}
	}

	void setSampleRateReduction(int arg)
	{
		sampleRateReduction = arg;
		controlDivider.setDivision(sampleRateReduction);
		matrixDivider.setDivision(sampleRateReduction);
	}

	void onReset(const ResetEvent& e) override
	{
		Module::onReset(e);
		for (size_t j = 0; j < columns; j++)
		{
			controlKnobBaseValues[j] = 0.;
			currentControlKnobValues[j] = 0.;
			previousControlKnobValues[j] = 0.;
			midiControlKnobValues[j] = -2;
			previousMidiControlKnobValues[j] = -2;
		}
	}

	void onAdd(const AddEvent& e) override
	{
		Module::onAdd(e);
		for (size_t j = 0; j < columns; j++)
		{
			controlKnobBaseValues[j] = controlKnobs[j]->getValue();
			currentControlKnobValues[j] = controlKnobs[j]->getValue();
			previousControlKnobValues[j] = controlKnobs[j]->getValue();
			midiControlKnobValues[j] = -2;
			previousMidiControlKnobValues[j] = -2;
		}
	}

	void onRandomize(const RandomizeEvent& e) override
	{
		Module::onRandomize(e);
		for (size_t j = 0; j < columns; j++)
		{
			controlKnobBaseValues[j] = controlKnobs[j]->getValue();
			currentControlKnobValues[j] = controlKnobs[j]->getValue();
			previousControlKnobValues[j] = controlKnobs[j]->getValue();
			midiControlKnobValues[j] = -2;
			previousMidiControlKnobValues[j] = -2;
		}
	}

	void process(const ProcessArgs& args) override
	{
		if (controlDivider.process())
		{
			//
			// channels
			//
			channels = 0;

			for (auto& in : ins)
			{
				channels = std::max(channels, in->getChannels());
			}

			for (auto& out : outs)
			{
				out->setChannels(channels);
			}

			outputs[_KNOB_BASE_VALUES_OUTPUT].setChannels(columns);
			outputs[_KNOB_CURRENT_VALUES_OUTPUT].setChannels(columns);

			//
			// control
			//
			// relative control
			for (size_t j = 0; j < columns; j++)
			{
				currentControlKnobValues[j] = controlKnobs[j]->getValue();
				if (currentControlKnobValues[j] != previousControlKnobValues[j])
				{
					midiControlKnobValues[j] = currentControlKnobValues[j];
					if (previousMidiControlKnobValues[j] < -1.1)
					{
						// first control via MIDI
						previousMidiControlKnobValues[j] = currentControlKnobValues[j];
					}
				}
			}


			// check if/which control button is pressed
			size_t selectedControl = 0; // 0 means no button is pressed
			for (size_t i = 0; i < rows; i++)
			{
				if (controlSelectors[i]->getValue())
				{
					selectedControl = i+1;
					break;
				}
			}

			// lights
			for (size_t i = 0; i < rows; i++)
			{
				lights[i].setBrightness(i == selectedControl-1);
			}

			// a control button is pressed
			if (selectedControl && selectedControl != prevSelectedControl)
			{
				for (size_t j = 0; j < columns; j++)
				{
					// set control knobs to values of row when control button is pressed
					float value = matrix[selectedControl-1][j]->getValue();
					controlKnobs[j]->setValue(value);
					currentControlKnobValues[j] = value;
					previousControlKnobValues[j] = value;
				}
			}
			// control buttons are released
			else if (prevSelectedControl && !selectedControl)
			{
				// set control knobs to previous values when control buttons are released
				for (size_t j = 0; j < columns; j++)
				{
					controlKnobs[j]->setValue(controlKnobBaseValues[j]);
					currentControlKnobValues[j] = controlKnobBaseValues[j];
					previousControlKnobValues[j] = controlKnobBaseValues[j];
				}
			}


			if (!selectedControl)
			{
				// update controlKnobBaseValues
				for (size_t j = 0; j < columns; j++)
				{
					float newValue;
					if (relative)
					{
						newValue = controlKnobBaseValues[j] + (midiControlKnobValues[j] - previousMidiControlKnobValues[j]);
					}
					else
					{
						newValue = currentControlKnobValues[j];
					}
					newValue = std::fmin(newValue, 1.);
					newValue = std::fmax(newValue, bipolar ? -1. : 0.);

					controlKnobs[j]->setValue(newValue);
					currentControlKnobValues[j] = newValue;
					controlKnobBaseValues[j] = newValue;
				}
			}
			else
			{
				// control selected rows
				for (size_t j = 0; j < columns; j++)
				{
					float newValue;
					if (relative)
					{
						newValue = matrix[selectedControl-1][j]->getValue() + midiControlKnobValues[j] - previousMidiControlKnobValues[j];
					}
					else
					{
						newValue = currentControlKnobValues[j];
					}
					newValue = std::fmin(newValue, 1.);
					newValue = std::fmax(newValue, bipolar ? -1. : 0.);

					controlKnobs[j]->setValue(newValue);
					currentControlKnobValues[j] = newValue;
					matrix[selectedControl-1][j]->setValue(newValue);
				}
			}


			// update 'previous' values
			prevSelectedControl = selectedControl;

			for (size_t j = 0; j < columns; j++)
			{
				previousMidiControlKnobValues[j] = midiControlKnobValues[j];
				previousControlKnobValues[j] = currentControlKnobValues[j];
			}
		}

		// knob values output
		for (size_t j = 0; j < columns; j++)
		{
			outputs[_KNOB_BASE_VALUES_OUTPUT].setVoltage(controlKnobBaseValues[j] * (bipolar ? 5. : 10.), j);
			outputs[_KNOB_CURRENT_VALUES_OUTPUT].setVoltage(currentControlKnobValues[j] * (bipolar ? 5. : 10.), j);
		}

		//
		// calc matrix
		//
		if (matrixDivider.process())
		{
			for (int c = 0; c < channels; c += 4) {
				// loop over outs
				for (size_t j = 0; j < columns; j++)
				{
					Output* out = outs[j];
					if (out->isConnected())
					{
						// loop over ins, multiply with params
						float_4 val = inputs[_0_INPUT].isConnected() ? inputs[_0_INPUT].getPolyVoltage(j) :
								bipolar ? 5. : 10.;
						val *= controlKnobBaseValues[j];

						for (size_t i = 0; i < rows; i++)
						{
							Input* in = ins[i];
							val += in->getPolyVoltageSimd<float_4>(c) * matrix[i][j]->getValue();
						}

						out->setVoltageSimd(simd::clamp(val, -12.f, 12.f), c);
					}
				}
			}
		}
	}

	json_t* dataToJson() override {
		json_t* rootJ = json_object();
		json_object_set_new(rootJ, "sampleRateReduction", json_integer(sampleRateReduction));
		json_object_set_new(rootJ, "latchButtons", json_boolean(latchButtons));
		json_object_set_new(rootJ, "bipolar", json_boolean(bipolar));
		json_object_set_new(rootJ, "relative", json_boolean(relative));
		return rootJ;
	}

	void dataFromJson(json_t* rootJ) override {
		json_t* sampleRateReductionJ = json_object_get(rootJ, "sampleRateReduction");
		if (sampleRateReductionJ)
		{
			setSampleRateReduction(json_integer_value(sampleRateReductionJ));
		}
		json_t* latchButtonsJ = json_object_get(rootJ, "latchButtons");
		if (latchButtonsJ)
		{
			latchButtons = json_boolean_value(latchButtonsJ);
		}
		json_t* bipolarJ = json_object_get(rootJ, "bipolar");
		if (bipolarJ)
		{
			bipolar = json_boolean_value(bipolarJ);
			setPolarity();
		}
		json_t* relativeJ = json_object_get(rootJ, "relative");
		if (relativeJ)
		{
			relative = json_boolean_value(relativeJ);
			setPolarity();
		}
	}
};


struct ModMatrixWidget : ModuleWidget {
	ModMatrixWidget(ModMatrix* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/ModMatrix.svg"), asset::plugin(pluginInstance, "res/ModMatrix-dark.svg")));

		addChild(createWidget<ThemedScrew>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ThemedScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ThemedScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ThemedScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(14.455, 8.29)), module, ModMatrix::CTRL1_PARAM));
		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(22.476, 8.29)), module, ModMatrix::CTRL2_PARAM));
		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(30.497, 8.29)), module, ModMatrix::CTRL3_PARAM));
		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(38.518, 8.29)), module, ModMatrix::CTRL4_PARAM));
		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(47.068, 8.29)), module, ModMatrix::CTRL5_PARAM));
		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(55.089, 8.29)), module, ModMatrix::CTRL6_PARAM));
		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(63.11, 8.29)), module, ModMatrix::CTRL7_PARAM));
		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(71.131, 8.29)), module, ModMatrix::CTRL8_PARAM));
		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(79.681, 8.29)), module, ModMatrix::CTRL9_PARAM));
		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(87.702, 8.29)), module, ModMatrix::CTRL10_PARAM));
		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(95.723, 8.29)), module, ModMatrix::CTRL11_PARAM));
		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(103.745, 8.29)), module, ModMatrix::CTRL12_PARAM));
		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(112.295, 8.29)), module, ModMatrix::CTRL13_PARAM));
		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(120.316, 8.29)), module, ModMatrix::CTRL14_PARAM));
		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(128.337, 8.29)), module, ModMatrix::CTRL15_PARAM));
		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(136.358, 8.29)), module, ModMatrix::CTRL16_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(14.692, 18.601)), module, ModMatrix::_1_1_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(22.713, 18.601)), module, ModMatrix::_1_2_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(30.734, 18.601)), module, ModMatrix::_1_3_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(38.755, 18.601)), module, ModMatrix::_1_4_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(47.306, 18.601)), module, ModMatrix::_1_5_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(55.327, 18.601)), module, ModMatrix::_1_6_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(63.348, 18.601)), module, ModMatrix::_1_7_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(71.369, 18.601)), module, ModMatrix::_1_8_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(79.919, 18.601)), module, ModMatrix::_1_9_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(87.94, 18.601)), module, ModMatrix::_1_10_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(95.961, 18.601)), module, ModMatrix::_1_11_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(103.982, 18.601)), module, ModMatrix::_1_12_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(112.533, 18.6)), module, ModMatrix::_1_13_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(120.554, 18.601)), module, ModMatrix::_1_14_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(128.575, 18.601)), module, ModMatrix::_1_15_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(136.596, 18.601)), module, ModMatrix::_1_16_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(14.692, 26.89)), module, ModMatrix::_2_1_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(22.713, 26.891)), module, ModMatrix::_2_2_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(30.734, 26.89)), module, ModMatrix::_2_3_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(38.755, 26.891)), module, ModMatrix::_2_4_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(47.306, 26.891)), module, ModMatrix::_2_5_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(55.327, 26.891)), module, ModMatrix::_2_6_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(63.348, 26.891)), module, ModMatrix::_2_7_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(71.369, 26.891)), module, ModMatrix::_2_8_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(79.919, 26.891)), module, ModMatrix::_2_9_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(87.94, 26.89)), module, ModMatrix::_2_10_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(95.961, 26.891)), module, ModMatrix::_2_11_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(103.982, 26.891)), module, ModMatrix::_2_12_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(112.533, 26.89)), module, ModMatrix::_2_13_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(120.554, 26.891)), module, ModMatrix::_2_14_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(128.575, 26.891)), module, ModMatrix::_2_15_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(136.596, 26.891)), module, ModMatrix::_2_16_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(14.692, 35.181)), module, ModMatrix::_3_1_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(22.713, 35.182)), module, ModMatrix::_3_2_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(30.734, 35.181)), module, ModMatrix::_3_3_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(38.755, 35.182)), module, ModMatrix::_3_4_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(47.306, 35.182)), module, ModMatrix::_3_5_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(55.327, 35.182)), module, ModMatrix::_3_6_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(63.348, 35.182)), module, ModMatrix::_3_7_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(71.369, 35.182)), module, ModMatrix::_3_8_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(79.919, 35.182)), module, ModMatrix::_3_9_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(87.94, 35.182)), module, ModMatrix::_3_10_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(95.961, 35.182)), module, ModMatrix::_3_11_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(103.982, 35.182)), module, ModMatrix::_3_12_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(112.533, 35.181)), module, ModMatrix::_3_13_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(120.554, 35.182)), module, ModMatrix::_3_14_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(128.575, 35.182)), module, ModMatrix::_3_15_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(136.596, 35.182)), module, ModMatrix::_3_16_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(14.692, 43.471)), module, ModMatrix::_4_1_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(22.713, 43.472)), module, ModMatrix::_4_2_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(30.734, 43.471)), module, ModMatrix::_4_3_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(38.755, 43.472)), module, ModMatrix::_4_4_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(47.306, 43.472)), module, ModMatrix::_4_5_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(55.327, 43.472)), module, ModMatrix::_4_6_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(63.348, 43.472)), module, ModMatrix::_4_7_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(71.369, 43.472)), module, ModMatrix::_4_8_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(79.919, 43.472)), module, ModMatrix::_4_9_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(87.94, 43.472)), module, ModMatrix::_4_10_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(95.961, 43.472)), module, ModMatrix::_4_11_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(103.982, 43.472)), module, ModMatrix::_4_12_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(112.533, 43.471)), module, ModMatrix::_4_13_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(120.554, 43.472)), module, ModMatrix::_4_14_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(128.575, 43.472)), module, ModMatrix::_4_15_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(136.596, 43.472)), module, ModMatrix::_4_16_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(14.692, 51.762)), module, ModMatrix::_5_1_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(22.713, 51.762)), module, ModMatrix::_5_2_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(30.734, 51.761)), module, ModMatrix::_5_3_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(38.755, 51.762)), module, ModMatrix::_5_4_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(47.306, 51.762)), module, ModMatrix::_5_5_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(55.327, 51.762)), module, ModMatrix::_5_6_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(63.348, 51.762)), module, ModMatrix::_5_7_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(71.369, 51.762)), module, ModMatrix::_5_8_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(79.919, 51.762)), module, ModMatrix::_5_9_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(87.94, 51.763)), module, ModMatrix::_5_10_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(95.961, 51.763)), module, ModMatrix::_5_11_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(103.982, 51.763)), module, ModMatrix::_5_12_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(112.533, 51.762)), module, ModMatrix::_5_13_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(120.554, 51.763)), module, ModMatrix::_5_14_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(128.575, 51.763)), module, ModMatrix::_5_15_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(136.596, 51.763)), module, ModMatrix::_5_16_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(14.692, 60.053)), module, ModMatrix::_6_1_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(22.713, 60.053)), module, ModMatrix::_6_2_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(30.734, 60.052)), module, ModMatrix::_6_3_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(38.755, 60.053)), module, ModMatrix::_6_4_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(47.306, 60.053)), module, ModMatrix::_6_5_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(55.327, 60.053)), module, ModMatrix::_6_6_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(63.348, 60.053)), module, ModMatrix::_6_7_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(71.369, 60.053)), module, ModMatrix::_6_8_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(79.919, 60.053)), module, ModMatrix::_6_9_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(87.94, 60.053)), module, ModMatrix::_6_10_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(95.961, 60.053)), module, ModMatrix::_6_11_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(103.982, 60.053)), module, ModMatrix::_6_12_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(112.533, 60.052)), module, ModMatrix::_6_13_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(120.554, 60.053)), module, ModMatrix::_6_14_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(128.575, 60.053)), module, ModMatrix::_6_15_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(136.596, 60.053)), module, ModMatrix::_6_16_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(14.692, 68.342)), module, ModMatrix::_7_1_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(22.713, 68.342)), module, ModMatrix::_7_2_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(30.734, 68.341)), module, ModMatrix::_7_3_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(38.755, 68.342)), module, ModMatrix::_7_4_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(47.306, 68.342)), module, ModMatrix::_7_5_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(55.327, 68.342)), module, ModMatrix::_7_6_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(63.348, 68.342)), module, ModMatrix::_7_7_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(71.369, 68.342)), module, ModMatrix::_7_8_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(79.919, 68.342)), module, ModMatrix::_7_9_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(87.94, 68.343)), module, ModMatrix::_7_10_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(95.961, 68.343)), module, ModMatrix::_7_11_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(103.982, 68.343)), module, ModMatrix::_7_12_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(112.533, 68.342)), module, ModMatrix::_7_13_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(120.554, 68.343)), module, ModMatrix::_7_14_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(128.575, 68.343)), module, ModMatrix::_7_15_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(136.596, 68.343)), module, ModMatrix::_7_16_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(14.692, 76.632)), module, ModMatrix::_8_1_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(22.713, 76.632)), module, ModMatrix::_8_2_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(30.734, 76.631)), module, ModMatrix::_8_3_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(38.755, 76.632)), module, ModMatrix::_8_4_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(47.306, 76.632)), module, ModMatrix::_8_5_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(55.327, 76.632)), module, ModMatrix::_8_6_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(63.348, 76.632)), module, ModMatrix::_8_7_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(71.369, 76.632)), module, ModMatrix::_8_8_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(79.919, 76.632)), module, ModMatrix::_8_9_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(87.94, 76.633)), module, ModMatrix::_8_10_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(95.961, 76.633)), module, ModMatrix::_8_11_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(103.982, 76.633)), module, ModMatrix::_8_12_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(112.533, 76.632)), module, ModMatrix::_8_13_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(120.554, 76.633)), module, ModMatrix::_8_14_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(128.575, 76.633)), module, ModMatrix::_8_15_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(136.596, 76.633)), module, ModMatrix::_8_16_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(14.692, 84.923)), module, ModMatrix::_9_1_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(22.713, 84.923)), module, ModMatrix::_9_2_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(30.734, 84.922)), module, ModMatrix::_9_3_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(38.755, 84.923)), module, ModMatrix::_9_4_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(47.306, 84.923)), module, ModMatrix::_9_5_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(55.327, 84.923)), module, ModMatrix::_9_6_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(63.348, 84.923)), module, ModMatrix::_9_7_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(71.369, 84.923)), module, ModMatrix::_9_8_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(79.919, 84.923)), module, ModMatrix::_9_9_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(87.94, 84.923)), module, ModMatrix::_9_10_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(95.961, 84.923)), module, ModMatrix::_9_11_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(103.982, 84.923)), module, ModMatrix::_9_12_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(112.533, 84.922)), module, ModMatrix::_9_13_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(120.554, 84.923)), module, ModMatrix::_9_14_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(128.575, 84.923)), module, ModMatrix::_9_15_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(136.596, 84.923)), module, ModMatrix::_9_16_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(14.692, 93.212)), module, ModMatrix::_10_1_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(22.713, 93.213)), module, ModMatrix::_10_2_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(30.734, 93.212)), module, ModMatrix::_10_3_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(38.755, 93.213)), module, ModMatrix::_10_4_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(47.306, 93.213)), module, ModMatrix::_10_5_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(55.327, 93.213)), module, ModMatrix::_10_6_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(63.348, 93.213)), module, ModMatrix::_10_7_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(71.369, 93.213)), module, ModMatrix::_10_8_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(79.919, 93.213)), module, ModMatrix::_10_9_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(87.94, 93.214)), module, ModMatrix::_10_10_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(95.961, 93.214)), module, ModMatrix::_10_11_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(103.982, 93.214)), module, ModMatrix::_10_12_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(112.533, 93.212)), module, ModMatrix::_10_13_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(120.554, 93.214)), module, ModMatrix::_10_14_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(128.575, 93.214)), module, ModMatrix::_10_15_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(136.596, 93.214)), module, ModMatrix::_10_16_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(14.692, 101.503)), module, ModMatrix::_11_1_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(22.713, 101.503)), module, ModMatrix::_11_2_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(30.734, 101.502)), module, ModMatrix::_11_3_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(38.755, 101.503)), module, ModMatrix::_11_4_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(47.306, 101.503)), module, ModMatrix::_11_5_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(55.327, 101.503)), module, ModMatrix::_11_6_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(63.348, 101.503)), module, ModMatrix::_11_7_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(71.369, 101.503)), module, ModMatrix::_11_8_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(79.919, 101.503)), module, ModMatrix::_11_9_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(87.94, 101.504)), module, ModMatrix::_11_10_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(95.961, 101.504)), module, ModMatrix::_11_11_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(103.982, 101.504)), module, ModMatrix::_11_12_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(112.533, 101.503)), module, ModMatrix::_11_13_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(120.554, 101.504)), module, ModMatrix::_11_14_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(128.575, 101.504)), module, ModMatrix::_11_15_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(136.596, 101.504)), module, ModMatrix::_11_16_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(14.692, 109.795)), module, ModMatrix::_12_1_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(22.713, 109.795)), module, ModMatrix::_12_2_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(30.734, 109.794)), module, ModMatrix::_12_3_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(38.755, 109.795)), module, ModMatrix::_12_4_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(47.306, 109.794)), module, ModMatrix::_12_5_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(55.327, 109.794)), module, ModMatrix::_12_6_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(63.348, 109.795)), module, ModMatrix::_12_7_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(71.369, 109.795)), module, ModMatrix::_12_8_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(79.919, 109.795)), module, ModMatrix::_12_9_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(87.94, 109.795)), module, ModMatrix::_12_10_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(95.961, 109.795)), module, ModMatrix::_12_11_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(103.982, 109.795)), module, ModMatrix::_12_12_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(112.533, 109.794)), module, ModMatrix::_12_13_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(120.554, 109.795)), module, ModMatrix::_12_14_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(128.575, 109.795)), module, ModMatrix::_12_15_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(136.596, 109.795)), module, ModMatrix::_12_16_PARAM));

		addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<WhiteLight>>>(mm2px(Vec(144.617,  18.601)), module, ModMatrix::SEL1_PARAM, ModMatrix::SEL1_LIGHT));
		addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<WhiteLight>>>(mm2px(Vec(144.617,  26.891)), module, ModMatrix::SEL2_PARAM, ModMatrix::SEL2_LIGHT));
		addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<WhiteLight>>>(mm2px(Vec(144.617,  35.182)), module, ModMatrix::SEL3_PARAM, ModMatrix::SEL3_LIGHT));
		addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<WhiteLight>>>(mm2px(Vec(144.617,  43.472)), module, ModMatrix::SEL4_PARAM, ModMatrix::SEL4_LIGHT));
		addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<WhiteLight>>>(mm2px(Vec(144.617,  51.762)), module, ModMatrix::SEL5_PARAM, ModMatrix::SEL5_LIGHT));
		addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<WhiteLight>>>(mm2px(Vec(144.617,  60.053)), module, ModMatrix::SEL6_PARAM, ModMatrix::SEL6_LIGHT));
		addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<WhiteLight>>>(mm2px(Vec(144.617,  68.342)), module, ModMatrix::SEL7_PARAM, ModMatrix::SEL7_LIGHT));
		addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<WhiteLight>>>(mm2px(Vec(144.617,  76.632)), module, ModMatrix::SEL8_PARAM, ModMatrix::SEL8_LIGHT));
		addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<WhiteLight>>>(mm2px(Vec(144.617,  84.923)), module, ModMatrix::SEL9_PARAM, ModMatrix::SEL9_LIGHT));
		addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<WhiteLight>>>(mm2px(Vec(144.617,  93.213)), module, ModMatrix::SEL10_PARAM, ModMatrix::SEL10_LIGHT));
		addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<WhiteLight>>>(mm2px(Vec(144.617, 101.503)), module, ModMatrix::SEL11_PARAM, ModMatrix::SEL11_LIGHT));
		addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<WhiteLight>>>(mm2px(Vec(144.617, 109.795)), module, ModMatrix::SEL12_PARAM, ModMatrix::SEL12_LIGHT));

		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(5.904, 8.819)), module, ModMatrix::_0_INPUT));
		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(6.142, 18.601)), module, ModMatrix::_1_INPUT));
		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(6.142, 26.891)), module, ModMatrix::_2_INPUT));
		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(6.142, 35.181)), module, ModMatrix::_3_INPUT));
		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(6.142, 43.471)), module, ModMatrix::_4_INPUT));
		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(6.142, 51.762)), module, ModMatrix::_5_INPUT));
		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(6.142, 60.052)), module, ModMatrix::_6_INPUT));
		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(6.142, 68.342)), module, ModMatrix::_7_INPUT));
		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(6.142, 76.632)), module, ModMatrix::_8_INPUT));
		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(6.142, 84.923)), module, ModMatrix::_9_INPUT));
		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(6.142, 93.213)), module, ModMatrix::_10_INPUT));
		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(6.142, 101.503)), module, ModMatrix::_11_INPUT));
		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(6.142, 109.794)), module, ModMatrix::_12_INPUT));

		addOutput(createOutputCentered<ThemedPJ301MPort>(mm2px(Vec(14.692, 120.21)), module, ModMatrix::_1_OUTPUT));
		addOutput(createOutputCentered<ThemedPJ301MPort>(mm2px(Vec(22.713, 120.21)), module, ModMatrix::_2_OUTPUT));
		addOutput(createOutputCentered<ThemedPJ301MPort>(mm2px(Vec(30.734, 120.21)), module, ModMatrix::_3_OUTPUT));
		addOutput(createOutputCentered<ThemedPJ301MPort>(mm2px(Vec(38.755, 120.21)), module, ModMatrix::_4_OUTPUT));
		addOutput(createOutputCentered<ThemedPJ301MPort>(mm2px(Vec(47.306, 120.21)), module, ModMatrix::_5_OUTPUT));
		addOutput(createOutputCentered<ThemedPJ301MPort>(mm2px(Vec(55.327, 120.21)), module, ModMatrix::_6_OUTPUT));
		addOutput(createOutputCentered<ThemedPJ301MPort>(mm2px(Vec(63.348, 120.21)), module, ModMatrix::_7_OUTPUT));
		addOutput(createOutputCentered<ThemedPJ301MPort>(mm2px(Vec(71.369, 120.21)), module, ModMatrix::_8_OUTPUT));
		addOutput(createOutputCentered<ThemedPJ301MPort>(mm2px(Vec(79.919, 120.21)), module, ModMatrix::_9_OUTPUT));
		addOutput(createOutputCentered<ThemedPJ301MPort>(mm2px(Vec(87.94, 120.21)), module, ModMatrix::_10_OUTPUT));
		addOutput(createOutputCentered<ThemedPJ301MPort>(mm2px(Vec(95.961, 120.21)), module, ModMatrix::_11_OUTPUT));
		addOutput(createOutputCentered<ThemedPJ301MPort>(mm2px(Vec(103.982, 120.21)), module, ModMatrix::_12_OUTPUT));
		addOutput(createOutputCentered<ThemedPJ301MPort>(mm2px(Vec(112.533, 120.21)), module, ModMatrix::_13_OUTPUT));
		addOutput(createOutputCentered<ThemedPJ301MPort>(mm2px(Vec(120.554, 120.21)), module, ModMatrix::_14_OUTPUT));
		addOutput(createOutputCentered<ThemedPJ301MPort>(mm2px(Vec(128.575, 120.21)), module, ModMatrix::_15_OUTPUT));
		addOutput(createOutputCentered<ThemedPJ301MPort>(mm2px(Vec(136.596, 120.21)), module, ModMatrix::_16_OUTPUT));

		addOutput(createOutputCentered<ThemedPJ301MPort>(mm2px(Vec(144.617, 8.29)), module, ModMatrix::_KNOB_BASE_VALUES_OUTPUT));
		addOutput(createOutputCentered<ThemedPJ301MPort>(mm2px(Vec(152.638, 8.29)), module, ModMatrix::_KNOB_CURRENT_VALUES_OUTPUT));
	}

	void appendContextMenu(Menu* menu) override {
		ModMatrix* module = getModule<ModMatrix>();

		menu->addChild(new MenuSeparator);

		menu->addChild(createIndexSubmenuItem("Reduce internal sample rate", {"1x", "2x", "4x", "8x", "16x", "32x", "64x", "128x", "256x", "512x", "1024x"},
			[=]() {
				return log2(module->sampleRateReduction);
			},
			[=](int mode) {
				module->setSampleRateReduction(std::pow(2, mode));
			}
		));

		menu->addChild(createBoolMenuItem("Latch buttons", "",
			[=]() {
				return module->latchButtons;
			},
			[=](int mode) {
				module->latchButtons = mode;
				setLatch(module);

				// redraw
				event::Change c;
				for (ParamWidget* param : getParams())
				{
					param->onChange(c);
				}
			}
		));
		setLatch(module);

		menu->addChild(createBoolMenuItem("Bipolar", "",
			[=]() {
				return module->bipolar;
			},
			[=](int mode) {
				module->bipolar = mode;
				module->setPolarity();

				// redraw
				event::Change c;
				for (ParamWidget* param : getParams())
				{
					param->onChange(c);
				}
			}
		));

		menu->addChild(createBoolMenuItem("Relative MIDI control mode", "",
			[=]() {
				return module->relative;
			},
			[=](int mode) {
				module->relative = mode;
			}
		));
	}

	void setLatch(ModMatrix* module)
	{
		for (size_t i = module->SEL1_PARAM; i <= module->SEL12_PARAM; i++)
		{
			VCVLightLatch<MediumSimpleLight<WhiteLight>>* p = dynamic_cast<VCVLightLatch<MediumSimpleLight<WhiteLight>>*>(getParam(i));
			p->momentary = !module->latchButtons;
		}
	}
};

Model* modelModMatrix = createModel<ModMatrix, ModMatrixWidget>("ModMatrix");

}
