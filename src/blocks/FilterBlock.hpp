#include "plugin.hpp"
#include "../dsp/odeFilters.hpp"

namespace musx {

using namespace rack;
using simd::float_4;

class FilterBlock {
private:
	Filter1Pole<float_4> filter1Pole;
	LadderFilter2Pole<float_4> ladderFilter2Pole;
	LadderFilter4Pole<float_4> ladderFilter4Pole;
	SallenKeyFilterLpBp<float_4> sallenKeyFilterLpBp;
	SallenKeyFilterHp<float_4> sallenKeyFilterHp;
	DiodeClipper<float_4> diodeClipper;
	DiodeClipperAsym<float_4> diodeClipperAsym;

public:
	static std::vector<std::string> getModeLabels()
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
			"2-pole Sallen-Key highpass, 12 dB/Oct",
			"Diode Clipper (Symmetric)",
			"Diode Clipper (Asymmetric)",
			"Bypass"
		};
		return labels;
	}

	static std::vector<std::string> getOdeSolverLabels()
	{
		std::vector<std::string> labels = {
			"1st order Euler",
			"2nd order Runge-Kutta",
			"4th order Runge-Kutta"
		};
		return labels;
	}

	static std::vector<std::string> getIntegratorTypeLabels()
	{
		std::vector<std::string> labels = {
			"Linear",
			"OTA with tanh",
			"OTA with alternate saturator",
			"Transistor with tanh",
			"Transistor with alternate saturator"
		};
		return labels;
	}

	static std::vector<std::string> getNonlinearityTypeLabels()
	{
		std::vector<std::string> labels = {
			"tanh",
			"alt"
		};
		return labels;
	}

	void setMethod(Method m)
	{
		filter1Pole.setMethod(m);
		ladderFilter2Pole.setMethod(m);
		ladderFilter4Pole.setMethod(m);
		sallenKeyFilterLpBp.setMethod(m);
		sallenKeyFilterHp.setMethod(m);
		diodeClipper.setMethod(m);
		diodeClipperAsym.setMethod(m);
	}

	void setIntegratorType(IntegratorType t)
	{
		filter1Pole.setIntegratorType(t);
		ladderFilter2Pole.setIntegratorType(t);
		ladderFilter4Pole.setIntegratorType(t);
		sallenKeyFilterLpBp.setIntegratorType(t);
		sallenKeyFilterHp.setIntegratorType(t);
		diodeClipper.setIntegratorType(t);
		diodeClipperAsym.setIntegratorType(t);
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
		case 12:
			diodeClipper.setCutoffFreq(frequency);
			diodeClipper.setResonance(resonance);
			break;
		case 13:
			diodeClipperAsym.setCutoffFreq(frequency);
			diodeClipperAsym.setResonance(resonance);
			break;
		}
	}

	float_4 process(float_4 in, float_4 dt, int mode)
	{
		switch (mode)
		{
		case 0:
			filter1Pole.process(in, dt);
			return filter1Pole.lowpass();
		case 1:
			filter1Pole.process(in, dt);
			return filter1Pole.highpass();
		case 2:
			ladderFilter2Pole.process(in, dt);
			return ladderFilter2Pole.lowpass();
		case 3 :
			ladderFilter2Pole.process(in, dt);
			return ladderFilter2Pole.bandpass();
		case 4:
			ladderFilter4Pole.process(in, dt);
			return ladderFilter4Pole.lowpass6();
		case 5:
			ladderFilter4Pole.process(in, dt);
			return ladderFilter4Pole.lowpass12();
		case 6:
			ladderFilter4Pole.process(in, dt);
			return ladderFilter4Pole.lowpass18();
		case 7:
			ladderFilter4Pole.process(in, dt);
			return ladderFilter4Pole.lowpass24();
		case 8:
			sallenKeyFilterLpBp.process(in, dt);
			return sallenKeyFilterLpBp.lowpass();
		case 9:
			sallenKeyFilterLpBp.process(in, dt);
			return sallenKeyFilterLpBp.bandpass();
		case 10:
			sallenKeyFilterHp.process(in, dt);
			return sallenKeyFilterHp.highpass6();
		case 11:
			sallenKeyFilterHp.process(in, dt);
			return sallenKeyFilterHp.highpass12();
		case 12:
			diodeClipper.process(in, dt);
			return diodeClipper.out();
		case 13:
			diodeClipperAsym.process(in, dt);
			return diodeClipperAsym.out();
		case 14:
		default:
			return in;
		}
	}
};

}
