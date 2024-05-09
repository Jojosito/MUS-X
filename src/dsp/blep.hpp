#include <rack.hpp>

namespace musx {

using namespace rack;
using simd::float_4;
using simd::int32_4;

/**
 * O max oversampling
 */
template <size_t O = 1, typename T = float_4>
struct BlepGenerator {
	T buffer[2*O] = {};
	size_t pos;

	BlepGenerator()
	{
		static_assert(O>0 && ((O & (O-1)) == 0), "O must be power of 2");
	}

	/**
	 * Insert blep at fractional (non-oversampled) sample time t [0..1]
	 * scale > 0 for step from - to +
	 * scale < 0 for step from + to -
	 * scale = 1 for step from -1 to 1
	 * inserts 2*oversampling samples
	 */
	void insertBlep(T t, T scale = 1, size_t oversampling = 1)
	{
		T mask = (t > 1. - 1. / oversampling) & (t < 1);

		if (!simd::movemask(mask))
		{
			return;
		}

		T x = -t; // [-1..0]

		size_t index = pos;

		for (size_t i = 0; i < 2 * oversampling; i++)
		{
			buffer[index] += simd::ifelse(mask & (x <  0), scale * (x + x + x*x + 1.), 0);
			buffer[index] += simd::ifelse(mask & (x >= 0), scale * (x + x - x*x - 1.), 0);

			index = (index + 1) & (2 * O - 1); // advance and wrap index
			x += 1. / oversampling;
		}
	}

	/**
	 * Insert blamp at fractional sample time t [0..L/2], with scale
	 */
	void insertBlamp(T t, T scale, size_t oversampling = 1)
	{
		// TODO
	}

	/**
	 * call once per sample
	 */
	T process()
	{
		T value = buffer[pos];
		buffer[pos] = T(0); // reset buffer at i
		pos = (pos + 1) & (2 * O - 1); // advance and wrap pos
		return value;
	}

};


}
