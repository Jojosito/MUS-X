#include "plugin.hpp"
#include "../dsp/blep.hpp"
#include "../dsp/functions.hpp"

namespace musx {

using namespace rack;
using simd::float_4;
using simd::int32_4;

/**
 * O max oversampling
 */
template <size_t O>
class OscillatorsBlock {
private:
	// phase accumulators. integers overflow, so phase resets automatically
	int32_4 phasor1Sub[4] = {0};
	int32_4 phasor1Old[4] = {0};
	int32_4 phasor2[4] = {0};

	// sample rates
	int sampleRate;
	int oversamplingRate = 1;
	float oneOverSampleRateTimesOversamplingRate;

	// freq limits
	float minFreq = 0.0001f; // [Hz]
	float maxFreq = 20000.f; // [Hz]

	// parameters
	float_4 osc1Freq[4] = {0}; // [Hz]
	float_4 osc1Shape[4] = {0};
	float_4 osc1PW[4] = {0};
	float_4 osc1Vol[4] = {0};
	float_4 osc1Subvol[4] = {0};

	float_4 osc2Freq[4] = {0}; // [Hz]
	float_4 osc2Shape[4] = {0};
	float_4 osc2PW[4] = {0};
	float_4 osc2Vol[4] = {0};

	int32_4 sync[4];
	float_4 fmUnscaled[4] = {0};
	float_4 fmAmt[4] = {0};
	float_4 ringmodVol[4] = {0};

	// blep generators
	BlepGenerator<O, float_4> osc1Blep[4];
	BlepGenerator<1, float_4> osc2Blep[4];

public:
	// set sample rate [Hz]
	void setSampleRate(int sr)
	{
		sampleRate = sr;
		oneOverSampleRateTimesOversamplingRate = 1. / (sampleRate * oversamplingRate);
		for (int c = 0; c < 16; c += 4)
		{
			setFmAmount(fmUnscaled[c/4], c);
		}
	}

	// set oversampling factor
	void setOversamplingRate(int n)
	{
		oversamplingRate = n;
		oneOverSampleRateTimesOversamplingRate = 1. / (sampleRate * oversamplingRate);
		for (int c = 0; c < 16; c += 4)
		{
			setFmAmount(fmUnscaled[c/4], c);
		}
	}

	// set oscillators minimum frequency [Hz]
	void setMinFreq(float freq)
	{
		minFreq = freq;
	}

