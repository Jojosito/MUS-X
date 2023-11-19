#include <rack.hpp>
#include "filters.hpp"

namespace musx {

using namespace rack;

template <typename T = float>
struct TCompander {

	// compressor
	musx::TOnePole<T> compAmplitude;

	// expander
	musx::TOnePole<T> expAmplitude;

	void setCompressorCutoffFreq(T f)
	{
		compAmplitude.setCutoffFreq(f);
	}

	void setExpanderCutoffFreq(T f)
	{
		expAmplitude.setCutoffFreq(f);
	}

	inline T compress(T in)
	{
		T gain = 1. / simd::fmax(compAmplitude.lowpass(), 0.1f);
		T out = gain * in;
		compAmplitude.process(simd::abs(out));
		return out;
	}

	inline T expand(T in)
	{
		expAmplitude.process(simd::abs(in));
		T gain = simd::fmin(expAmplitude.lowpass(), 10.f);
		return gain * in;
	}

	inline T compressorAmplitude()
	{
		return compAmplitude.lowpass();
	}
};


}
