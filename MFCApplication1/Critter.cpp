#include "pch.h"
#include "Critter.h"

World theWorld;
Brain theBrain;
Critter critters[MAX_CRITTERS];
int numCritters;

inline World* TheWorld() { return &theWorld; }
inline Brain* TheBrain() { return &theBrain; }

Critter* CritterAt(int index) {
	return &critters[index];
}

bool isCritterAt(byte x, byte y) {
	return (theWorld.EntityAt(x, y) & CRITTER_MASK) != 0;
}

double numCrittersNearby(byte X, byte Y, byte dist) {
	int num = 0;
	World* w = TheWorld();
	byte sX = max((int)X-dist, 0);
	byte sY = max((int)Y-dist, 0);
	byte eX = min(X + dist, w->sz_x);
	byte eY = min(Y + dist, w->sz_y);
	for (int x = sX; x < eX; x++) {
		for (int y = sY; y < eY; y++) {
			num += (isCritterAt(x, y)) ? 1 : 0;
		}
	}
	return ((double)num-2) / ((double)dist*10);
}

double nearNorth(byte X, byte Y) {
	double sz = TheWorld()->sz_y;
	return Y / sz;
}

double nearSouth(byte X, byte Y) {
	double sz = TheWorld()->sz_y;
	return (sz - Y) / sz;
}

double nearWest(byte X, byte Y) {
	double sz = TheWorld()->sz_x;
	return X / sz;
}

double nearEast(byte X, byte Y) {
	double sz = TheWorld()->sz_x;
	return (sz - X) / sz;
}

void selectCritter(Critter *c, int which) {
	World* w = TheWorld();
	switch (which) {
	case 1: c->health = (c->x < 25) ? 1 : 0;
		break;
	case 2: c->health = ((w->sz_x - 25) < c->x) ? 1 : 0;
		break;
	}
}

void copyCritter(Critter* t, Critter* f) {
	World* w = TheWorld();
	t->health = 1;
	while ((t->x + t->y) == 0) {
		int x = RAND(w->sz_x);
		int y = RAND(w->sz_y);
		t->MoveTo(x, y);
	}
}


Critter* nextAlive(int start) {
	for (int i = start + 1; i <= numCritters; i++) {
		Critter* c = CritterAt(i);
		if (c->health) { return c; }
	}
	return NULL;
}

void nextGeneration() {
	World* w = TheWorld();
	w->Init();
	Critter* a = nextAlive(0);
	for (int i = 1; i <= numCritters; i++) {
		Critter* c = CritterAt(i);
		c->x = c->y = 0;
		c->RememberLoc();
		while ((c->x + c->y) == 0) {
			c->MoveTo(RAND(w->sz_x), RAND(w->sz_y));
		}

		if (c->health == 0) {
			if (a) {
				c->CreateDescendent(a);
				a = nextAlive(a->id);
			}
			else {
				c->CreateRandom(c->id);
			}
		}
		c->health = 100;
	}
}

void selectCritters(int which) {
	for (int i = 1; i <= numCritters; i++) {
		selectCritter(CritterAt(i), which);
	}
}

void Critter::CreateDescendent(Critter *parent) {
	World *w = TheWorld();
	x = y = lX = lY = 0;
	while ((x + y) == 0) {
		MoveTo(RAND(w->sz_x), RAND(w->sz_y));
	}
	SetHeading(RAND(8));
	for (int i = 0; i < theBrain.numConnections; i++) {
		CopyConnection(ConnectionAt(i), parent->ConnectionAt(i));
	}
}

void Critter::CreateRandom(int ID) {
	World* w = TheWorld();
	id = ID;
	x = y = lX = lY = 0;
	while ((x + y) == 0) {
		MoveTo(RAND(w->sz_x), RAND(w->sz_y));
	}
	SetHeading(RAND(8));
	for (int i = 0; i < theBrain.numConnections; i++) {
		createRandomConnection(ConnectionAt(i));
	}
}

double Critter::getInput(byte type) {
	// TRACE("critter.getInput (%d)\n", n->type);
	switch (type) {
	case 0: return nearNorth(x, y);
	case 1: return nearEast(x, y);
	case 2: return nearSouth(x, y);
	case 3: return nearWest(x, y);
	case 4: return numCrittersNearby(x, y, 1);
	default: break;
	}
	return 0;
}

