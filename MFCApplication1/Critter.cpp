#include "pch.h"
#include "Critter.h"

World World::theWorld;
Critter Critter::critters[1000];
int Critter::numCritters;

inline World* TheWorld() { return &World::theWorld; }

Critter* Critter::At(int index) {
	return &critters[index];
}

void Critter::CreateRandom(int ID, byte xPos, byte yPos, Brain* brain) {
	id = ID;
	x = y = lX = lY = 0;
	MoveTo(xPos, yPos);
	heading = RAND(8);
	for (int i = 0; i < brain->numConnections; i++) {
		brain->createRandomConnection(ConnectionAt(i));
	}
}

double Critter::getInput(byte type) {
	// TRACE("critter.getInput (%d)\n", n->type);
	switch (type) {
	case 0: return abs(sin(rand()));
	case 1: return abs(sin(rand()));
	case 2: return abs(sin(rand()));
	case 3: return abs(sin(rand()));
	case 4: return abs(sin(rand()));
	case 5: return abs(sin(rand()));
	case 6: return abs(sin(rand()));
	case 7: return abs(sin(rand()));
	case 8: return abs(sin(rand()));
	case 9: return abs(sin(rand()));
	case 10: return abs(sin(rand()));
	case 12: return abs(sin(rand()));
	case 13: return abs(sin(rand()));
	case 14: return abs(sin(rand()));
	case 15: return abs(sin(rand()));
	default: break;
	}
	return 0;
}

void Critter::MoveTo(byte X, byte Y) {
	if (CanMoveTo(X, Y)) { 
		TheWorld()->SetEntityAt(x, y, 0);
		x = X;
		y = Y; 
		TheWorld()->SetEntityAt(x, y, id);
		// RememberLoc();
	}
}


bool Critter::CanMoveTo(byte X, byte Y) {
	World* w = TheWorld();
	if (w->sz_x < X) return false;
	if (w->sz_y < Y) return false;
	if (TheWorld()->EntityAt(X, Y)) return false;
	return true;
}


void Critter::doOutput(byte type, int signalStrength) {
	// TRACE("critter.doOutput(%d)\n", type);
	switch (type) {
	case 0: MoveTo(x + 0, y + 1);  break; // N
	case 1: MoveTo(x + 1, y + 1);  break; // NE
	case 2: MoveTo(x + 1, y + 0);  break; // E
	case 3: MoveTo(x + 1, y - 1);  break; // SE
	case 4: MoveTo(x + 0, y - 1);  break; // S
	case 5: MoveTo(x - 1, y - 1);  break; // SW
	case 6: MoveTo(x - 1, y + 0);  break; // W
	case 7: MoveTo(x - 1, y + 1);  break; // NW
	default: break;
	}
	if (x < 0) { x = 0; }
	if (y < 0) { y = 0; }
	if (128 < x) { x = 128; }
	if (128 < y) { y = 128; }
}

void Critter::DumpConnecton(int i) {
	CString x;
	CONN_T* c = ConnectionAt(i);
	x.Format("(%d,%d) -> (%d,%d)", NEURON_TYPE(c->src), NEURON_ID(c->src),
		NEURON_TYPE(c->sink), NEURON_ID(c->sink));
	TRACE("%s\n", x);
}

void Brain::DumpConnectons(Critter* c) {
	for (int i = 0; i < numConnections; i++) { c->DumpConnecton(i); }
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
	NEURON_T* t = getSinkNeuron(conn->sink);
	double v = 0;
	if (IS_INPUT(conn->src)) {
		v = critter->getInput(NEURON_ID(conn->src));
	}
	else {
		v = tanh(getSourceNeuron(conn->src)->sum);
	}
	t->sum += (v * conn->weight);
}

void Brain::doOutput(Critter* critter, CONN_T* conn) {
	NEURON_T* t = getSinkNeuron(conn->sink);
	// t->sum == 0 means do nothing
	if (t->sum != 0) {
		double s = tanh(t->sum);
		int p = (int)(s*100);
		int r = RAND100;
		// In case there are multiple connections to this neuron
		t->sum = 0;
		if (r < p) {
			critter->doOutput(NEURON_ID(conn->sink), p);
		}
	}
}

void Brain::OneStep(Critter * critter) {
	for (int i = 0; i < numConnections; i++) {
		CONN_T* pC = critter->ConnectionAt(i);
		getSourceNeuron(pC->src)->sum = 0;
		getSinkNeuron(pC->sink)->sum = 0;
	}

	for (int i = 0; i < numConnections; i++) {
		doInput(critter, critter->ConnectionAt(i));
	}

	for (int i = 0; i < numConnections; i++) {
		CONN_T* pC = critter->ConnectionAt(i);
		if (IS_OUTPUT(pC->sink)) {
			doOutput(critter, pC);
		}
	}
}

void Brain::Init(int numH, int numC) {
	numHidden = numH;
	numConnections = numC;
}

byte Brain::getRandomNeuronID(bool isInput) {
	int type = 0x80, id = 0;
	int pct = (numHidden * 100) / numConnections;
	bool isHidden = RAND100 < pct;
	if (isHidden) {
		type = 0;
		id = RAND(numHidden);
	}
	else {
		type = 0x80;
		id = RAND((isInput ? MAX_INPUT : MAX_OUTPUT));
	}
	return type | id;
}

void Brain::createRandomConnection(CONN_T *pC) {
	pC->src = getRandomNeuronID(true);
	pC->sink = getRandomNeuronID(false);
	pC->wt = rand();
	if (RAND100 < 40) { pC->wt = -pC->wt; }
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
