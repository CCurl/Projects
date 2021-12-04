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

#define RAND100         (rand() / 328)

#define MAX_INPUT       16
#define MAX_HIDDEN      16
#define MAX_OUTPUT      18

#define WORLD_SZX       256
#define WORLD_SZY       256

#define WSX             WORLD_SZX
#define WSY             WORLD_SZY
#define MX(x, sz)       (x&(sz-1))
#define RAND(mx)        (rand() % mx)

class Brain;
class World;
	World *TheWorld();

class World {
public:
	static World theWorld;
	byte sz_x, sz_y;
	int entity[WORLD_SZX][WORLD_SZY];
	void Init() { memset(&entity, 0, sizeof(entity)); }
	void SetSize(byte X, byte Y) { sz_x = MX(X, WSX); sz_y = MX(Y, WSY); }
	int EntityAt(byte x, byte y) { return entity[MX(x, WSX)][MX(y, WSY)]; }
	void SetEntityAt(byte x, byte y, int E) { entity[MX(x, WSX)][MX(y, WSY)] = E; }
	bool IsCritter(int E) { return (E & 0x0FFF) != 0; }
	int CritterID(int E) { return E & 0x0FFF; }
	int CritterAt(byte x, byte y) { return CritterID(EntityAt(y, y)); }
};

class Critter
{
public:
	static Critter critters[1000];
	static int numCritters;
	static Critter* At(int index);

	int id;
	byte x, y, lX, lY, heading;
	void SetHeading(byte d) { heading = d & 0x07; doOutput(heading, 0);  }
	CONN_T connection[512];
	void CreateRandom(int ID, byte x, byte y, Brain *brain);
	CONN_T* ConnectionAt(int index) { return &connection[index]; }
	double getInput(byte type);
	void doOutput(byte type, int signalStrength);
	void RememberLoc() { lX = x; lY = y; }
	void MoveTo(byte X, byte Y);
	bool CanMoveTo(byte X, byte Y);
	void DumpConnecton(int i);
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
	void DumpConnectons(Critter* c);
};
