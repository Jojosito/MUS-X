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
	int32_4 phasor1Sub = {0};
	int32_4 phasor1Old = {0};
	int32_4 phasor2 = {0};

	// sample rates
	int sampleRate = 48000;
	int oversamplingRate = 1;
	float oneOverSampleRateTimesOversamplingRate;

	// freq limits
	float minFreq = 0.0001f; // [Hz]
	float maxFreq = 20000.f; // [Hz]

	// parameters
	float_4 osc1Freq = {440.}; // [Hz]
	float_4 osc1Shape = {0};
	float_4 osc1PW = {0};
	float_4 osc1Vol = {0};
	float_4 osc1Pan = {0};
	float_4 osc1Subvol = {0};
	float_4 osc1SubPan = {0};

	float_4 osc2Freq = {440.}; // [Hz]
	float_4 osc2Shape = {0};
	float_4 osc2PW = {0};
	float_4 osc2Vol = {0};
	float_4 osc2Pan = {0};

	float_4 syncMask = {0};
	int calcFm = 0;
	float_4 fmUnscaled = {0};
	float_4 fmAmt = {0};
	float_4 ringmodVol = {0};
	float_4 ringmodPan = {0};

	// more parameters
	int32_4 phase1SubInc = {0};
	int32_4 phase1Inc = {0};
	float_4 tri1Amt = {0};
	float_4 sawSq1Amt  = {0};
	float_4 sq1Amt = {0};
	int32_4 phase1Offset = {0};

	int calcTri1 = 0;
	int calcSawSq1 = 0;
	int calcSq1 = 0;
	int calcSub = 0;

	int32_4 phase2Inc = {0};
	float_4 tri2Amt = {0};
	float_4 sawSq2Amt = {0};
	float_4 sq2Amt = {0};
	int32_4 phase2Offset = {0};

	int calcSync = 0;
	int calcTri2 = 0;
	int calcSawSq2 = 0;
	int calcSq2 = 0;

	// blep generators
	static constexpr size_t blepSize = 4;
	BlepGenerator<O, blepSize, float_4> osc1Blep;
	BlepGenerator<O, blepSize, float_4> oscSubBlep;
	BlepGenerator<O, blepSize, float_4> osc2Blep;

	// buffer for applying 4-point blep
	size_t bufferReadIndex = 0;
	size_t bufferWriteIndex = oversamplingRate * blepSize / 2 - 1;
	float_4 prevSub1 [O * blepSize / 2] = {0};
	float_4 prevWave1[O * blepSize / 2] = {0};
	float_4 prevWave2[O * blepSize / 2] = {0};

	void setPhase1Inc()
	{
		phase1SubInc = INT32_MAX * osc1Freq * oneOverSampleRateTimesOversamplingRate;
		phase1Inc 	 = phase1SubInc + phase1SubInc;
	}

	void setPhase2Inc()
	{
		phase2Inc = INT32_MAX * osc2Freq * 2 * oneOverSampleRateTimesOversamplingRate;
	}


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

		setFmAmount(fmUnscaled);
		setPhase1Inc();
		setPhase2Inc();
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
		phasor1Sub = 0.;
		phasor2 = 0.;
	}


	// set oscillator 1 frequency in V/Oct. 0 V = C4 [V]
	inline void setOsc1FreqVOct(float_4 freq)
	{
		osc1Freq = simd::clamp(dsp::FREQ_C4 * dsp::exp2_taylor5(freq), minFreq, maxFreq);
		if (calcFm) setFmAmount(fmUnscaled);
		setPhase1Inc();
	}

	// set oscillator 1 frequency in V/Oct. 0 V = 2 Hz [V]
	inline void setOsc1FreqVOctLFO(float_4 freq)
	{
		osc1Freq = simd::clamp(2. * dsp::exp2_taylor5(freq), minFreq, maxFreq);
		if (calcFm) setFmAmount(fmUnscaled);
		setPhase1Inc();
	}

	// set oscillator 1 frequency [Hz]
	inline void setOsc1FreqHz(float_4 freq)
	{
		osc1Freq = simd::clamp(freq, minFreq, maxFreq);
		if (calcFm) setFmAmount(fmUnscaled);
		setPhase1Inc();
	}

	// set oscillator 1 shape [-1..1]
	inline void setOsc1Shape(float_4 shape)
	{
		osc1Shape = simd::clamp(shape, -1.f, 1.f);

		tri1Amt   = simd::fmax(-osc1Shape, 0.f);  		// [1, 0, 0]
		sawSq1Amt = simd::fmin(1.f + osc1Shape, 1.f); 	// [0, 1, 1]
		sq1Amt 	  = simd::fmax(osc1Shape, 0.f);          // [0, 0, 1]

		calcTri1 = simd::movemask(tri1Amt > 0);
		calcSawSq1 = simd::movemask(sawSq1Amt > 0);
		calcSq1 = simd::movemask(sq1Amt > 0);
	}

	// set oscillator 1 pulsewidth [-1..1]
	inline void setOsc1PW(float_4 pw)
	{
		osc1PW = simd::clamp(pw, -1.f, 1.f);
		phase1Offset = simd::ifelse(osc1PW < 0, (-1.f - osc1PW) * INT32_MAX, (1.f - osc1PW) * INT32_MAX); // for pulse wave = saw + inverted saw with phaseshift
	}

	// set oscillator 1 volume [0..1]
	inline void setOsc1Vol(float_4 vol)
	{
		osc1Vol  = simd::clamp(vol, 0.f, 1.f);
		osc1Vol *= 10.f / INT32_MAX;
	}

	// set oscillator 1 pan [-1..1]
	inline void setOsc1Pan(float_4 pan)
	{
		osc1Pan = simd::clamp(pan, -1.f, 1.f);
	}

	// set oscillator 1 suboscillator volume [0..1]
	inline void setOsc1Subvol(float_4 vol)
	{
		osc1Subvol  = simd::clamp(vol, 0.f, 1.f);
		osc1Subvol *= 10.f / INT32_MAX;

		calcSub = simd::movemask(osc1Subvol > 0);
	}

	// set oscillator 1 suboscillator pan [-1..1]
	inline void setOsc1SubPan(float_4 pan)
	{
		osc1SubPan = simd::clamp(pan, -1.f, 1.f);
	}

	// set oscillator 2 frequency in V/Oct. 0 V = C4 [V]
	inline void setOsc2FreqVOct(float_4 freq)
	{
		osc2Freq = simd::clamp(dsp::FREQ_C4 * dsp::exp2_taylor5(freq), minFreq, maxFreq);
		if (calcFm) setFmAmount(fmUnscaled);
		setPhase2Inc();
	}

	// set oscillator 2 frequency in V/Oct. 0 V = 2 Hz [V]
	inline void setOsc2FreqVOctLFO(float_4 freq)
	{
		osc2Freq = simd::clamp(2. * dsp::exp2_taylor5(freq), minFreq, maxFreq);
		if (calcFm) setFmAmount(fmUnscaled);
		setPhase2Inc();
	}

	// set oscillator 2 frequency [Hz]
	inline void setOsc2Freq(float_4 freq)
	{
		osc2Freq = simd::clamp(freq, minFreq, maxFreq);
		if (calcFm) setFmAmount(fmUnscaled);
		setPhase2Inc();
	}

	// set oscillator 2 shape [-1..1]
	inline void setOsc2Shape(float_4 shape)
	{
		osc2Shape = simd::clamp(shape, -1.f, 1.f);

		tri2Amt   = simd::fmax(-osc2Shape, 0.f);
		sawSq2Amt = simd::fmin(1.f + osc2Shape, 1.f);
		sq2Amt 	  = simd::fmax(osc2Shape, 0.f);

		calcTri2   = simd::movemask(tri2Amt > 0);
		calcSawSq2 = simd::movemask(sawSq2Amt > 0);
		calcSq2    = simd::movemask(sq2Amt > 0);
	}

	// set oscillator 2 pulsewidth [-1..1]
	inline void setOsc2PW(float_4 pw)
	{
		osc2PW = simd::clamp(pw, -1.f, 1.f);
		phase2Offset = simd::ifelse(osc2PW < 0, (-1.f - osc2PW) * INT32_MAX, (1.f - osc2PW) * INT32_MAX); // for pulse wave
	}

	// set oscillator 2 volume [0..1]
	inline void setOsc2Vol(float_4 vol)
	{
		osc2Vol  = simd::clamp(vol, 0.f, 1.f);
		osc2Vol *= 10.f / INT32_MAX;
	}

	// set oscillator 2 pan [-1..1]
	inline void setOsc2Pan(float_4 pan)
	{
		osc2Pan = simd::clamp(pan, -1.f, 1.f);
	}


	// set if oscillator 2 should be synced to oscillator 1 [0, 1]
	inline void setSync(float_4 s)
	{
		syncMask = s > 0.5f; // 0x00000000 or 0xffffffff
		calcSync = simd::movemask(syncMask);
	}

	// set oscillator 1 -> oscillator 2 frequency-modulation amount [0..1]
	inline void setFmAmount(float_4 fm)
	{
		calcFm = simd::movemask(fm > 1.e-6f);

		fmUnscaled = simd::clamp(fm, 0.f, 1.f);

		// use adapted carsons rule to limit FM amount
		// value 7000 is chosen so that
		// osc 2 fm'ed fundamental frequency stays below nyquist (oversamplingRate*sampleRate/2.)
		float_4 maxFm = simd::fmax(
				((oversamplingRate*sampleRate/2. - osc2Freq) / 2. - osc1Freq) / 7000.,
				0.f);
		fmAmt = simd::fmin(fmUnscaled * fmUnscaled, maxFm);

		fmAmt = fmAmt * 0.5f / oversamplingRate * 48000 / sampleRate; // scale
	}

	// set ringmodulator volume [0..1]
	inline void setRingmodVol(float_4 vol)
	{
		ringmodVol  = simd::clamp(vol, 0.f, 1.f);
		ringmodVol *= 10.f / INT32_MAX / INT32_MAX;
	}

	// set ringmodulator pan [-1..1]
	inline void setRingmodPan(float_4 pan)
	{
		ringmodPan = simd::clamp(pan, -1.f, 1.f);
	}


	// calculate output, fill buffer with #oversamplingRate samples
	// output can have DC offset when using fm or ringmod
	// output in NOT bound to +-10V. The individual components (osc1, subosc, osc2, ringmod) are within +-10V
	// it is recommended to feed the output through a DC blocker and saturator
	void process(float_4* buffer)
	{
		// calculate the oversampled oscillators and mix
		for (int i = 0; i < oversamplingRate; ++i)
		{
			// phasors for subosc 1 and osc 1
			phasor1Sub += phase1SubInc;
			int32_4 phasor1 = phasor1Sub + phasor1Sub;
			int32_4 phasor1Offset = phasor1 + phase1Offset;

			// osc 1 waveform
			float_4 wave1 = -2.f * tri1Amt * (simd::abs(phasor1Offset) - INT32_MAX/2); // +-INT32_MAX
			wave1 += sawSq1Amt * (phasor1Offset * sq1Amt - 1.f * phasor1); // +-INT32_MAX

			// osc 1 suboscillator
			float_4 sub1 = 1.f * (phasor1Sub + INT32_MAX) - 1.f * phasor1Sub; // +-INT32_MAX

			// phasor for osc 2
			phasor2 += phase2Inc + int32_4(fmAmt * wave1);

			// syncMask / reset phasor2 ?
			int32_4 resetPhaseMask = phasor1Old > phasor1;
			phasor2 = simd::ifelse(syncMask & musx::castIntMaskToFloat(resetPhaseMask), INT32_MIN, phasor2);
			phasor1Old = phasor1;
			int32_4 phasor2Offset = phasor2 + phase2Offset;

			// osc 2 waveform
			float_4 wave2 = -2.f * tri2Amt * (simd::abs(phasor2Offset) - INT32_MAX/2); // +-INT32_MAX
			wave2 += sawSq2Amt * (phasor2Offset * sq2Amt - 1.f * phasor2); // +-INT32_MAX

			// mix
			float_4 out = osc1Subvol * sub1 + osc1Vol * wave1 + osc2Vol * wave2 + ringmodVol * wave1 * wave2; // +-5V each

			buffer[i] = out;
		}
	}

	// calculate bandlimited output, fill buffer with #oversamplingRate samples
	// output can have DC offset when using fm or ringmod
	// output in NOT bound to +-10V. The individual components (osc1, subosc, osc2, ringmod) are within +-10V
	// it is recommended to feed the output through a DC blocker and saturator
	void processBandlimited(float_4* bufferLMono, float_4* bufferR = nullptr)
	{
		// calculate the oversampled oscillators and mix
		for (int i = 0; i < oversamplingRate; ++i)
		{
			float_4 sub1 = 0;
			float_4 wave1 = 0;
			float_4 wave2 = 0;
			float_4 outLMono = 0;
			float_4 outR = 0;


			//
			// osc 1 ============================================================================================================
			//

			// phasors for osc 1
			int32_4 phasor1 = phasor1Sub + phasor1Sub;
			int32_4 phasor1Offset = phasor1 + phase1Offset;

			if (calcTri1)
			{
				float_4 tri1 = -2 * simd::abs(phasor1Offset) + INT32_MAX; // +-INT32_MAX

				wave1 += tri1Amt * tri1; // +-INT32_MAX

				int32_4 effPhasor = phasor1Offset + phasor1Offset + INT32_MAX;
				osc1Blep.insertBlamp(
						(1.f * effPhasor + 2.f * phase1Inc > 1.f * INT32_MAX),
						(1.f * INT32_MAX - 1.f * effPhasor) / (2.f * phase1Inc),
						simd::sgn(float_4(phasor1Offset)) * tri1Amt * phase1Inc,
						oversamplingRate);
			}

			if (calcSawSq1)
			{
				if (calcSq1)
				{
					wave1 += sawSq1Amt * (sq1Amt * phasor1Offset - 1.f * phasor1); // +-INT32_MAX

					osc1Blep.insertBlep(
							getBlepMask(phasor1Offset, phase1Inc),
							(INT32_MAX - phasor1Offset) / (1.f * phase1Inc),
							-sawSq1Amt * sq1Amt * INT32_MAX,
							oversamplingRate);
				}
				else
				{
					wave1 += -sawSq1Amt * phasor1; // +-INT32_MAX
				}

				osc1Blep.insertBlep(
						getBlepMask(phasor1, phase1Inc),
						(INT32_MAX - phasor1) / (1.f * phase1Inc),
						sawSq1Amt * INT32_MAX,
						oversamplingRate);
			}

			if (calcSub)
			{
				int32_4 phasor1SubOffset = phasor1Sub + INT32_MAX;

				sub1 = 1.f * phasor1SubOffset - 1.f * phasor1Sub; // +-INT32_MAX

				oscSubBlep.insertBlep(
						getBlepMask(phasor1Sub, phase1SubInc),
						simd::clamp((INT32_MAX - phasor1Sub) / (1.f * phase1SubInc), 0.f , 1.f),
						INT32_MAX,
						oversamplingRate);

				oscSubBlep.insertBlep(
						getBlepMask(phasor1SubOffset, phase1SubInc),
						simd::clamp((INT32_MAX - phasor1SubOffset) / (1.f * phase1SubInc), 0.f , 1.f),
						-INT32_MAX,
						oversamplingRate);

				if (bufferR)
				{
					outLMono += osc1Subvol * getLVol(osc1SubPan) * (prevSub1[bufferReadIndex] + oscSubBlep.process());
					outR += osc1Subvol * getRVol(osc1SubPan) * (prevSub1[bufferReadIndex] + oscSubBlep.process());
				}
				else
				{
					outLMono += osc1Subvol * (prevSub1[bufferReadIndex] + oscSubBlep.process());
				}
			}

			phasor1Sub += phase1SubInc;

			// apply bleps
			prevWave1[bufferReadIndex] += osc1Blep.process();


			//
			// osc 2 ============================================================================================================
			//

			// phasors for osc 2
			int32_4 phase2IncWithFm = phase2Inc + int32_4(fmAmt * prevWave1[bufferReadIndex]); // can be negative!

			float_4 blep2Scale = simd::sgn(float_4(phase2IncWithFm)) * INT32_MAX; // [-INT32_MAX, INT32_MAX]
			if (calcSync)
			{
				float_4 syncBlepMask = getBlepMask(phasor1, phase1Inc);
				float_4 doSyncMask = syncMask & syncBlepMask;

				if (simd::movemask(doSyncMask))
				{
					float_4 fractionalSyncTime = (INT32_MAX - phasor1) / (1.f * phase1Inc); // [0..1]
					fractionalSyncTime = simd::clamp(fractionalSyncTime, 0.f, 1.0f);
					fractionalSyncTime = simd::ifelse(doSyncMask, fractionalSyncTime, 1.f); // get rid of some numerical errors

					int32_4 phase2IncWithFmBeforeSync = phase2IncWithFm;
					phase2IncWithFmBeforeSync -= castFloatMaskToInt(doSyncMask) & int32_4((1.f - fractionalSyncTime) * phase2IncWithFm);
					int32_4 phase2IncWithFmAfterSync = phase2IncWithFm - phase2IncWithFmBeforeSync;

					// calc osc2 and bleps from sample begin to fractionalSyncTime
					calcOsc2(phase2IncWithFmBeforeSync,
							blep2Scale,
							wave2,
							0.f,
							fractionalSyncTime);

					// calc osc2 wave right before sync for blep scale
					float_4 wave2BeforeSync = 0.f;
					calcOsc2Wave(wave2BeforeSync);

					// syncMask? -> reset phasor2
					phasor2 += castFloatMaskToInt(doSyncMask) & -phasor2 + INT32_MIN; // reset to INT32_MIN
					int32_4 scaleMask = phase2IncWithFm < 0;
					phasor2 += scaleMask & -1; // roll over to INT32_MAX if phase2IncWithFm < 0

					// calc osc2 wave right after sync for blep scale
					float_4 wave2AfterSync = 0.f;
					calcOsc2Wave(wave2AfterSync);

					// insert blep for sync
					osc2Blep.insertBlep(
							syncBlepMask,
							fractionalSyncTime,
							0.5 * (wave2AfterSync - wave2BeforeSync),
							oversamplingRate);

					// calc osc2 bleps from fractionalSyncTime to sample end
					calcOsc2Bleps(phase2IncWithFmAfterSync,
							blep2Scale,
							fractionalSyncTime);
				}
				else
				{
					// calc osc2 normally
					calcOsc2(phase2IncWithFm,
							blep2Scale,
							wave2);
				}
			}
			else
			{
				// calc osc2 normally
				calcOsc2(phase2IncWithFm,
						blep2Scale,
						wave2);
			}

			// apply bleps
			prevWave2[bufferReadIndex] += osc2Blep.process();


			// mix
			if (bufferR)
			{
				outLMono += osc1Vol * getLVol(osc1Pan) * prevWave1[bufferReadIndex] +
						osc2Vol * getLVol(osc2Pan) * prevWave2[bufferReadIndex] +
						ringmodVol * getLVol(ringmodPan) * prevWave1[bufferReadIndex] * prevWave2[bufferReadIndex]; // +-5V each

				bufferLMono[i] = outLMono;

				outR += osc1Vol * getRVol(osc1Pan) * prevWave1[bufferReadIndex] +
						osc2Vol * getRVol(osc2Pan) * prevWave2[bufferReadIndex] +
						ringmodVol * getRVol(ringmodPan) * prevWave1[bufferReadIndex] * prevWave2[bufferReadIndex]; // +-5V each

				bufferR[i] = outR;
			}
			else
			{
				outLMono += osc1Vol * prevWave1[bufferReadIndex] +
						osc2Vol * prevWave2[bufferReadIndex] +
						ringmodVol * prevWave1[bufferReadIndex] * prevWave2[bufferReadIndex]; // +-5V each

				bufferLMono[i] = outLMono;
			}

			// buffers
			prevSub1 [bufferWriteIndex] = sub1;
			prevWave1[bufferWriteIndex] = wave1;
			prevWave2[bufferWriteIndex] = wave2;

			bufferReadIndex = (bufferReadIndex + 1) & (O * blepSize / 2 - 1);
			bufferWriteIndex = (bufferReadIndex + oversamplingRate * blepSize / 2 - 1) & (O * blepSize / 2 - 1);

		}
	}

