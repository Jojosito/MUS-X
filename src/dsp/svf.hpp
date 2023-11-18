#include <rack.hpp>

namespace musx {

template <typename T = float>
struct TSVF {
	T c = {0.f};
	T q = {1.f};
	T scale = {1.f};

	T lp = {0};
	T hp = {0};
	T bp = {0};

	TSVF() {
		reset();
	}

	void reset() {
		lp = 0.f;
		hp = 0.f;
		bp = 0.f;
	}

	/** Sets the cutoff frequency.
	`f` is the ratio between the cutoff frequency and sample rate, i.e. f = f_c / f_s
	*/
	void setCutoffFreq(T f) {
		f = simd::clamp(f, 0.001, 0.2f);
		c = 2.f * simd::sin(M_PI * f);
	}

	/**
	 * Set resonance between 0 and 1
	 */
	void setResonance(T r) {
		q = simd::clamp(1.f - r, 0.f, 1.f);
		scale = q;
	}

	void process(T x) {
		lp = lp + c * bp;
		hp = scale * x - lp - q * bp;
		bp = c * hp + bp;
		//notch = hp + lp;
	}
	T lowpass() {
		return lp;
	}
	T highpass() {
		return hp;
	}
	T bandpass() {
		return bp;
	}
};

typedef TSVF<> SVF;

}
