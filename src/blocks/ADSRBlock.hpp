#include "plugin.hpp"

namespace musx {

using namespace rack;
using simd::float_4;

class ADSRBlock {
private:
	float attackTarget;
	float minTime;
	float logLambdaBase;

	float_4 gate[4] = {};
	float_4 attacking[4] = {};
	float_4 env[4] = {};
	dsp::TSchmittTrigger<float_4> trigger[4];
	float_4 attackLambda[4] = {};
	float_4 decayLambda[4] = {};
	float_4 releaseLambda[4] = {};
	float_4 sustain[4] = {};

	float_4 velScaling[4] = {};
	float_4 velocity[4] = {};

public:
	// [s]
	ADSRBlock(float minTime = 0.001f, float maxTime = 10.f, float attackTarget = 1.2f)
	{
		this->attackTarget = attackTarget;
		this->minTime = minTime;
		logLambdaBase = std::log(maxTime / minTime);
	}

	// [s]
	void setAttackTime(float_4 t, int c)
	{
		attackLambda[c/4] = simd::exp(-t * logLambdaBase) / minTime;
	}

	void multAttackLambda(float_4 mult, int c)
	{
		attackLambda[c/4] *= mult;
	}

	// [s]
	void setDecayTime(float_4 t, int c)
	{
		decayLambda[c/4] = simd::exp(-t * logLambdaBase) / minTime;
	}

	void multDecayLambda(float_4 mult, int c)
	{
		decayLambda[c/4] *= mult;
	}

	// [0..1]
	void setSustainLevel(float_4 s, int c)
	{
		sustain[c/4] = simd::clamp(s, 0.f, 1.f);;
	}

	// [s]
	void setReleaseTime(float_4 t, int c)
	{
		releaseLambda[c/4] = simd::exp(-t * logLambdaBase) / minTime;
	}

	void multReleaseLambda(float_4 mult, int c)
	{
		releaseLambda[c/4] *= mult;
	}

	// [0..1]
	void setVelocityScaling(float_4 v, int c)
	{
		velScaling[c/4] = simd::clamp(v, 0.f, 1.f);;
	}

	void setGate(float_4 g, int c)
	{
		attacking[c/4] |= ((g >= 1.f) & ~gate[c/4]);
		gate[c/4] = g >= 1.f;
	}

	void setRetrigger(float_4 t, int c)
	{
		float_4 triggered = trigger[c/4].process(t);
		attacking[c/4] |= triggered;
	}

	// [0..10]
	void setVelocity(float_4 v, int c)
	{
		velocity[c/4] = simd::clamp(v, 0.f, 10.f);;
	}

	float_4 getDecaySustainGate(int c)
	{
		return simd::ifelse((gate[c/4] & ~attacking[c/4]), 10.f, 0.f);
	}

	float_4 process(float sampleTime, int c)
	{
		// Turn off attacking state if gate is LOW
		attacking[c/4] &= gate[c/4];

		// Get target and lambda for exponential decay
		float_4 target = simd::ifelse(attacking[c/4], attackTarget, simd::ifelse(gate[c/4], sustain[c/4], 0.f));
		float_4 lambda = simd::ifelse(attacking[c/4], attackLambda[c/4], simd::ifelse(gate[c/4], decayLambda[c/4], releaseLambda[c/4]));

		// Adjust env
		env[c/4] += (target - env[c/4]) * lambda * sampleTime;

		// Turn off attacking state if envelope is HIGH
		attacking[c/4] &= (env[c/4] < 1.f);

		// velocity
		float_4 scale = 1.f - velScaling[c/4] +
				0.1f * velocity[c/4] * velScaling[c/4];

		// Set output
		return 10.f * scale * env[c/4];
	}

};

}
