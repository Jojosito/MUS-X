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
	OTA_tanh,
	OTA_alt,
	Transistor_tanh,
	Transistor_alt,
};

/**
 * S is size of state vector
 */
template <typename T, size_t S, IntegratorType integratorType = IntegratorType::Transistor_tanh>
class FilterAbstract {
protected:
	T omega0;
	T resonance = 0;
	T state[S];
	T lastInput;
	T input;
	T dt;
	const T maxAmplitude = 12.f;
	
	Method method = Method::RK4;

	virtual void f(T t, const T x[], T dxdt[]) const = 0;

	T clip(T x) const
	{
		return maxAmplitude * musx::tanh(x/maxAmplitude);
	}

	T clipAlt(T x) const
	{
		return musx::AntialiasedCheapSaturator<T>::processNonBandlimited(x);
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
		case IntegratorType::OTA_tanh:
			dxdt[iStage] = omega0 * clip(in - x[iStage]);
			break;
		case IntegratorType::OTA_alt:
			dxdt[iStage] = omega0 * clipAlt(in - x[iStage]);
			break;
		case IntegratorType::Transistor_tanh:
		default:
			dxdt[iStage] = omega0 * (clip(in) - clip(x[iStage]));
			break;
		case IntegratorType::Transistor_alt:
			dxdt[iStage] = omega0 * (clipAlt(in) - clipAlt(x[iStage]));
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
		case IntegratorType::OTA_tanh:
		case IntegratorType::Transistor_tanh:
		default:
			dxdt[iStage] = omega0 * -clip(x[iStage] + in);
			break;
		case IntegratorType::OTA_alt:
		case IntegratorType::Transistor_alt:
			dxdt[iStage] = omega0 * -clipAlt(x[iStage] + in);
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
		case IntegratorType::OTA_tanh:
		case IntegratorType::Transistor_tanh:
		default:
			dxdt[iStage] = omega0 * (in - clip(x[iStage]));
			break;
		case IntegratorType::OTA_alt:
		case IntegratorType::Transistor_alt:
			dxdt[iStage] = omega0 * (in - clipAlt(x[iStage]));
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
		resonance = fmax(0.f, res);
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

	void processEuler(T input, T dt)
	{
		this->input = input;
		this->dt = dt;
		stepEuler(T(0));
		this->lastInput = input;
	}

	void processRK2(T input, T dt)
	{
		this->input = input;
		this->dt = dt;
		stepRK2(T(0));
		this->lastInput = input;
	}

	void processRK4(T input, T dt)
	{
		this->input = input;
		this->dt = dt;
		stepRK4(T(0));
		this->lastInput = input;
	}

};


template <typename T, IntegratorType integratorType = IntegratorType::Transistor_tanh>
class Filter1Pole : public FilterAbstract<T, 1, integratorType>
{
protected:
	void f(T t, const T x[], T dxdt[]) const override
	{
		T input = this->getInputt(t);
		input = clamp(input, -this->maxAmplitude, this->maxAmplitude);

		this->calcLowpass(input, 0, x, dxdt);
	}

public:
	T lowpass() const
	{
		return this->state[0];
	}

	T highpass() const
	{
		return clamp(this->input, -this->maxAmplitude, this->maxAmplitude) - this->state[0];
	}
};

template <typename T, IntegratorType integratorType = IntegratorType::Transistor_tanh>
class LadderFilter2Pole : public FilterAbstract<T, 2, integratorType>
{
protected:
	void f(T t, const T x[], T dxdt[]) const override
	{
		T input = this->getInputt(t) - this->resonance * x[1]; // negative feedback
		input = clamp(input, -this->maxAmplitude, this->maxAmplitude);

		this->calcLowpass(input, 0, x, dxdt);
		this->calcLowpass(x[0], 1, x, dxdt);
	}

public:
	T lowpass() const
	{
		return this->state[1];
	}

	T bandpass() const
	{
		return this->state[0] - this->state[1]; // hp1
	}
};

template <typename T, IntegratorType integratorType = IntegratorType::Transistor_tanh>
class LadderFilter4Pole : public FilterAbstract<T, 4, integratorType>
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
	T lowpass6() const
	{
		return this->state[0];
	}
	T lowpass12() const
	{
		return this->state[1];
	}
	T lowpass18() const
	{
		return this->state[2];
	}
	T lowpass24() const
	{
		return this->state[3];
	}
};

