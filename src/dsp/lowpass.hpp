#include <rack.hpp>

/**
 * 3rd order lowpass
 */
template <typename T = float>
struct TLowpass {
	T a = 0.f;
	T b = 0.f;
	T q = 0.f;
	T tmp[4];

	TLowpass() {
		reset();
	}

	void reset() {
		for (int i=0; i<4; ++i)
		{
			tmp[i] = {0.f};
		}
	}

	/** Sets the cutoff frequency.
	`f` is the ratio between the cutoff frequency and sample rate, i.e. f = f_c / f_s
	*/
	void setCutoffFreq(T f) {
		f = simd::fmin(f, 0.3);
		T x = simd::exp(-2.0f * M_PI * f);
		a = 1.0f - x;
		b = -x;
	}

	void setResonance(T res)
	{
		q = res;
	}

	void process(T x) {
		// resonance
		x -= q * tmp[3];
		x = simd::clamp(x, -5.f, 5.f);

		T out = a*x - b*tmp[0];
		tmp[0] = out;

		out = a*out - b*tmp[1];
		tmp[1] = out;

		out = a*out - b*tmp[2];
		tmp[2] = out;

		out = a*out - b*tmp[3];
		tmp[3] = out;
	}

	T lowpass() {
		return simd::clamp(tmp[2] * (1.f + q), -5.f, 5.f);
	}
};

typedef TLowpass<> Lowpass;
