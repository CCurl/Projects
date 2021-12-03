#pragma once

typedef struct {
	byte src;
	byte sink;
	short wt;
	double weight;
} CONN_T;

typedef struct {
	byte id;
	double sum;
} NEURON_T;

#define NEURON_TYPE(id) (id & 0x80)
#define NEURON_ID(id)   (id & 0x7f)
#define IS_INPUT(id)    (NEURON_TYPE(id) != 0)
#define IS_OUTPUT(id)   (NEURON_TYPE(id) != 0)
#define IS_HIDDEN(id)   (NEURON_TYPE(id) == 0)

#define MAX_INPUT       16
#define MAX_HIDDEN      16
#define MAX_OUTPUT      16

#define WORLD_SZX       256
#define WORLD_SZY       256

class Brain;

class Critter
{
public:
	static Critter critters[1000];
	static int numCritters;
	static Critter* At(int index);

	int x, y, lX, lY, heading;
	CONN_T connection[512];
	void CreateRandom(int x, int y, Brain *brain);
	CONN_T* getConnection(int index) { return &connection[index]; }
	double getInput(byte type);
	void doOutput(byte type, int signalStrength);
	void RememberLoc() { lX = x; lY = y; }
	void MoveTo(int X, int Y) { x = X; y = Y; }
};

class Brain
{
public:
	int numConnections;
	int numInputs;
	int numHidden;
	int numOutputs;
	NEURON_T input[MAX_INPUT];
	NEURON_T hidden[MAX_HIDDEN];
	NEURON_T output[MAX_OUTPUT];

	void Init(int numH, int numC);
	void OneStep(Critter *critter);
	void doInput(Critter * critter, CONN_T *conn);
	void doOutput(Critter * critter, CONN_T *conn);
	NEURON_T *getSourceNeuron(byte id);
	NEURON_T *getSinkNeuron(byte id);
	byte getRandomNeuronID(bool isInput);
	int CopyBit(int bit, int bitPos);
	int CopyBits(unsigned long bits, int num);
	void CopyConnection(CONN_T* f, CONN_T* t);
	void createRandomConnection(CONN_T *pC);
};

class World {
public:
	int sz_x, sz_y;
	int entity[WORLD_SZX][WORLD_SZY];
	void Init() { memset(&entity, 0, sizeof(entity)); }
	int EntityAt(byte x, byte y) { return entity[x][y]; }
	void SetEntityAt(byte x, byte y, int E) { entity[x][y] = E; }
	bool IsCritter(int E) { return (E & 0x0FFF) != 0; }
	int CritterID(int E) { return E & 0x0FFF; }
	int CritterAt(byte x, byte y) { return CritterID(EntityAt(y, y)); }
};
