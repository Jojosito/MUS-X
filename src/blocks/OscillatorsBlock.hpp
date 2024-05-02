#include "plugin.hpp"

namespace musx {

using namespace rack;
using simd::float_4;
using simd::int32_4;

class OscillatorsBlock {
private:
	// phase accumulators. integers overflow, so phase resets automatically
	int32_4 phasor1Sub[4] = {0};
	int32_4 phasor2[4] = {0};

	// sample rates
	int sampleRate;
	int oversamplingRate = 1;

	// parameters
	float_4 osc1Freq[4] = {0};
	float_4 osc1Shape[4] = {0};
	float_4 osc1PW[4] = {0};
	float_4 osc1Vol[4] = {0};
	float_4 osc1Subvol[4] = {0};

	float_4 osc2Freq[4] = {0};
	float_4 osc2Shape[4] = {0};
	float_4 osc2PW[4] = {0};
	float_4 osc2Vol[4] = {0};

	int32_4 sync[4];
	float_4 fmUnscaled[4] = {0};
	float_4 fmAmt[4] = {0};
	float_4 ringmodVol[4] = {0};

public:
	// set sample rate [Hz]
	inline void setSampleRate(int sr)
	{
		sampleRate = sr;
		for (int c = 0; c < 16; c += 4)
		{
			setFmAmount(fmUnscaled[c/4], c);
		}
	}

	inline void setOversamplingRate(int n)
	{
		oversamplingRate = n;
		for (int c = 0; c < 16; c += 4)
		{
			setFmAmount(fmUnscaled[c/4], c);
		}
	}

	// set oscillator 1 frequency [Hz]
	inline void setOsc1Freq(float_4 freq, int c)
	{
		osc1Freq[c/4] = freq;
	}

	// set oscillator 1 shape [-1..1]
	inline void setOsc1Shape(float_4 shape, int c)
	{
		osc1Shape[c/4] = simd::clamp(shape, -1.f, 1.f);
	}

	// set oscillator 1 pulsewidth [-1..1]
	inline void setOsc1PW(float_4 pw, int c)
	{
		osc1PW[c/4] = simd::clamp(pw, -1.f, 1.f);
	}

	// set oscillator 1 volume [0..1]
	inline void setOsc1Vol(float_4 vol, int c)
	{
		osc1Vol[c/4]  = simd::clamp(vol, 0.f, 1.f);
		osc1Vol[c/4] *= 10.f / INT32_MAX;
	}

	// set oscillator 1 suboscillator volume [0..1]
	inline void setOsc1Subvol(float_4 vol, int c)
	{
		osc1Subvol[c/4]  = simd::clamp(vol, 0.f, 1.f);
		osc1Subvol[c/4] *= 10.f / INT32_MAX;
	}


	// set oscillator 2 frequency [Hz]
	inline void setOsc2Freq(float_4 freq, int c)
	{
		osc2Freq[c/4] = freq;
	}

	// set oscillator 2 shape [-1..1]
	inline void setOsc2Shape(float_4 shape, int c)
	{
		osc2Shape[c/4] = simd::clamp(shape, -1.f, 1.f);
	}

	// set oscillator 2 pulsewidth [-1..1]
	inline void setOsc2PW(float_4 pw, int c)
	{
		osc2PW[c/4] = simd::clamp(pw, -1.f, 1.f);
	}

	// set oscillator 2 volume [0..1]
	inline void setOsc2Vol(float_4 vol, int c)
	{
		osc2Vol[c/4]  = simd::clamp(vol, 0.f, 1.f);
		osc2Vol[c/4] *= 10.f / INT32_MAX;
	}


	// set if oscillator 2 should be synced to oscillator 1 [0, 1]
	inline void setSync(int32_4 s, int c)
	{
		sync[c/4] = simd::round(clamp(s, 0.f, 1.f));
	}

	// set oscillator 1 -> oscillator 2 frequency-modulation amount [0..1]
	inline void setFmAmount(float_4 fm, int c)
	{
		fmUnscaled[c/4] = simd::clamp(fm, 0.f, 1.f);
		fmAmt[c/4] = fmUnscaled[c/4] * fmUnscaled[c/4] * 0.5f / oversamplingRate * 48000 / sampleRate; // scale
	}

