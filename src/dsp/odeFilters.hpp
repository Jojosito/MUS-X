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
	
	IntegratorType integratorType = IntegratorType::Transistor;
	NonlinearityType nonlinearityType = NonlinearityType::tanh;

	virtual void f(T t, const T x[], T dxdt[]) = 0;

	T clip(T x) const
	{
		switch (nonlinearityType)
		{
		case NonlinearityType::tanh:
			return 12.f * musx::tanh(x/12.f);
		case NonlinearityType::alt1:
			return musx::cheapSaturator(x);
		case NonlinearityType::alt2:
			return musx::AntialiasedCheapSaturator<T>::processNonBandlimited(x);
		case NonlinearityType::alt3:
			return 12.f * musx::waveshape(x/12.f);
		case NonlinearityType::hardclip:
			return clamp(x, -12.f, 12.f);
		}
	}

	T getInputt(T t) const
	{
		return crossfade(this->lastInput, this->input, t / this->dt);
	}
	
	T calcLowpass(size_t iStage, T in, T& dxdt) const
	{
		switch (integratorType)
		{
		case IntegratorType::Linear:
			dxdt = omega0 * (in - state[iStage]);
			return state[iStage];
		case IntegratorType::OTA:
			dxdt = omega0 * clip(in - state[iStage]);
			return state[iStage];
		case IntegratorType::Transistor:
		default:
			dxdt = omega0 * (clip(in) - clip(state[iStage]));
			return state[iStage];
		}
	}

	T calcLowpassInverting(size_t iStage, T in, T& dxdt) const
	{
		switch (integratorType)
		{
		case IntegratorType::Linear:
			dxdt = omega0 * (-(state[iStage] + in));
			return state[iStage];
		case IntegratorType::OTA:
		case IntegratorType::Transistor:
		default:
			dxdt = omega0 * -clip(state[iStage] + in);
			return state[iStage];
		}
	}

	T calcHighpass(size_t iStage, T in, T& dxdt) const
	{
		switch (integratorType)
		{
		case IntegratorType::Linear:
			dxdt = omega0 * (in - state[iStage]);
			return in - state[iStage];
		case IntegratorType::OTA:
		case IntegratorType::Transistor:
		default:
			dxdt = omega0 * (in - clip(state[iStage]));
			return in - state[iStage];
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

		// prevent filter from exploding
		clampStates(100.f);
	}

};


template <typename T>
class LadderFilter2Pole : public FilterAbstract<T, 4>
{
private:
	T lp2Out;
public:
	void f(T t, const T x[], T dxdt[]) override
	{
		T input = this->getInputt(t) - T(2.) * this->resonance * lp2Out; // negative feedback
		input = clamp(input, -12.f ,12.f);

		T out0 = this->calcLowpass(0, input, dxdt[0]);
		lp2Out = this->calcLowpass(1, out0,  dxdt[1]);
	}

	T lowpass()
	{
		return lp2Out;
	}
};

template <typename T>
class LadderFilter4Pole : public FilterAbstract<T, 4>
{
private:
	T lp4Out;
public:
	void f(T t, const T x[], T dxdt[]) override
	{
		T input = this->getInputt(t) - T(2.) * this->resonance * lp4Out; // negative feedback
		input = clamp(input, -12.f ,12.f);

		T out0 = this->calcLowpass(0, input, dxdt[0]);
		T out1 = this->calcLowpass(1, out0,  dxdt[1]);
		T out2 = this->calcLowpass(2, out1,  dxdt[2]);
		lp4Out = this->calcLowpass(3, out2,  dxdt[3]);
	}

	T lowpass()
	{
		return lp4Out;
	}
};

template <typename T>
class SallenKeyFilterLpBp : public FilterAbstract<T, 2>
{
private:
	T bpOut;
public:
	void f(T t, const T x[], T dxdt[]) override
	{
		T input = this->getInputt(t) + this->resonance * bpOut; // positive feedback
		input *= T(0.8);
		input = clamp(input, -12.f ,12.f);

		T out0  = this->calcLowpass(0, input, dxdt[0]);
		bpOut = this->calcHighpass(1, out0,  dxdt[1]);
	}

	T lowpass()
	{
		return this->state[1];
	}

	T bandpass()
	{
		return bpOut;
	}
};

template <typename T>
class SallenKeyFilterHp : public FilterAbstract<T, 2>
{
private:
	T lpOut;
	T hpOut;
public:
	void f(T t, const T x[], T dxdt[]) override
	{
		T input = this->getInputt(t) + T(0.8) * this->resonance * lpOut; // positive feedback
		input = clamp(input, -12.f ,12.f);

		hpOut  = this->calcHighpass(0, input, dxdt[0]);
		lpOut = this->calcLowpass(1, hpOut,  dxdt[1]);
	}

	T highpass6()
	{
		return hpOut;
	}

	T highpass12()
	{
		return hpOut - this->state[1];
	}
};

}