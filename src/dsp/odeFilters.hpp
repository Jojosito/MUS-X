#include "plugin.hpp"
#include "../dsp/functions.hpp"

/**
 * A lot of code is copied and adapted from Rack Fundamental (VCF.cpp) and Rack-SDK (ode.hpp)
 */

namespace musx {

using namespace rack;
using simd::float_4;

template <typename T>
static T clip(T x) {
	//return tanh(x);
	return musx::AntialiasedCheapSaturator<T>::processNonBandlimited(x);
}

/**
 * https://urs.silvrback.com/one-pole-unlimited
 *
 * OTA: Iota = g * tanh(V+ - V-) with simple circuit schematic
 * Transistor ladder: Itl = g * ( tanh(V+) - tanh(V-) ) with rather complex schematic
 */

enum struct Method
{
	Euler,
	RK2,
	RK4
};

/**
 * S is size of state vector
 */
template <typename T, size_t S>
class FilterAbstract {
protected:
	T omega0;
	T resonance = 0;
	T state[S];
	T lastInput;
	T input;
	T dt;

	virtual void f(T t, const T x[], T dxdt[]) const = 0;

	T getInputt(T t) const
	{
		return crossfade(this->lastInput, this->input, t / this->dt);
	}

	/** Solves an ODE system using the 1st order Euler method */
	void stepEuler(T t) {
		T k[S];

		f(t, state, k);

		for (size_t i = 0; i < S; i++) {
			state[i] += dt * k[i];
		}
	}

	/** Solves an ODE system using the 2nd order Runge-Kutta method */
	void stepRK2(T t) {
		T k1[S];
		T k2[S];
		T yi[S];

		f(t, state, k1);

		for (size_t i = 0; i < S; i++) {
			yi[i] = state[i] + k1[i] * dt / T(2);
		}
		f(t + dt / T(2), yi, k2);

		for (size_t i = 0; i < S; i++) {
			state[i] += dt * k2[i];
		}
	}

	/** Solves an ODE system using the 4th order Runge-Kutta method */
	void stepRK4(T t) {
		T k1[S];
		T k2[S];
		T k3[S];
		T k4[S];
		T yi[S];

		f(t, state, k1);

		for (size_t i = 0; i < S; i++) {
			yi[i] = state[i] + k1[i] * dt / T(2);
		}
		f(t + dt / T(2), yi, k2);

		for (size_t i = 0; i < S; i++) {
			yi[i] = state[i] + k2[i] * dt / T(2);
		}
		f(t + dt / T(2), yi, k3);

		for (size_t i = 0; i < S; i++) {
			yi[i] = state[i] + k3[i] * dt;
		}
		f(t + dt, yi, k4);

		for (size_t i = 0; i < S; i++) {
			state[i] += dt * (k1[i] + T(2) * k2[i] + T(2) * k3[i] + k4[i]) / T(6);
		}
	}

public:
	FilterAbstract() {
		reset();
		setCutoff(0);
	}

	void reset() {
		for (size_t i = 0; i < S; i++) {
			state[i] = 0;
		}
	}

	/**
	 * cutoff is Hz
	 */
	void setCutoff(T cutoff) {
		omega0 = 2 * T(M_PI) * cutoff;
	}

	void setCutoffFreq(T cutoff)
	{
		setCutoff(cutoff);
	}

	void setResonance(T res)
	{
		resonance = res;
	}

	void process(T input, T dt, Method method = Method::RK4)
	{
		this->input = input;
		this->dt = dt;
		switch (method)
		{
		case Method::Euler:
			stepEuler(T(0));
			break;
		case Method::RK2:
			stepRK2(T(0));
			break;
		case Method::RK4:
		default:
			stepRK4(T(0));
		}
		this->lastInput = input;
	}

};


template <typename T>
class LadderFilter : public FilterAbstract<T, 4>
{
public:
	void f(T t, const T x[], T dxdt[]) const override
	{
		T inputc = clip(this->getInputt(t) - this->resonance * x[3]);
		T yc0 = clip(x[0]);
		T yc1 = clip(x[1]);
		T yc2 = clip(x[2]);
		T yc3 = clip(x[3]);

		dxdt[0] = this->omega0 * (inputc - yc0);
		dxdt[1] = this->omega0 * (yc0 - yc1);
		dxdt[2] = this->omega0 * (yc1 - yc2);
		dxdt[3] = this->omega0 * (yc2 - yc3);
	}

	T lowpass()
	{
		return this->state[3];
	}
	T highpass() {
		// TODO will only work for RK4???
		return clip((this->input - this->resonance * this->state[3]) -
				4 * this->state[0] + 6 * this->state[1] - 4 * this->state[2] + this->state[3]);
	}
};

}
