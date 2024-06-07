#include "plugin.hpp"
#include "../dsp/functions.hpp"

/**
 * A lot of code is copied and adapted from Rack Fundamental (VCF.cpp) and Rack-SDK (ode.hpp)
 */

namespace musx {

using namespace rack;
using simd::float_4;


enum struct Method
{
	Euler,
	RK2,
	RK4
};

/**
 * https://urs.silvrback.com/one-pole-unlimited
 *
 * OTA: Iota = g * tanh(V+ - V-) with simple circuit schematic
 * Transistor ladder: Itl = g * ( tanh(V+) - tanh(V-) ) with rather complex schematic
 */
enum struct IntegratorType
{
	Linear,
	OTA,
	Transistor
};

enum struct NonlinearityType
{
	tanh,
	alt1,
	alt2,
	alt3,
	hardclip
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
	T maxAmplitude = 12.f;
	
	Method method = Method::RK4;
	IntegratorType integratorType = IntegratorType::Transistor;
	NonlinearityType nonlinearityType = NonlinearityType::tanh;

	virtual void f(T t, const T x[], T dxdt[]) const = 0;

	T clip(T x) const
	{
		switch (nonlinearityType)
		{
		default:
		case NonlinearityType::tanh:
			return maxAmplitude * musx::tanh(x/maxAmplitude);
		case NonlinearityType::alt1:
			return musx::cheapSaturator(x);
		case NonlinearityType::alt2:
			return musx::AntialiasedCheapSaturator<T>::processNonBandlimited(x);
		case NonlinearityType::alt3:
			return maxAmplitude * musx::waveshape(x/maxAmplitude);
		case NonlinearityType::hardclip:
			return clamp(x, -maxAmplitude, maxAmplitude);
		}
	}

	T getInputt(T t) const
	{
		return crossfade(this->lastInput, this->input, t / this->dt);
	}
	
	void calcLowpass(T in, size_t iStage, const T x[], T dxdt[]) const
	{
		switch (integratorType)
		{
		case IntegratorType::Linear:
			dxdt[iStage] = omega0 * (in - x[iStage]);
			break;
		case IntegratorType::OTA:
			dxdt[iStage] = omega0 * clip(in - x[iStage]);
			break;
		case IntegratorType::Transistor:
		default:
			dxdt[iStage] = omega0 * (clip(in) - clip(x[iStage]));
			break;
		}
	}

	void calcLowpassInverting(T in, size_t iStage, const T x[], T dxdt[]) const
	{
		switch (integratorType)
		{
		case IntegratorType::Linear:
			dxdt[iStage] = omega0 * (-(x[iStage] + in));
			break;
		case IntegratorType::OTA:
		case IntegratorType::Transistor:
		default:
			dxdt[iStage] = omega0 * -clip(x[iStage] + in);
			break;
		}
	}

