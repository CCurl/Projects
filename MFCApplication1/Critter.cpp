#include "pch.h"
#include "Critter.h"

Critter Critter::critters[1000];
int Critter::numCritters;

Critter* Critter::At(int index) {
	return &critters[index];
}

void Critter::CreateRandom(int xPos, int yPos, Brain* brain) {
	x = xPos;
	y = yPos;
	heading = rand() % 8;
	for (int i = 0; i < brain->numConnections; i++) {
		brain->createRandomConnection(getConnection(i));
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
		if (0.01 < v) {
			critter->doOutput(NEURON_ID(conn->sink), (int)(v * 100));
		}
	}
}

void Brain::OneStep(Critter * critter) {
	for (int i = 0; i < numConnections; i++) {
		CONN_T* pC = critter->getConnection(i);
		getSourceNeuron(pC->src)->sum = 0;
		getSinkNeuron(pC->sink)->sum = 0;
	}

	for (int i = 0; i < numConnections; i++) {
		doInput(critter, critter->getConnection(i));
	}

	for (int i = 0; i < numConnections; i++) {
		CONN_T* pC = critter->getConnection(i);
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

void Brain::createRandomConnection(CONN_T *pC) {
	pC->src = getRandomNeuronID();
	pC->sink = getRandomNeuronID();
	pC->wt = rand();
	if (rand() < 16000) { pC->wt = -pC->wt; }
	pC->weight = pC->wt / 8000.0;
	// TRACE("%02x, %02x, %g\n", pC->src, pC->sink, pC->weight);
}

int Brain::CopyBit(int bits, int bitPos) {
	int bit = (bits & bitPos);
	if (rand() < 35) {
		return bit ? 0 : bitPos;
	}
	return bit;
}

int Brain::CopyBits(unsigned long bits, int num) {
	unsigned long ret = 0;
	int bitPos = 1;
	for (int i = 0; i < num; i++) {
		ret |= CopyBit(bits, bitPos);
		bitPos = (bitPos << 1);
	}
	return (int)ret;
}

void Brain::CopyConnection(CONN_T* f, CONN_T* t) {
	t->src = (byte)CopyBits(f->src, 8);
	t->sink = (byte)CopyBits(f->sink, 8);
	t->wt = (short)CopyBits(f->wt, 16);
	t->weight = t->wt / 8000.0;
}