	// set ringmodulator volume [0..1]
	inline void setRingmodVol(float_4 vol, int c)
	{
		ringmodVol[c/4]  = simd::clamp(vol, 0.f, 1.f);
		ringmodVol[c/4] *= 10.f / INT32_MAX / INT32_MAX;
	}


	// reset oscillator phases
	inline void resetPhases()
	{
		for (int c = 0; c < 16; c += 4)
		{
			phasor1Sub[c/4] = 0.;
			phasor2[c/4] = 0.;
		}
	}


	// calculate output, fill buffer with #oversamplingRate samples
	void process(float_4* buffer, int c)
	{
		int32_4 phase1SubInc = INT32_MAX / sampleRate * osc1Freq[c/4] / oversamplingRate;
		int32_4 phase1Inc = phase1SubInc + phase1SubInc;
		float_4 tri1Amt = 2.f * simd::fmax(-osc1Shape[c/4], 0.f);  // [2, 0, 0]
		float_4 sawSq1Amt = simd::fmin(1.f + osc1Shape[c/4], 1.f); // [0, 1, 1]
		float_4 sq1Amt = simd::fmax(osc1Shape[c/4], 0.f);          // [0, 0, 1]
		int32_4 phase1Offset = simd::ifelse(osc1PW[c/4] < 0, (-1.f - osc1PW[c/4]) * INT32_MAX, (1.f - osc1PW[c/4]) * INT32_MAX); // for pulse wave = saw + inverted saw with phaseshift

		int32_4 phase2Inc = INT32_MAX / sampleRate * osc2Freq[c/4] / oversamplingRate * 2;
		float_4 tri2Amt = 2.f * simd::fmax(-osc2Shape[c/4], 0.f);
		float_4 sawSq2Amt = simd::fmin(1.f + osc2Shape[c/4], 1.f);
		float_4 sq2Amt = simd::fmax(osc2Shape[c/4], 0.f);
		int32_4 phase2Offset = simd::ifelse(osc2PW[c/4] < 0, (-1.f - osc2PW[c/4]) * INT32_MAX, (1.f - osc2PW[c/4]) * INT32_MAX); // for pulse wave

		// calculate the oversampled oscillators and mix
		for (int i = 0; i < oversamplingRate; ++i)
		{
			// phasors for subosc 1 and osc 1
			phasor1Sub[c/4] += phase1SubInc;
			int32_4 phasor1 = phasor1Sub[c/4] + phasor1Sub[c/4];
			int32_4 phasor1Offset = phasor1 + phase1Offset;

			// osc 1 waveform
			float_4 wave1 = tri1Amt * ((1.f*phasor1Offset + (phasor1Offset > 0) * 2.f * phasor1Offset) + INT32_MAX/2); // +-INT32_MAX
			wave1 += sawSq1Amt * (phasor1Offset * sq1Amt - 1.f * phasor1); // +-INT32_MAX

			// osc 1 suboscillator
			float_4 sub1 = 1.f * (phasor1Sub[c/4] + INT32_MAX) - 1.f * phasor1Sub[c/4]; // +-INT32_MAX

			// phasor for osc 2
			phasor2[c/4] += phase2Inc + int32_4(fmAmt[c/4] * wave1);

			// sync / reset phasor2 ?
			phasor2[c/4] -= (sync[c/4] & (phasor1 + phase1Inc < phasor1)) * (phasor2[c/4] + INT32_MAX);
			int32_4 phasor2Offset = phasor2[c/4] + phase2Offset;

			// osc 2 waveform
			float_4 wave2 = tri2Amt * ((1.f*phasor2Offset + (phasor2Offset > 0) * 2.f * phasor2Offset) + INT32_MAX/2); // +-INT32_MAX
			wave2 += sawSq2Amt * (phasor2Offset * sq2Amt - 1.f * phasor2[c/4]); // +-INT32_MAX

			// mix
			float_4 out = osc1Subvol[c/4] * sub1 + osc1Vol[c/4] * wave1 + osc2Vol[c/4] * wave2 + ringmodVol[c/4] * wave1 * wave2; // +-5V each

			buffer[i] = out;
		}
	}
};

}