	void calcHighpass(T in, size_t iStage, const T x[], T dxdt[]) const
	{
		switch (integratorType)
		{
		case IntegratorType::Linear:
			dxdt[iStage] = omega0 * (in - x[iStage]);
			break;
		case IntegratorType::OTA:
		case IntegratorType::Transistor:
		default:
			dxdt[iStage] = omega0 * (in - clip(x[iStage]));
			break;
		}
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

	// in Eurorack, voltage is limited to +-12V, so the capacitors cannot be charged indefinitely!
	// also prevents the filter from exploding
	void clampStates(T v)
	{
		for (size_t i = 0; i < S; i++) {
			state[i] = clamp(state[i], -v, v);
		}
	}

	void setMethod(Method m)
	{
		method = m;
	}

	void setIntegratorType(IntegratorType t)
	{
		integratorType = t;
	}

	IntegratorType getIntegratorType()
	{
		return integratorType;
	}

	void setNonlinearityType(NonlinearityType t)
	{
		nonlinearityType = t;
	}

	NonlinearityType getNonlinearityType()
	{
		return nonlinearityType;
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

	void process(T input, T dt)
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
class Filter1Pole : public FilterAbstract<T, 1>
{
protected:
	void f(T t, const T x[], T dxdt[]) const override
	{
		T input = this->getInputt(t);
		input = clamp(input, -this->maxAmplitude, this->maxAmplitude);

		this->calcLowpass(input, 0, x, dxdt);
	}

public:
	T lowpass()
	{
		return this->state[0];
	}

	T highpass()
	{
		return clamp(this->input, -this->maxAmplitude, this->maxAmplitude) - this->state[0];
	}
};

template <typename T>
class LadderFilter2Pole : public FilterAbstract<T, 2>
{
protected:
	void f(T t, const T x[], T dxdt[]) const override
	{
		T input = this->getInputt(t) - T(4.) * this->resonance * x[1]; // negative feedback
		input = clamp(input, -this->maxAmplitude, this->maxAmplitude);

		this->calcLowpass(input, 0, x, dxdt);
		this->calcLowpass(x[0], 1, x, dxdt);
	}

public:
	T lowpass()
	{
		return this->state[1];
	}

	T bandpass()
	{
		return this->state[0] - this->state[1]; // hp1
	}
};

template <typename T>
class LadderFilter4Pole : public FilterAbstract<T, 4>
{
protected:
	void f(T t, const T x[], T dxdt[]) const override
	{
		T input = this->getInputt(t) - T(2.) * this->resonance * x[3]; // negative feedback
		input = clamp(input, -this->maxAmplitude, this->maxAmplitude);

		this->calcLowpass(input, 0, x, dxdt);
		this->calcLowpass(x[0], 1, x, dxdt);
		this->calcLowpass(x[1], 2, x, dxdt);
		this->calcLowpass(x[2], 3, x, dxdt);
	}

public:
	T lowpass6()
	{
		return this->state[0];
	}
	T lowpass12()
	{
		return this->state[1];
	}
	T lowpass18()
	{
		return this->state[2];
	}
	T lowpass24()
	{
		return this->state[3];
	}
};

template <typename T>
class SallenKeyFilterLpBp : public FilterAbstract<T, 2>
{
protected:
	void f(T t, const T x[], T dxdt[]) const override
	{
		T hp1 = x[0] - x[1];
		T input = this->getInputt(t) + this->resonance * hp1; // positive feedback
		input *= T(0.8);
		input = clamp(input, -this->maxAmplitude, this->maxAmplitude);

		this->calcLowpass(input, 0, x, dxdt);
		this->calcHighpass(x[0], 1, x, dxdt);
	}

public:
	T lowpass()
	{
		return this->state[1];
	}

	T bandpass()
	{
		return this->state[0] - this->state[1];
	}
};

template <typename T>
class SallenKeyFilterHp : public FilterAbstract<T, 2>
{
protected:
	void f(T t, const T x[], T dxdt[]) const override
	{
		T input = this->getInputt(t) + T(0.8) * this->resonance * x[1]; // positive feedback
		input = clamp(input, -this->maxAmplitude, this->maxAmplitude);

		this->calcHighpass(input, 0, x, dxdt);
		this->calcLowpass(input - x[0], 1, x, dxdt);
	}

public:
	T highpass6()
	{
		return clamp(this->input, -this->maxAmplitude, this->maxAmplitude) - this->state[0];
	}

	T highpass12()
	{
		return clamp(this->input, -this->maxAmplitude, this->maxAmplitude) - this->state[0] - this->state[1];
	}
};

// TODO becomes unstable when res or cutoff too high
template <typename T>
class DiodeClipper : public FilterAbstract<T, 1>
{
protected:
	void f(T t, const T x[], T dxdt[]) const override
	{
		T input = this->getInputt(t);

		static const T a = {-1.e-6};
		T b = 10.f * this->resonance;

		dxdt[0] = this->omega0 * (input - x[0])
				+ a * (exp(0.5 * b * x[0]) - T(1.))
				- a * (exp(-b * x[0]) - T(1.));
	}

public:
	T out()
	{
		this->clampStates(this->maxAmplitude);
		return this->state[0];
	}
};

}
