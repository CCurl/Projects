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

class Brain;

class Critter
{
public:
	static Critter critters[1000];
	static int numCritters;
	static Critter* At(int index);

	int x, y, heading;
	CONN_T connection[64];
	void CreateRandom(int x, int y, Brain *brain);
	CONN_T* getConnection(int index) { return &connection[index]; }
	double getInput(byte type);
	void doOutput(byte type, int signalStrength);
};

class Brain
{
public:
	int numNeurons;
	int numConnections;
	int numInputs;
	int numHidden;
	int numOutputs;
	NEURON_T input[128];
	NEURON_T hidden[128];
	NEURON_T output[128];

	void Init(int numI, int numH, int numO, int numC);
	void OneStep(Critter *critter);
	void doInput(Critter * critter, CONN_T *conn);
	void doOutput(Critter * critter, CONN_T *conn);
	NEURON_T *getSourceNeuron(byte id);
	NEURON_T *getSinkNeuron(byte id);
	byte getRandomNeuronID();
	int CopyBit(int bit, int bitPos);
	int CopyBits(unsigned long bits, int num);
	void CopyConnection(CONN_T* f, CONN_T* t);
	void createRandomConnection(CONN_T *pC);
};

class World {
public:
	int sz_x, sz_y;
	byte entity[500][500];
};