	// set oscillators maximum frequency [Hz]
	void setMaxFreq(float freq)
	{
		maxFreq = freq;
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


	// set oscillator 1 frequency in V/Oct. 0 V = C4 [V]
	inline void setOsc1FreqVOct(float_4 freq, int c)
	{
		osc1Freq[c/4] = simd::clamp(dsp::FREQ_C4 * dsp::exp2_taylor5(freq), minFreq, maxFreq);
	}

	// set oscillator 1 frequency in V/Oct. 0 V = 2 Hz [V]
	inline void setOsc1FreqVOctLFO(float_4 freq, int c)
	{
		osc1Freq[c/4] = simd::clamp(2. * dsp::exp2_taylor5(freq), minFreq, maxFreq);
	}

	// set oscillator 1 frequency [Hz]
	inline void setOsc1FreqHz(float_4 freq, int c)
	{
		osc1Freq[c/4] = simd::clamp(freq, minFreq, maxFreq);
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


	// set oscillator 2 frequency in V/Oct. 0 V = C4 [V]
	inline void setOsc2FreqVOct(float_4 freq, int c)
	{
		osc2Freq[c/4] = simd::clamp(dsp::FREQ_C4 * dsp::exp2_taylor5(freq), minFreq, maxFreq);
	}

	// set oscillator 2 frequency in V/Oct. 0 V = 2 Hz [V]
	inline void setOsc2FreqVOctLFO(float_4 freq, int c)
	{
		osc2Freq[c/4] = simd::clamp(2. * dsp::exp2_taylor5(freq), minFreq, maxFreq);
	}

	// set oscillator 2 frequency [Hz]
	inline void setOsc2Freq(float_4 freq, int c)
	{
		osc2Freq[c/4] = simd::clamp(freq, minFreq, maxFreq);
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


	// calculate output, fill buffer with #oversamplingRate samples
	// output can have DC offset when using fm or ringmod
	// output in NOT bound to +-10V. The individual components (osc1, subosc, osc2, ringmod) are within +-10V
	// it is recommended to feed the output through a DC blocker and saturator
	void process(float_4* buffer, int c)
	{
		int32_4 phase1SubInc = INT32_MAX * osc1Freq[c/4] * oneOverSampleRateTimesOversamplingRate;
		float_4 tri1Amt = -2.f * simd::fmax(-osc1Shape[c/4], 0.f);  // [2, 0, 0]
		float_4 sawSq1Amt = simd::fmin(1.f + osc1Shape[c/4], 1.f); // [0, 1, 1]
		float_4 sq1Amt = simd::fmax(osc1Shape[c/4], 0.f);          // [0, 0, 1]
		int32_4 phase1Offset = simd::ifelse(osc1PW[c/4] < 0, (-1.f - osc1PW[c/4]) * INT32_MAX, (1.f - osc1PW[c/4]) * INT32_MAX); // for pulse wave = saw + inverted saw with phaseshift

		int32_4 phase2Inc = INT32_MAX * osc2Freq[c/4] * 2 * oneOverSampleRateTimesOversamplingRate;
		float_4 tri2Amt = -2.f * simd::fmax(-osc2Shape[c/4], 0.f);
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
			float_4 wave1 = tri1Amt * (simd::abs(phasor1Offset) - INT32_MAX/2); // +-INT32_MAX
			wave1 += sawSq1Amt * (phasor1Offset * sq1Amt - 1.f * phasor1); // +-INT32_MAX

			// osc 1 suboscillator
			float_4 sub1 = 1.f * (phasor1Sub[c/4] + INT32_MAX) - 1.f * phasor1Sub[c/4]; // +-INT32_MAX

			// phasor for osc 2
			phasor2[c/4] += phase2Inc + int32_4(fmAmt[c/4] * wave1);

			// sync / reset phasor2 ?
			phasor2[c/4] = simd::ifelse(sync[c/4] & (phasor1Old[c/4] > phasor1), INT32_MIN, phasor2[c/4]);
			phasor1Old[c/4] = phasor1;
			int32_4 phasor2Offset = phasor2[c/4] + phase2Offset;

			// osc 2 waveform
			float_4 wave2 = tri2Amt * (simd::abs(phasor2Offset) - INT32_MAX/2); // +-INT32_MAX
			wave2 += sawSq2Amt * (phasor2Offset * sq2Amt - 1.f * phasor2[c/4]); // +-INT32_MAX

			// mix
			float_4 out = osc1Subvol[c/4] * sub1 + osc1Vol[c/4] * wave1 + osc2Vol[c/4] * wave2 + ringmodVol[c/4] * wave1 * wave2; // +-5V each

			buffer[i] = out;
		}
	}

	// calculate bandlimited output, fill buffer with #oversamplingRate samples
	// output can have DC offset when using fm or ringmod
	// output in NOT bound to +-10V. The individual components (osc1, subosc, osc2, ringmod) are within +-10V
	// it is recommended to feed the output through a DC blocker and saturator
	void processBandlimited(float_4* buffer, int c)
	{
		int32_4 phase1SubInc 	= INT32_MAX * osc1Freq[c/4] * oneOverSampleRateTimesOversamplingRate;
		int32_4 phase1Inc 		= phase1SubInc + phase1SubInc;
		float_4 tri1Amt 		= simd::fmax(-osc1Shape[c/4], 0.f);  		// [1, 0, 0]
		float_4 sawSq1Amt 		= simd::fmin(1.f + osc1Shape[c/4], 1.f); 	// [0, 1, 1]
		float_4 sq1Amt 			= simd::fmax(osc1Shape[c/4], 0.f);          // [0, 0, 1]
		int32_4 phase1Offset 	= simd::ifelse(osc1PW[c/4] < 0, (-1.f - osc1PW[c/4]) * INT32_MAX, (1.f - osc1PW[c/4]) * INT32_MAX); // for pulse wave = saw + inverted saw with phaseshift

		int calcTri1 = simd::movemask(tri1Amt > 0);
		int calcSawSq1 = simd::movemask(sawSq1Amt > 0);
		int calcSq1 = simd::movemask(sq1Amt > 0);
		int calcSub = simd::movemask(osc1Subvol[c/4] > 0);

		int32_4 phase2Inc 		= INT32_MAX * osc2Freq[c/4] * 2 * oneOverSampleRateTimesOversamplingRate;
		float_4 tri2Amt 		= simd::fmax(-osc2Shape[c/4], 0.f);
		float_4 sawSq2Amt 		= simd::fmin(1.f + osc2Shape[c/4], 1.f);
		float_4 sq2Amt 			= simd::fmax(osc2Shape[c/4], 0.f);
		int32_4 phase2Offset 	= simd::ifelse(osc2PW[c/4] < 0, (-1.f - osc2PW[c/4]) * INT32_MAX, (1.f - osc2PW[c/4]) * INT32_MAX); // for pulse wave

		int calcSync = simd::movemask(sync[c/4] > 0);
		int calcTri2 = simd::movemask(tri2Amt > 0);
		int calcSawSq2 = simd::movemask(sawSq2Amt > 0);
		int calcSq2 = simd::movemask(sq2Amt > 0);

		// calculate the oversampled oscillators and mix
		for (int i = 0; i < oversamplingRate; ++i)
		{
			float_4 wave1 = 0;
			float_4 wave2 = 0;
			float_4 out = 0;

			// phasors and bleps for subosc 1

			// phasors and bleps for osc 1
			int32_4 phasor1 = phasor1Sub[c/4] + phasor1Sub[c/4];
			int32_4 phasor1Offset = phasor1 + phase1Offset;

			if (calcTri1)
			{
				// triangle and blamp
				float_4 tri1 = -2 * simd::abs(phasor1Offset) + INT32_MAX; // +-INT32_MAX

				//osc1Blep[c/4].insertBlamp((INT32_MAX - (phasor1Offset + phasor1Offset)) / (2.*phase1Inc), simd::sgn((float_4)phasor1Offset) * tri1Amt * INT32_MAX, oversamplingRate);

				wave1 += tri1Amt * tri1; // +-INT32_MAX
			}

			if (calcSawSq1)
			{
				if (calcSq1)
				{
					wave1 += sawSq1Amt * (phasor1Offset * sq1Amt - 1.f * phasor1); // +-INT32_MAX

					//osc1Blep[c/4].insertBlep(float_4(INT32_MAX - phasor1Offset) / phase1Inc, sawSq1Amt * sq1Amt * INT32_MAX, oversamplingRate);
				}
				else
				{
					wave1 += -sawSq1Amt * phasor1; // +-INT32_MAX
				}

				osc1Blep[c/4].insertBlep(float_4(INT32_MAX - phasor1) / (oversamplingRate * phase1Inc), sawSq1Amt * INT32_MAX, oversamplingRate);
			}

			if (calcSub)
			{
				float_4 phasor1SubBlep = blep(phasor1Sub[c/4], oversamplingRate * phase1SubInc);
				int32_4 phasor1SubOffset = phasor1Sub[c/4] + INT32_MAX;
				float_4 phasor1SubOffsetBlep = blep(phasor1SubOffset, oversamplingRate * phase1SubInc);

				float_4 sub1 = 1.f * ((float_4)phasor1SubOffset + phasor1SubOffsetBlep) - 1.f * ((float_4)phasor1Sub[c/4] + phasor1SubBlep); // +-INT32_MAX

				out += osc1Subvol[c/4] * sub1;
			}

			phasor1Sub[c/4] += phase1SubInc;

			//
			// osc 2
			//

			// phasor for osc 2
			int32_4 phase2IncWithFm = phase2Inc + int32_4(fmAmt[c/4] * wave1);
			if (calcSync)
			{
				int32_4 doSync = sync[c/4] & (phasor1Old[c/4] > phasor1);
				if (simd::movemask(doSync > 0))
				{
					float_4 fractionalSyncTime = 1. - 1.*phasor1 / phase1Inc; // [0..1]

					phasor2[c/4] = simd::ifelse(doSync,
							INT32_MIN + (1. - fractionalSyncTime) * phase2IncWithFm,
							phasor2[c/4] + phase2IncWithFm);
				}
				else
				{
					phasor2[c/4] += phase2IncWithFm;
				}
			}
			else
			{
				phasor2[c/4] += phase2IncWithFm;
			}

			int32_4 phasor2Offset = phasor2[c/4] + phase2Offset;

			if (calcTri2)
			{
				// triangle and blamp
				float_4 tri2 = -2 * simd::abs(phasor2Offset) + INT32_MAX; // +-INT32_MAX
				float_4 tri2Blamp = blamp(tri2, phase2IncWithFm);

				wave2 += tri2Amt * (tri2 + tri2Blamp); // +-INT32_MAX
			}

			if (calcSawSq2)
			{
				//float_4 phasor2Blep = blep(phasor2[c/4], phase2IncWithFm);
				if (calcSq2)
				{
					//float_4 phasor2OffsetBlep = blep(phasor2Offset, phase2IncWithFm);
					wave2 += sawSq2Amt * (phasor2Offset * sq2Amt - 1.f * phasor2[c/4]); // +-INT32_MAX
				}
				else
				{
					wave2 += -sawSq2Amt * phasor2[c/4]; // +-INT32_MAX
				}
			}

			// apply bleps
			wave2 = wave1; // test
			wave1 = osc1Blep[c/4].process();
//			wave2 += osc2Blep[c/4].process();

			out += osc1Vol[c/4] * wave1 + osc2Vol[c/4] * wave2 + ringmodVol[c/4] * wave1 * wave2; // +-5V each

			buffer[i] = out;
//			buffer[i] = osc1Blep[c/4].process();

			// bookkeeping
			phasor1Old[c/4] = phasor1;
		}
	}

private:
	/**
	 * polyBLEP for a ramp wave [-INT32_MAX..INT32_MAX]
	 */
	float_4 blep(float_4 ramp, float_4 deltaY)
	{
		float_4 thresholdY = INT32_MAX - deltaY;
		float_4 blep = 0;

		// step is less than deltaY ahead
		float_4 x = (ramp - INT32_MAX) / deltaY; // [-1..0]
		blep -= simd::ifelse(ramp > thresholdY, x*x+x+x+1., 0);

		// step was less than deltaY ago
		x = (ramp + INT32_MAX) / deltaY; // [0..1]
		blep -= simd::ifelse(ramp < -thresholdY, x+x-x*x-1., 0);

		return blep * INT32_MAX;
	}

	/**
	 * polyBLEP (only pre-step) for a ramp wave [-INT32_MAX..INT32_MAX]
	 */
	float_4 blepPre(float_4 ramp, float_4 deltaY)
	{
		float_4 thresholdY = INT32_MAX - deltaY;
		float_4 blep = 0;

		// step is less than deltaY ahead
		float_4 x = (ramp - INT32_MAX) / deltaY; // [-1..0]
		blep -= simd::ifelse(ramp > thresholdY, x*x+x+x+1., 0);

		return blep * INT32_MAX;
	}

	/**
	 * polyBLEP (only post-step) for a ramp wave [-INT32_MAX..INT32_MAX]
	 */
	float_4 blepPost(float_4 ramp, float_4 deltaY)
	{
		float_4 thresholdY = INT32_MAX - deltaY;
		float_4 blep = 0;

		// step was less than deltaY ago
		float_4 x = (ramp + INT32_MAX) / deltaY; // [0..1]
		blep -= simd::ifelse(ramp < -thresholdY, x+x-x*x-1., 0);

		return blep * INT32_MAX;
	}

	/**
	 * polyBLAMP for a triangle wave [-INT32_MAX..INT32_MAX]
	 */
	float_4 blamp(float_4 tri, float_4 deltaY)
	{
		float_4 thresholdY = INT32_MAX - deltaY;
		float_4 blamp = 0;

		float_4 factor = 0.5 * deltaY / INT32_MAX;

		// bottom of triangle
		float_4 x = (tri + INT32_MAX) / deltaY; // [1..0..1]
		blamp += simd::ifelse(tri < -thresholdY, factor*(x-1)*(x-1), 0);

		// top of triangle
		x = (tri - INT32_MAX) / deltaY; // [-1..0..-1]
		blamp -= simd::ifelse(tri > thresholdY, factor*(x+1)*(x+1), 0);

		return blamp * INT32_MAX;
	}
};

}
