#include <rack.hpp>

namespace musx {

using namespace rack;
using simd::float_4;

/**
 * 1 pole lowpass/highpass
 */
template <typename T = float>
struct TOnePole {
	T a = 0.f;
	T b = 0.f;
	T in;
	T tmp;

	TOnePole() {
		reset();
	}

	void reset() {
		in = {0.f};
		tmp = {0.f};
	}

	/** Sets the cutoff frequency.
	`f` is the ratio between the cutoff frequency and sample rate, i.e. f = f_c / f_s
	*/
	void setCutoffFreq(T f) {
		T x;
		//if (typeid(T) == typeid(float_4))
		//{
			f = simd::fmin(f, 0.3);
			x = simd::exp(-2.0f * M_PI * f);
		/*}
		else
		{
			f = std::fmin(f, 0.3);
			x = std::exp(-2.0f * M_PI * f);
		}*/
		a = 1.0f - x;
		b = -x;
	}

	void copyCutoffFreq(TOnePole<T> other) {
		a = other.a;
		b = other.b;
	}

	void process(T x) {
		in = x;
		tmp = a*x - b*tmp;
	}

	inline T processLowpass(T x) {
		tmp = a*x - b*tmp;
		return tmp;
	}

	T lowpass()
	{
		return tmp;
	}

	T highpass()
	{
		return in - tmp;
	}
};


/**
 * 4 pole lowpass
 */
template <typename T = float>
struct TFourPole {
	T a = 0.f;
	T b = 0.f;
	T q;
	T tmp4[4];

	TFourPole() {
		reset();
	}

	void reset() {
		for (int i=0; i<4; ++i)
		{
			tmp4[i] = {0.f};
		}
	}

	/** Sets the cutoff frequency.
	`f` is the ratio between the cutoff frequency and sample rate, i.e. f = f_c / f_s
	*/
	void setCutoffFreq(T f) {
		T x;
		//if (typeid(T) == typeid(float_4))
		//{
			f = simd::fmin(f, 0.3);
			x = simd::exp(-2.0f * M_PI * f);
		/*}
		else
		{
			f = std::fmin(f, 0.3);
			x = std::exp(-2.0f * M_PI * f);
		}*/
		a = 1.0f - x;
		b = -x;
	}

	void setResonance(T res)
	{
		q = res;
	}

	void process(T x) {
		// resonance
		x -= q * tmp4[3];
		x = simd::clamp(x, -5.f, 5.f);

		T out = a*x - b*tmp4[0];
		tmp4[0] = out;

		out = a*out - b*tmp4[1];
		tmp4[1] = out;

		out = a*out - b*tmp4[2];
		tmp4[2] = out;

		out = a*out - b*tmp4[3];
		tmp4[3] = out;
	}

	T lowpass()
	{
		return lowpass4();
	}

	T lowpass1() {
		return clamp(tmp4[0] * (1.f + q));
	}

	T lowpass2() {
		return clamp(tmp4[1] * (1.f + q));
	}

	T lowpass3() {
		return clamp(tmp4[2] * (1.f + q));
	}

	T lowpass4() {
		return clamp(tmp4[3] * (1.f + q));
	}

	T lowpassN(int order) {
		return clamp(tmp4[order] * (1.f + q));
	}

	T clamp(T in)
	{
		//if (typeid(T) == typeid(float_4))
		//{
			return simd::clamp(in, -5.f, 5.f);
		//}
		//return std::fmin(std::fmax(in, -5.f), 5.f);
	}
};


/**
 * State variable filter
 */
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
		//if (typeid(T) == typeid(float_4))
		//{
			f = simd::clamp(f, 0.001, 0.2f);
			c = 2.f * simd::sin(M_PI * f);
		/*}
		else
		{
			f = std::fmin(std::fmax(f, 0.001), 0.2f);
			c = 2.f * std::sin(M_PI * f);
		}*/
	}

	/**
	 * Set resonance between 0 and 1
	 */
	void setResonance(T r) {
		//if (typeid(T) == typeid(float_4))
		//{
			q = simd::clamp(1.f - r, 0.f, 1.f);
		/*}
		else
		{
			q = std::fmin(std::fmax(1.f - r, 0.f), 1.f);
		}*/
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

}
