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

inline float_4 tanh(float_4 x)
{
	// Pade approximant of tanh
	x = simd::clamp(x, -3.f, 3.f);
	return x * (27 + x * x) / (27 + 9 * x * x);
}

inline float_4 cheapSaturator(float_4 x)
{
	x = simd::clamp(x, -0.5773502692f, 0.5773502692f);
	return x - x*x*x;
}

}