private:

	/**
	 * calc wave2 at beginning of sample,
	 * insert bleps and blamp
	 * advance phasor2 by phase2IncWithFm
	 */
	void calcOsc2(int32_4 phase2IncWithFm,
			float_4 blep2Scale,
			float_4& wave2,
			float_4 minTime = 0.f,
			float_4 maxTime = 1.f)
	{
		int32_4 phasor2Offset = phasor2 + phase2Offset;

		if (calcTri2)
		{
			float_4 tri2 = -2 * simd::abs(phasor2Offset) + INT32_MAX; // +-INT32_MAX

			wave2 += tri2Amt * tri2; // +-INT32_MAX

			int32_4 effPhasor = phasor2Offset + phasor2Offset + INT32_MAX;
			osc2Blep.insertBlamp(
					(1.f * effPhasor + 2.f * phase2IncWithFm > 1.f * INT32_MAX),
					maxTime * (minTime + (1.f * INT32_MAX - 1.f * effPhasor) / (2.f * phase2IncWithFm)),
					simd::sgn(float_4(phasor2Offset)) * tri2Amt * phase2IncWithFm,
					oversamplingRate);
		}

		if (calcSawSq2)
		{
			if (calcSq2)
			{
				wave2 += sawSq2Amt * (sq2Amt * phasor2Offset - 1.f * phasor2); // +-INT32_MAX

				osc2Blep.insertBlep(
						getBlepMaskSigned(phasor2Offset, phase2IncWithFm),
						maxTime * (minTime + (INT32_MAX - phasor2Offset) / (1.f * phase2IncWithFm)),
						blep2Scale * -sawSq2Amt * sq2Amt,
						oversamplingRate);
			}
			else
			{
				wave2 += -sawSq2Amt * phasor2; // +-INT32_MAX
			}

			osc2Blep.insertBlep(
					getBlepMaskSigned(phasor2, phase2IncWithFm),
					maxTime * (minTime + (INT32_MAX - phasor2) / (1.f * phase2IncWithFm)),
					blep2Scale * sawSq2Amt,
					oversamplingRate);
		}

		phasor2 += phase2IncWithFm;
	}

	/**
	 * calc wave2 at beginning of sample
	 */
	void calcOsc2Wave(float_4& wave2)
	{
		int32_4 phasor2Offset = phasor2 + phase2Offset;

		if (calcTri2)
		{
			float_4 tri2 = -2 * simd::abs(phasor2Offset) + INT32_MAX; // +-INT32_MAX

			wave2 += tri2Amt * tri2; // +-INT32_MAX
		}

		if (calcSawSq2)
		{
			if (calcSq2)
			{
				wave2 += sawSq2Amt * (sq2Amt * phasor2Offset - 1.f * phasor2); // +-INT32_MAX
			}
			else
			{
				wave2 += -sawSq2Amt * phasor2; // +-INT32_MAX
			}
		}
	}

	/**
	 * insert bleps and blamp
	 * advance phasor2 by ph
	 */
	void calcOsc2Bleps(int32_4 phase2IncWithFm,
			float_4 blep2Scale,
			float_4 timeOffset = 0.)
	{
		int32_4 phasor2Offset = phasor2 + phase2Offset;

		if (calcTri2)
		{
			int32_4 effPhasor = phasor2Offset + phasor2Offset + INT32_MAX;
			osc2Blep.insertBlamp(
					(1.f * effPhasor + 2.f * phase2IncWithFm > 1.f * INT32_MAX),
					(INT32_MAX - effPhasor) / (2.f * phase2IncWithFm),
					simd::sgn(float_4(phasor2Offset)) * tri2Amt * phase2IncWithFm,
					oversamplingRate);
		}

		if (calcSawSq2)
		{
			if (calcSq2)
			{
				osc2Blep.insertBlep(
						getBlepMaskSigned(phasor2Offset, phase2IncWithFm),
						timeOffset + (INT32_MAX - phasor2Offset) / (1.f * phase2IncWithFm),
						blep2Scale * -sawSq2Amt * sq2Amt,
						oversamplingRate);
			}
			osc2Blep.insertBlep(
					getBlepMaskSigned(phasor2, phase2IncWithFm),
					timeOffset + (INT32_MAX - phasor2) / (1.f * phase2IncWithFm),
					blep2Scale * sawSq2Amt,
					oversamplingRate);
		}

		phasor2 += phase2IncWithFm;
	}

	// assumes phaseInc > 0
	float_4 getBlepMask(int32_4 phasor, int32_4 phaseInc)
	{
		int32_4 phasorResetMaskInt = (phasor + phaseInc) < phasor;
		return castIntMaskToFloat(phasorResetMaskInt);
	}

	float_4 getBlepMaskSigned(int32_4 phasor, int32_4 phaseInc)
	{
		int32_4 phasorResetMaskIntPos = (phasor + phaseInc) < phasor;
		float_4 phasorResetMaskFloatPos = castIntMaskToFloat(phasorResetMaskIntPos);

		int32_4 phasorResetMaskIntNeg = (phasor + phaseInc) > phasor;
		float_4 phasorResetMaskFloatNeg = castIntMaskToFloat(phasorResetMaskIntNeg);

		return simd::ifelse(1.f * phaseInc > 0.f, phasorResetMaskFloatPos, phasorResetMaskFloatNeg);
	}

	float_4 getLVol(float_4 pan)
	{
		return 0.5f - 0.5f * pan;
	}

	float_4 getRVol(float_4 pan)
	{
		return 0.5f + 0.5f * pan;
	}
};

}
