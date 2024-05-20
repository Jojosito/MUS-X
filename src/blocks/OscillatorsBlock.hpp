#include "plugin.hpp"
#include "../dsp/blep.hpp"
#include "../dsp/filters.hpp"
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
	static constexpr size_t blepSize = 4;
	BlepGenerator<O, blepSize, float_4> osc1Blep[4];
	BlepGenerator<O, blepSize, float_4> oscSubBlep[4];
	BlepGenerator<1, blepSize, float_4> osc2Blep[4];

	// buffer for applying 4-point blep
	size_t bufferReadIndex = 0;
	size_t bufferWriteIndex = oversamplingRate;
	float_4 prevSub1[4][O] = {0};
	float_4 prevWave1[4][O] = {0};
	float_4 prevWave2[4] = {0};

	musx::TOnePoleZDF<float_4> lowpass[4];


public:
	// set sample rate [Hz]
	void setSampleRate(int sr)
	{
		sampleRate = sr;
		setOversamplingRate(oversamplingRate);
	}

	// set oversampling factor
	void setOversamplingRate(int n)
	{
		oversamplingRate = std::min(O, size_t(n));
		oneOverSampleRateTimesOversamplingRate = 1. / (sampleRate * oversamplingRate);

		for (int c = 0; c < 16; c += 4)
		{
			lowpass[c/4].setCutoffFreq(14000. / (sampleRate * oversamplingRate));
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
		setFmAmount(fmUnscaled[c/4], c);
	}

	// set oscillator 1 frequency in V/Oct. 0 V = 2 Hz [V]
	inline void setOsc1FreqVOctLFO(float_4 freq, int c)
	{
		osc1Freq[c/4] = simd::clamp(2. * dsp::exp2_taylor5(freq), minFreq, maxFreq);
		setFmAmount(fmUnscaled[c/4], c);
	}

	// set oscillator 1 frequency [Hz]
	inline void setOsc1FreqHz(float_4 freq, int c)
	{
		osc1Freq[c/4] = simd::clamp(freq, minFreq, maxFreq);
		setFmAmount(fmUnscaled[c/4], c);
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
		setFmAmount(fmUnscaled[c/4], c);
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

		// use adapted carsons rule to limit FM amount
		// value 7000 is chosen so that
		// osc 2 fm'ed fundamental frequency stays below nyquist (oversamplingRate*sampleRate/2.)
		float_4 maxFm = simd::fmax(
				((oversamplingRate*sampleRate/2. - osc2Freq[c/4]) / 2. - osc1Freq[c/4]) / 7000.,
				0.f);
		fmAmt[c/4] = simd::fmin(fmUnscaled[c/4] * fmUnscaled[c/4], maxFm);

		fmAmt[c/4] = fmAmt[c/4] * 0.5f / oversamplingRate * 48000 / sampleRate; // scale
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
			float_4 sub1 = 0;
			float_4 wave1 = 0;
			float_4 wave2 = 0;
			float_4 out = 0;


			//
			// osc 1 ============================================================================================================
			//

			// phasors for osc 1
			int32_4 phasor1 = phasor1Sub[c/4] + phasor1Sub[c/4];
			int32_4 phasor1Offset = phasor1 + phase1Offset;

			if (calcTri1)
			{
				float_4 tri1 = -2 * simd::abs(phasor1Offset) + INT32_MAX; // +-INT32_MAX

				wave1 += tri1Amt * tri1; // +-INT32_MAX

				osc1Blep[c/4].insertBlamp(
						(1.f * INT32_MAX - 1.f * (phasor1Offset + phasor1Offset + INT32_MAX)) / (oversamplingRate * 2.f * phase1Inc),
						simd::sgn(float_4(phasor1Offset)) * tri1Amt * phase1Inc,
						oversamplingRate,
						1. - 1. / oversamplingRate);
			}

			if (calcSawSq1)
			{
				if (calcSq1)
				{
					wave1 += sawSq1Amt * (sq1Amt * phasor1Offset - 1.f * phasor1); // +-INT32_MAX

					osc1Blep[c/4].insertBlep(
							(INT32_MAX - phasor1Offset) / (1.f * oversamplingRate * phase1Inc),
							-sawSq1Amt * sq1Amt * INT32_MAX,
							oversamplingRate,
							1. - 1. / oversamplingRate);
				}
				else
				{
					wave1 += -sawSq1Amt * phasor1; // +-INT32_MAX
				}

				osc1Blep[c/4].insertBlep(
						(INT32_MAX - phasor1) / (1.f * oversamplingRate * phase1Inc),
						sawSq1Amt * INT32_MAX,
						oversamplingRate,
						1. - 1. / oversamplingRate);
			}

			if (calcSub)
			{
				int32_4 phasor1SubOffset = phasor1Sub[c/4] + INT32_MAX;

				sub1 = 1.f * phasor1SubOffset - 1.f * phasor1Sub[c/4]; // +-INT32_MAX

				oscSubBlep[c/4].insertBlep(
						(INT32_MAX - phasor1Sub[c/4]) / (1.f * oversamplingRate * phase1SubInc),
						INT32_MAX,
						oversamplingRate,
						1. - 1. / oversamplingRate);

				oscSubBlep[c/4].insertBlep(
						(INT32_MAX - phasor1SubOffset) / (1.f * oversamplingRate * phase1SubInc),
						-INT32_MAX,
						oversamplingRate,
						1. - 1. / oversamplingRate);

				out += osc1Subvol[c/4] * (prevSub1[c/4][bufferReadIndex] + oscSubBlep[c/4].process());
			}

			phasor1Sub[c/4] += phase1SubInc;

			// apply bleps
			prevWave1[c/4][bufferReadIndex] += osc1Blep[c/4].process();


			//
			// osc 2 ============================================================================================================
			//

			// phasors for osc 2
			int32_4 phase2IncWithFm = phase2Inc + int32_4(fmAmt[c/4] * prevWave1[c/4][bufferReadIndex]); // can be negative!

			float_4 blep2Scale = simd::sgn(float_4(phase2IncWithFm)) * INT32_MAX; // [-INT32_MAX, INT32_MAX]
			if (calcSync)
			{
				int32_4 doSync = sync[c/4] & (phasor1 + phase1Inc < phasor1);
				if (simd::movemask(doSync > 0))
				{
					float_4 fractionalSyncTime = (INT32_MAX - phasor1) / (1.f * phase1Inc); // [0..1]
					fractionalSyncTime = simd::clamp(fractionalSyncTime, 0.f, 1.f);
					fractionalSyncTime = simd::ifelse(doSync, fractionalSyncTime, 1.f); // get rid of some numerical errors

					// calc osc2 and bleps from sample begin to fractionalSyncTime
					calcOsc2(phase2Offset,
							phase2IncWithFm,
							calcTri2, tri2Amt,
							calcSawSq2, sawSq2Amt,
							calcSq2, sq2Amt,
							blep2Scale,
							wave2,
							c,
							0.f, fractionalSyncTime);

					// calc osc2 wave right before sync for blep scale
					float_4 wave2BeforeSync = 0.f;
					calcOsc2Wave(phase2Offset,
							calcTri2, tri2Amt,
							calcSawSq2, sawSq2Amt,
							calcSq2, sq2Amt,
							wave2BeforeSync,
							c);

					// sync? -> reset phasor2
					phasor2[c/4] = simd::ifelse(doSync,
							simd::ifelse(blep2Scale > 0, INT32_MIN, INT32_MAX),
							phasor2[c/4]);

					// calc osc2 wave right after sync for blep scale
					float_4 wave2AfterSync = 0.f;
					calcOsc2Wave(phase2Offset,
							calcTri2, tri2Amt,
							calcSawSq2, sawSq2Amt,
							calcSq2, sq2Amt,
							wave2AfterSync,
							c);

					// insert blep for sync
					osc2Blep[c/4].insertBlep(
							fractionalSyncTime,
							0.5 * (wave2AfterSync - wave2BeforeSync));

					// calc osc2 bleps from fractionalSyncTime to sample end
					calcOsc2Bleps(phase2Offset,
							phase2IncWithFm,
							calcTri2, tri2Amt,
							calcSawSq2, sawSq2Amt,
							calcSq2, sq2Amt,
							blep2Scale,
							c,
							fractionalSyncTime, 1.f);
				}
				else
				{
					// calc osc2 normally
					calcOsc2(phase2Offset,
							phase2IncWithFm,
							calcTri2, tri2Amt,
							calcSawSq2, sawSq2Amt,
							calcSq2, sq2Amt,
							blep2Scale,
							wave2,
							c);
				}
			}
			else
			{
				// calc osc2 normally
				calcOsc2(phase2Offset,
						phase2IncWithFm,
						calcTri2, tri2Amt,
						calcSawSq2, sawSq2Amt,
						calcSq2, sq2Amt,
						blep2Scale,
						wave2,
						c);
			}


			// apply bleps
			float_4 wave2forMix = prevWave2[c/4] + osc2Blep[c/4].process();

			// lowpass osc2
			if (oversamplingRate > 1)
			{
				// with oversampling > 1, osc1 sounds a bit duller than osc2 due to the blep being applied over 4*oversampling samples
				// -> lowpass osc2, will also help to reduce ringmod-aliasing and subsequent saturator aliasing a bit
				wave2forMix = lowpass[c/4].processLowpass(wave2forMix);
			}


			// mix
			out += osc1Vol[c/4] * prevWave1[c/4][bufferReadIndex] +
					osc2Vol[c/4] * wave2forMix +
					ringmodVol[c/4] * prevWave1[c/4][bufferReadIndex] * wave2forMix; // +-5V each

			// buffers
			prevSub1[c/4][bufferWriteIndex] = sub1;
			prevWave1[c/4][bufferWriteIndex] = wave1;
			prevWave2[c/4] = wave2;

			bufferReadIndex = (bufferReadIndex + 1) & (oversamplingRate - 1);
			bufferWriteIndex = (bufferReadIndex + oversamplingRate) & (oversamplingRate - 1);

			buffer[i] = out;
		}
	}

