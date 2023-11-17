#include <rack.hpp>

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
		f = std::min(f, 0.125f);
		c = 2.f * std::sin(M_PI * f);
	}

	/**
	 * Set resonance between 0 and 1
	 */
	void setResonance(T r) {
		r = std::min(r, 1.f);
		r = std::max(r, 0.f);
		q = 1.f - r;
		//scale = q;
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
