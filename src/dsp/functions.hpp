#pragma once

#include <rack.hpp>

namespace musx {

using namespace rack;
using simd::float_4;

inline float_4 waveshape(float_4 in)
{
	static const float a = {1.f/8.f};
	static const float b = {1.f/18.f};

	return in - a*in*in - b*in*in*in + a;
}

// y_max = +-1
inline float_4 tanh(float_4 x)
{
	// Pade approximant of tanh
	x = simd::clamp(x, -3.f, 3.f);
	return x * (27 + x * x) / (27 + 9 * x * x);
}

// y_max = +-10
inline float_4 cheapSaturator(float_4 x)
{
	x = simd::clamp(x, -15.f, 15.f);
	return x - 1.f/675.f * x*x*x;
}

/** Antiderivative-antialiased saturator */
// y_max = +-scale

class AntialiasedCheapSaturator {
private:
	static constexpr float epsilon = 1.e-3; // good balance between antialising and stability & noise
	static constexpr float scale = 13.1713; // deviation from 10*tanh(x/10) < 0.317 in [-23..23]
	float_4 x = 0; // previous input value

	// saturation function
	float_4 f(float_4 x)
	{
		return x * (simd::fabs(x) + 2.) / simd::pow(simd::fabs(x) + 1., 2);
	}

	// antiderivative of the saturation function
	float_4 F(float_4 x)
	{
		x = simd::fabs(x) + 1.;
		return x + 1./x;
	}

public:
	float_4 processBandlimited(float_4 in)
	{
		in /= 20.;
		float_4 bandlimited = (F(in) - F(x)) / (in - x);
		float_4 fallback = f(in);

		float_4 ret = scale * simd::ifelse(simd::fabs(in - x) < epsilon,
				fallback,
				bandlimited);

		x = in;

		return ret;
	}

	float_4 process(float_4 in)
	{
		in /= 20.;
		return scale * f(in);
	}
};

/** Limits the derivative of the output by a slew rate, in units/tick. */
template <typename T = float>
struct TSlewLimiter {
	T out = 0.f;
	T slew = 0.f;

	void reset() {
		out = 0.f;
	}

	void setMaxSlew(T slew) {
		this->slew = slew;
	}
	T process(T in) {
		out = simd::clamp(in, out - slew, out + slew);
		return out;
	}
};

}