private:

	/**
	 * calc wave2 at beginning of sample,
	 * insert bleps and blamp
	 * advance phasor2 by phase2IncWithFm (fractional if minTime or maxTime are set)
	 */
	void calcOsc2(int32_4 phase2Offset,
			int32_4 phase2IncWithFm,
			int calcTri2, float_4 tri2Amt,
			int calcSawSq2, float_4 sawSq2Amt,
			int calcSq2, float_4 sq2Amt,
			float_4 blep2Scale,
			float_4& wave2,
			int c,
			float_4 minTime = 0.,
			float_4 maxTime = 1.)
	{
		int32_4 phasor2Offset = phasor2[c/4] + phase2Offset;

		if (calcTri2)
		{
			float_4 tri2 = -2 * simd::abs(phasor2Offset) + INT32_MAX; // +-INT32_MAX

			wave2 += tri2Amt * tri2; // +-INT32_MAX

			osc2Blep[c/4].insertBlamp(
					(1.f * INT32_MAX - 1.f * (phasor2Offset + phasor2Offset + INT32_MAX)) / (2.f * phase2IncWithFm),
					simd::sgn(float_4(phasor2Offset)) * tri2Amt * phase2IncWithFm,
					1, minTime, maxTime);
		}

		if (calcSawSq2)
		{
			if (calcSq2)
			{
				wave2 += sawSq2Amt * (sq2Amt * phasor2Offset - 1.f * phasor2[c/4]); // +-INT32_MAX

				osc2Blep[c/4].insertBlep(
						(INT32_MAX - phasor2Offset) / (1.f * phase2IncWithFm),
						blep2Scale * -sawSq2Amt * sq2Amt,
						1, minTime, maxTime);
			}
			else
			{
				wave2 += -sawSq2Amt * phasor2[c/4]; // +-INT32_MAX
			}

			osc2Blep[c/4].insertBlep(
					minTime + (INT32_MAX - phasor2[c/4]) / (1.f * phase2IncWithFm),
					blep2Scale * sawSq2Amt,
					1, minTime, maxTime);
		}

		phasor2[c/4] += (maxTime - minTime) * phase2IncWithFm;
	}

	/**
	 * calc wave2 at beginning of sample
	 */
	void calcOsc2Wave(int32_4 phase2Offset,
			int calcTri2, float_4 tri2Amt,
			int calcSawSq2, float_4 sawSq2Amt,
			int calcSq2, float_4 sq2Amt,
			float_4& wave2,
			int c)
	{
		int32_4 phasor2Offset = phasor2[c/4] + phase2Offset;

		if (calcTri2)
		{
			float_4 tri2 = -2 * simd::abs(phasor2Offset) + INT32_MAX; // +-INT32_MAX

			wave2 += tri2Amt * tri2; // +-INT32_MAX
		}

		if (calcSawSq2)
		{
			if (calcSq2)
			{
				wave2 += sawSq2Amt * (sq2Amt * phasor2Offset - 1.f * phasor2[c/4]); // +-INT32_MAX
			}
			else
			{
				wave2 += -sawSq2Amt * phasor2[c/4]; // +-INT32_MAX
			}
		}
	}

	/**
	 * insert bleps and blamp
	 * advance phasor2 by phase2IncWithFm (fractional if minTime or maxTime are set)
	 */
	void calcOsc2Bleps(int32_4 phase2Offset,
			int32_4 phase2IncWithFm,
			int calcTri2, float_4 tri2Amt,
			int calcSawSq2, float_4 sawSq2Amt,
			int calcSq2, float_4 sq2Amt,
			float_4 blep2Scale,
			int c,
			float_4 minTime = 0.,
			float_4 maxTime = 1.)
	{
		int32_4 phasor2Offset = phasor2[c/4] + phase2Offset;

		if (calcTri2)
		{
			osc2Blep[c/4].insertBlamp(
					(1.f * INT32_MAX - 1.f * (phasor2Offset + phasor2Offset + INT32_MAX)) / (2.f * phase2IncWithFm),
					simd::sgn(float_4(phasor2Offset)) * tri2Amt * phase2IncWithFm,
					1, minTime, maxTime);
		}

		if (calcSawSq2)
		{
			if (calcSq2)
			{
				osc2Blep[c/4].insertBlep(
						(INT32_MAX - phasor2Offset) / (1.f * phase2IncWithFm),
						blep2Scale * -sawSq2Amt * sq2Amt,
						1, minTime, maxTime);
			}
			osc2Blep[c/4].insertBlep(
					minTime + (INT32_MAX - phasor2[c/4]) / (1.f * phase2IncWithFm),
					blep2Scale * sawSq2Amt,
					1, minTime, maxTime);
		}

		phasor2[c/4] += (maxTime - minTime) * phase2IncWithFm;
	}

};

}
