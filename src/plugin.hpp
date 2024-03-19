#pragma once
#include <rack.hpp>

using namespace rack;

// Declare the Plugin, defined in plugin.cpp
extern Plugin* pluginInstance;

namespace musx {

// Declare each Model, defined in each module source file
extern Model* modelADSR;
extern Model* modelDelay;
extern Model* modelDrift;
extern Model* modelLast;
extern Model* modelLFO;
extern Model* modelModMatrix;
extern Model* modelOnePole;
extern Model* modelOnePoleLP;
extern Model* modelOscillators;
extern Model* modelTuner;

}