void Critter::MoveTo(byte X, byte Y) {
	if (CanMoveTo(X, Y)) { 
		if (x && y) { TheWorld()->SetEntityAt(x, y, 0); }
		x = X;
		y = Y; 
		TheWorld()->SetEntityAt(x, y, id);
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
	case  0: MoveTo(x + 0, y + 1);  break; // N
	case  1: MoveTo(x + 1, y + 1);  break; // NE
	case  2: MoveTo(x + 1, y + 0);  break; // E
	case  3: MoveTo(x + 1, y - 1);  break; // SE
	case  4: MoveTo(x + 0, y - 1);  break; // S
	case  5: MoveTo(x - 1, y - 1);  break; // SW
	case  6: MoveTo(x - 1, y + 0);  break; // W
	case  7: MoveTo(x - 1, y + 1);  break; // NW
	case  8: doOutput(heading, 0);  break;
	case  9: SetHeading(0);      break;
	case 10: SetHeading(1);      break;
	case 11: SetHeading(2);      break;
	case 12: SetHeading(3);      break;
	case 13: SetHeading(3);      break;
	case 14: SetHeading(4);      break;
	case 15: SetHeading(5);      break;
	case 16: SetHeading(6);      break;
	case 17: SetHeading(7);      break;
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
	x.Format("(%d,%d) -> (%d,%d)", CONN_TYPE(c->src), CONN_ID(c->src),
		CONN_TYPE(c->sink), CONN_ID(c->sink));
	TRACE("%s\n", x);
}

void Brain::DumpConnectons(Critter* c) {
	for (int i = 0; i < numConnections; i++) { c->DumpConnecton(i); }
}

NEURON_T* getSourceNeuron(C_T c) {
	byte type = CONN_TYPE(c);
	byte id = CONN_ID(c);
	return (type == INPUT_NEURON) ? &theBrain.input[id] : &theBrain.hidden[id];
}

NEURON_T* getSinkNeuron(C_T c) {
	byte type = CONN_TYPE(c);
	byte id = CONN_ID(c);
	return (type == OUTPUT_NEURON) ? &theBrain.output[id] : &theBrain.hidden[id];
}

void doInput(Critter* critter, CONN_T* conn) {
	NEURON_T* t = getSinkNeuron(conn->sink);
	double v = 0;
	if (IS_INPUT(conn->src)) {
		v = critter->getInput(CONN_ID(conn->src));
	}
	else {
		v = tanh(getSourceNeuron(conn->src)->sum);
	}
	t->sum += (v * conn->weight);
}

void doOutput(Critter* critter, CONN_T* conn) {
	NEURON_T* t = getSinkNeuron(conn->sink);
	// t->sum == 0 means do nothing
	if (t->sum != 0) {
		double s = tanh(t->sum);
		int p = (int)(s*100);
		int r = RAND100;
		// In case there are multiple connections to this neuron
		t->sum = 0;
		if (r < p) {
			critter->doOutput(CONN_ID(conn->sink), p);
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

void setRandomSource(CONN_T* c) {
	int pct = (theBrain.numHidden * 100) / theBrain.numConnections;
	bool isHidden = RAND100 < pct;
	if (isHidden) {
		c->src.type = HIDDEN_NEURON;
		c->src.id = RAND(theBrain.numHidden);
	}
	else {
		c->src.type = INPUT_NEURON;
		c->src.id = RAND(MAX_INPUT);
	}
}

void setRandomSink(CONN_T* c) {
	int pct = (theBrain.numHidden * 100) / theBrain.numConnections;
	bool isHidden = RAND100 < pct;
	if (isHidden) {
		c->sink.type = HIDDEN_NEURON;
		c->sink.id = RAND(theBrain.numHidden);
	}
	else {
		c->sink.type = OUTPUT_NEURON;
		c->sink.id = RAND(MAX_OUTPUT);
	}
}

void createRandomConnection(CONN_T *pC) {
	setRandomSource(pC);
	setRandomSink(pC);
	pC->wt = rand();
	if (RAND100 < 40) { pC->wt = -pC->wt; }
	pC->weight = pC->wt / 8000.0;
}

int CopyBit(int bits, int bitPos) {
	int bit = (bits & bitPos);
	if (rand() < 35) {
		return bit ? 0 : bitPos;
	}
	return bit;
}

int CopyBits(unsigned long bits, int num) {
	unsigned long ret = 0;
	int bitPos = 1;
	for (int i = 0; i < num; i++) {
		ret |= CopyBit(bits, bitPos);
		bitPos = (bitPos << 1);
	}
	return (int)ret;
}

void CopyConnection(CONN_T* f, CONN_T* t) {
	t->src.type = (byte)CopyBits(f->src.type, 1);
	t->src.id = (byte)CopyBits(f->src.id, 8);
	t->sink.type = (byte)CopyBits(f->sink.type, 1);
	t->sink.id = (byte)CopyBits(f->sink.id, 8);
	t->wt = (short)CopyBits(f->wt, 16);
	t->weight = t->wt / 8000.0;
}

int World::EntityAt(byte x, byte y) {
	return entity[MX(x, WSX)][MX(y, WSY)]; 
}
