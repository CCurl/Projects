#include "pch.h"
#include "Critter.h"

Critter Critter::critters[1000];
int Critter::numCritters;

Critter* Critter::At(int index) {
	return &critters[index];
}

void Critter::CreateRandom(int xPos, int yPos, Brain *brain) {
	x = xPos;
	y = yPos;
	heading = rand() % 8;
	for (int i = 0; i < brain->numConnections; i++) {
		CONN_T* pC = &connection[i];
		pC->src = brain->getRandomNeuronID();
		pC->sink = brain->getRandomNeuronID();
		double w = rand();
		if (rand() < 16000) { w = -w; }
		pC->weight = w / 8000;
		TRACE("%02x, %02x, %g\n", pC->src, pC->sink, pC->weight);
	}
}

double Critter::getInput(byte type) {
	// TRACE("critter.getInput (%d)\n", n->type);
	switch (type) {
	case 1: break;
	case 2: break;
	case 3: break;
	default: break;
	}
	return 0;
}

void Critter::doOutput(byte type, int signalStrength) {
	// TRACE("critter.doOutput(%d)\n", n->type);
	switch (type) {
	case 0: break;
	case 1: break;
	case 2: break;
	case 3: break;
	default: break;
	}
}

NEURON_T* Brain::getSourceNeuron(byte neuronId) {
	byte type = NEURON_TYPE(neuronId);
	byte id = NEURON_ID(neuronId);
	return (type) ? &input[id] : &hidden[id];
}

NEURON_T* Brain::getSinkNeuron(byte neuronId) {
	byte type = NEURON_TYPE(neuronId);
	byte id = NEURON_ID(neuronId);
	return (type) ? &output[id] : &hidden[id];
}

void Brain::doInput(Critter* critter, CONN_T* conn) {
	NEURON_T* f = getSourceNeuron(conn->src);
	NEURON_T* t = getSinkNeuron(conn->sink);
	double v = 0;
	if (IS_INPUT(f->id)) {
		v = critter->getInput(NEURON_ID(f->id));
	}
	else {
		v = tanh(f->sum);
	}
	t->sum += (v * conn->weight);
}

void Brain::doOutput(Critter* critter, CONN_T* conn) {
	NEURON_T* t = getSinkNeuron(conn->sink);
	// t->sum == 0 means do nothing
	if (t->sum != 0) {
		double v = tanh(t->sum);
		// In case there are multiple connections to this neuron
		t->sum = 0;
		if (0.05 < v) {
			critter->doOutput(NEURON_ID(conn->sink), (int)(v * 100));
		}
	}
}

void Brain::OneStep(Critter * critter) {
	for (int i = 0; i < numConnections; i++) {
		CONN_T* pC = &critter->connection[i];
		getSourceNeuron(pC->src)->sum = 0;
		getSinkNeuron(pC->sink)->sum = 0;
	}

	// TEMP
	critter->x = rand() % 100;
	critter->y = rand() % 100;

	for (int i = 0; i < numConnections; i++) {
		CONN_T * pC = &critter->connection[i];
		doInput(critter, pC);
	}

	for (int i = 0; i < numConnections; i++) {
		CONN_T* pC = &critter->connection[i];
		if (IS_OUTPUT(pC->sink)) {
			doOutput(critter, pC);
		}
	}
}

void Brain::Init(int numI, int numH, int numO, int numC) {
	numNeurons = numI + numH + numO;
	numInputs = numI;
	numOutputs = numO;
	numHidden = numH;
	numConnections = numC;
}

byte Brain::getRandomNeuronID() {
	return rand() & 0xff;
}
