#pragma once

#include <rack.hpp>

namespace musx {

using namespace rack;
using simd::float_4;
using simd::int32_4;

/**
 * O max oversampling
 */
template <size_t O = 1, size_t L = 2, typename T = float_4>
class BlepGenerator {
private:
	T buffer[L * O] = {};
	size_t pos = 0;

public:
	BlepGenerator()
	{
		static_assert(O>0 && ((O & (O-1)) == 0), "O must be power of 2");
		static_assert(L == 2 || L == 4, "L must be 2 or 4");
	}

	/**
	 * Insert blep at fractional (non-oversampled) sample time t [0..1]
	 * scale > 0 for step from - to +
	 * scale < 0 for step from + to -
	 * scale = 1 for step from -1 to 1
	 * inserts L*oversampling samples
	 */
	void insertBlep(T mask, T time, T scale = 1, size_t oversampling = 1)
	{
		if (!simd::movemask(mask))
		{
			return;
		}

		T x = (-time - (L*oversampling/2 - 1)) / (L*oversampling/2);

		size_t index = pos;

		for (size_t i = 0; i < L * oversampling; i++)
		{
			buffer[index] -= mask & (simd::sgn(x) * scale * blepFunction(simd::fabs(x)));

			index = (index + 1) & (L * O - 1); // advance and wrap index
			x += 2. / (oversampling * L);
		}
	}

	/**
	 * Insert blamp at fractional (non-oversampled) sample time t [0..1]
	 * scale > 0 for ramp from - to +
	 * scale < 0 for ramp from + to -
	 * scale = 0.5 for ramp from gradient -1 to 1
	 * inserts L*oversampling samples
	 */
	void insertBlamp(T mask, T time, T scale = 1, size_t oversampling = 1)
	{
		if (!simd::movemask(mask))
		{
			return;
		}

		T x = (-time - (L*oversampling/2 - 1)) / (L*oversampling/2);

		size_t index = pos;

		for (size_t i = 0; i < L * oversampling; i++)
		{
			buffer[index] += mask & (oversampling * scale * blepFunction(simd::fabs(x)));

			index = (index + 1) & (L * O - 1); // advance and wrap index
			x += 2. / (oversampling * L);
		}
	}

	/**
	 * call once per (over)sample
	 */
	T process()
	{
		T value = buffer[pos];
		buffer[pos] = T(0); // reset buffer at i
		pos = (pos + 1) & (L * O - 1); // advance and wrap pos
		return value;
	}

	/**
	 * call once per (over)sample
	 */
	T process(size_t offset)
	{
		size_t posWithOffset = (pos + offset) & (L * O - 1);
		T value = buffer[posWithOffset];
		buffer[posWithOffset] = T(0); // reset buffer at i
		pos = (pos + 1) & (L * O - 1); // advance and wrap pos
		return value;
	}

private:

	// x [0..1], y [0..1]
	inline T blepFunction(T x)
	{
		if (L == 4)
		{
			// from https://www.kvraudio.com/forum/viewtopic.php?f=33&t=423884 / https://www.desmos.com/calculator/htfpu4fylh
			// must be applied over 4 samples!
			T x2 = x * x;
			T x4 = x2 * x2;
			return x4 * (x2 * (-4.36023 * x + 13.5038) - 11.5128 * x - 3.25094)
					+ x2 * (7.83529 * x - 0.2393)
					- 2.97566 * x + 0.99986;
		}
		if (L == 2)
		{
			// standard quadratic polyBlep
			return (x-1)*(x-1);
		}
	}
};


}
