#include "plugin.hpp"
#include "../dsp/odeFilters.hpp"

namespace musx {

using namespace rack;
using simd::float_4;

class FilterBlock {
private:
	Method method = Method::RK4;
	IntegratorType integratorType = IntegratorType::Transistor;
	NonlinearityType nonlinearityType = NonlinearityType::alt3;

	Filter1Pole<float_4> filter1Pole;
	LadderFilter2Pole<float_4> ladderFilter2Pole;
	LadderFilter4Pole<float_4> ladderFilter4Pole;
	SallenKeyFilterLpBp<float_4> sallenKeyFilterLpBp;
	SallenKeyFilterHp<float_4> sallenKeyFilterHp;

public:
	constexpr std::vector<std::string> getLabels()
	{
		std::vector<std::string> labels = {
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
			"2-pole Sallen-Key highpass, 12 dB/Oct"
		};
		return labels;
	}

	void setIntegratorType(IntegratorType t)
	{
		integratorType = t;
			filter1Pole.setIntegratorType(integratorType);
			ladderFilter2Pole.setIntegratorType(integratorType);
			ladderFilter4Pole.setIntegratorType(integratorType);
			sallenKeyFilterLpBp.setIntegratorType(integratorType);
			sallenKeyFilterHp.setIntegratorType(integratorType);
	}

	void setNonlinearityType(NonlinearityType t)
	{
		filter1Pole.setNonlinearityType(nonlinearityType);
		ladderFilter2Pole.setNonlinearityType(nonlinearityType);
		ladderFilter4Pole.setNonlinearityType(nonlinearityType);
		sallenKeyFilterLpBp.setNonlinearityType(nonlinearityType);
		sallenKeyFilterHp.setNonlinearityType(nonlinearityType);
	}

	void setCutoffFrequencyAndResonance(float_4 frequency, float_4 resonance, int mode)
	{
		switch (mode)
		{
		case 0:
		case 1:
			filter1Pole.setCutoffFreq(frequency);
			filter1Pole.setResonance(resonance);
			break;
		case 2:
		case 3:
			ladderFilter2Pole.setCutoffFreq(frequency);
			ladderFilter2Pole.setResonance(resonance);
			break;
		case 4:
		case 5:
		case 6:
		case 7:
			ladderFilter4Pole.setCutoffFreq(frequency);
			ladderFilter4Pole.setResonance(resonance);
			break;
		case 8:
		case 9:
			sallenKeyFilterLpBp.setCutoffFreq(frequency);
			sallenKeyFilterLpBp.setResonance(resonance);
			break;
		case 10:
		case 11:
			sallenKeyFilterHp.setCutoffFreq(frequency);
			sallenKeyFilterHp.setResonance(resonance);
			break;
		}
	}

	float_4 process(float_4 in, float_4 dt, int mode)
	{
		switch (mode)
		{
		case 0:
			filter1Pole.process(in, dt, method);
			return filter1Pole.lowpass();
		case 1:
			filter1Pole.process(in, dt, method);
			return filter1Pole.highpass();
		case 2:
			ladderFilter2Pole.process(in, dt, method);
			return ladderFilter2Pole.lowpass();
		case 3 :
			ladderFilter2Pole.process(in, dt, method);
			return ladderFilter2Pole.bandpass();
		case 4:
			ladderFilter4Pole.process(in, dt, method);
			return ladderFilter4Pole.lowpass6();
		case 5:
			ladderFilter4Pole.process(in, dt, method);
			return ladderFilter4Pole.lowpass12();
		case 6:
			ladderFilter4Pole.process(in, dt, method);
			return ladderFilter4Pole.lowpass18();
		case 7:
			ladderFilter4Pole.process(in, dt, method);
			return ladderFilter4Pole.lowpass24();
		case 8:
			sallenKeyFilterLpBp.process(in, dt, method);
			return sallenKeyFilterLpBp.lowpass();
		case 9:
			sallenKeyFilterLpBp.process(in, dt, method);
			return sallenKeyFilterLpBp.bandpass();
		case 10:
			sallenKeyFilterHp.process(in, dt, method);
			return sallenKeyFilterHp.highpass6();
		case 11:
			sallenKeyFilterHp.process(in, dt, method);
			return sallenKeyFilterHp.highpass12();
		default:
			return in;
		}
	}
};

}
