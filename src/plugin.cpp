#include "plugin.hpp"

using namespace rack;
using namespace musx;

Plugin* pluginInstance;


void init(Plugin* p) {
	pluginInstance = p;

	// Add modules here
	p->addModel(modelADSR);
	p->addModel(modelDelay);
	p->addModel(modelDrift);
	p->addModel(modelModMatrix);
	p->addModel(modelOnePole);
	p->addModel(modelOnePoleLP);
	p->addModel(modelOscillators);
	p->addModel(modelTuner);

	// Any other plugin initialization may go here.
	// As an alternative, consider lazy-loading assets and lookup tables when your module is created to reduce startup times of Rack.
}
