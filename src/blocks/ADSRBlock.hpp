#include "plugin.hpp"

namespace musx {

using namespace rack;
using simd::float_4;

class ADSRBlock {
private:
	float attackTarget;
	float minTime;
	float logLambdaBase;

	float_4 gate = {};
	float_4 attacking = {};
	float_4 env = {};
	dsp::TSchmittTrigger<float_4> trigger;
	float_4 attackLambda = {};
	float_4 decayLambda = {};
	float_4 releaseLambda = {};
	float_4 sustain = {};

	float_4 velScaling = {};
	float_4 velocity = {};

public:
	// [s]
	ADSRBlock(float minTime = 0.001f, float maxTime = 10.f, float attackTarget = 1.2f)
	{
		this->attackTarget = attackTarget;
		this->minTime = minTime;
		logLambdaBase = std::log(maxTime / minTime);
	}

	// [s]
	void setAttackTime(float_4 t)
	{
		attackLambda = simd::exp(-t * logLambdaBase) / minTime;
	}

	void multAttackLambda(float_4 mult)
	{
		attackLambda *= mult;
	}

	// [s]
	void setDecayTime(float_4 t)
	{
		decayLambda = simd::exp(-t * logLambdaBase) / minTime;
	}

	void multDecayLambda(float_4 mult)
	{
		decayLambda *= mult;
	}

	// [0..1]
	void setSustainLevel(float_4 s)
	{
		sustain = simd::clamp(s, 0.f, 1.f);;
	}

	// [s]
	void setReleaseTime(float_4 t)
	{
		releaseLambda = simd::exp(-t * logLambdaBase) / minTime;
	}

	void multReleaseLambda(float_4 mult)
	{
		releaseLambda *= mult;
	}

	// [0..1]
	void setVelocityScaling(float_4 v)
	{
		velScaling = simd::clamp(v, 0.f, 1.f);;
	}

	void setGate(float_4 g)
	{
		attacking |= ((g >= 1.f) & ~gate);
		gate = g >= 1.f;
	}

	void setRetrigger(float_4 t)
	{
		float_4 triggered = trigger.process(t);
		attacking |= triggered;
	}

	// [0..10]
	void setVelocity(float_4 v)
	{
		velocity = simd::clamp(v, 0.f, 10.f);;
	}

	float_4 getDecaySustainGate()
	{
		return simd::ifelse((gate & ~attacking), 10.f, 0.f);
	}

	float_4 process(float sampleTime)
	{
		// Turn off attacking state if gate is LOW
		attacking &= gate;

		// Get target and lambda for exponential decay
		float_4 target = simd::ifelse(attacking, attackTarget, simd::ifelse(gate, sustain, 0.f));
		float_4 lambda = simd::ifelse(attacking, attackLambda, simd::ifelse(gate, decayLambda, releaseLambda));

		// Adjust env
		env += (target - env) * lambda * sampleTime;

		// Turn off attacking state if envelope is HIGH
		attacking &= (env < 1.f);

		// velocity
		float_4 scale = 1.f - velScaling +
				0.1f * velocity * velScaling;

		// Set output
		return 10.f * scale * env;
	}

};

}