template <typename T, IntegratorType integratorType = IntegratorType::Transistor_tanh>
class SallenKeyFilterLpBp : public FilterAbstract<T, 2, integratorType>
{
protected:
	void f(T t, const T x[], T dxdt[]) const override
	{
		T hp1 = x[0] - x[1];
		T input = this->getInputt(t) + this->resonance * hp1; // positive feedback
		input = clamp(input, -this->maxAmplitude, this->maxAmplitude);

		this->calcLowpass(input, 0, x, dxdt);
		this->calcLowpass(x[0], 1, x, dxdt);
	}

public:
	T lowpass() const
	{
		return this->state[1];
	}

	T bandpass() const
	{
		return this->state[0] - this->state[1];
	}
};

template <typename T, IntegratorType integratorType = IntegratorType::Transistor_tanh>
class SallenKeyFilterHp : public FilterAbstract<T, 2, integratorType>
{
protected:
	void f(T t, const T x[], T dxdt[]) const override
	{
		T input = this->getInputt(t) + T(0.8) * this->resonance * x[1]; // positive feedback
		input = clamp(input, -this->maxAmplitude, this->maxAmplitude);

		this->calcLowpass(input, 0, x, dxdt);
		this->calcLowpass(input - x[0], 1, x, dxdt);
	}

public:
	T highpass6() const
	{
		return clamp(this->input, -this->maxAmplitude, this->maxAmplitude) - this->state[0];
	}

	T highpass12() const
	{
		return highpass6() - this->state[1];
	}
};

template <typename T, IntegratorType integratorType = IntegratorType::Transistor_tanh>
class DiodeClipper : public FilterAbstract<T, 1, integratorType>
{
protected:
	void f(T t, const T x[], T dxdt[]) const override
	{
		// resonance = drive
		T mult = T(0.5) + T(0.4) * this->resonance * this->resonance;
		T input = mult * this->getInputt(t);

		static const T a = {1.e-6};
		static const T b = 0.3 * this->maxAmplitude;

		T dxdtCap = this->omega0 * (input - x[0]); // dxdt of the capacitor

		// cap the diode currents (they can only limit dxdt, but not reverse it!)
		// if the exponentials are left uncapped, the solution can become instable easily
		dxdt[0] = dxdtCap
				- fmin(a * (exp( b * x[0]) - T(1.)), fabs(dxdtCap))
				+ fmin(a * (exp(-b * x[0]) - T(1.)), fabs(dxdtCap));
	}

public:
	T out()
	{
		this->clampStates(this->maxAmplitude);
		return this->state[0];
	}
};

template <typename T, IntegratorType integratorType = IntegratorType::Transistor_tanh>
class DiodeClipperAsym : public FilterAbstract<T, 2, integratorType>
{
protected:
	void f(T t, const T x[], T dxdt[]) const override
	{
		// resonance = drive
		T mult = T(0.5) + T(0.4) * this->resonance * this->resonance;
		T input = mult * this->getInputt(t);

		static const T a = {1.e-6};
		static const T b = 0.3 * this->maxAmplitude;

		T dxdtCap = this->omega0 * (input - x[0]); // dxdt of the capacitor

		// cap the diode currents (they can only limit dxdt, but not reverse it!)
		// if the exponentials are left uncapped, the solution can become instable easily
		dxdt[0] = dxdtCap
				- fmin(a * (exp( b * x[0]) - T(1.)), fabs(dxdtCap))
				+ fmin(a * T(0.5) * (exp(T(-2.) * b * x[0]) - T(1.)), fabs(dxdtCap));

		// highpass  20Hz to get rid of DC
		dxdt[1] = 2 * T(M_PI) * T(20.) * (x[0] - x[1]);
	}

public:
	T out()
	{
		this->clampStates(this->maxAmplitude);
		// scale to match amplitude of DiodeClipper
		return T(1.306) * (this->state[0] - this->state[1]);
	}
};

}
