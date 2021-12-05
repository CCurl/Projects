#pragma once

typedef struct {
	byte type;
	byte id;
} C_T ;

typedef struct {
	C_T src;
	C_T sink;
	short wt;
	double weight;
} CONN_T;

typedef struct {
	byte id;
	double sum;
} NEURON_T;

#define HIDDEN_NEURON  0
#define INPUT_NEURON   1
#define OUTPUT_NEURON  2

#define CONN_TYPE(c)   (c.type)
#define CONN_ID(c)     (c.id)
#define IS_HIDDEN(c)   (CONN_TYPE(c) == HIDDEN_NEURON)
#define IS_INPUT(c)    (CONN_TYPE(c) == INPUT_NEURON)
#define IS_OUTPUT(c)   (CONN_TYPE(c) == OUTPUT_NEURON)

#define RAND100         (rand() / 328)

#define MAX_INPUT       5
#define MAX_HIDDEN      16
#define MAX_OUTPUT      18

#define MAX_CRITTERS    1024
#define WORLD_SZX       256
#define WORLD_SZY       256

#define WSX             WORLD_SZX
#define WSY             WORLD_SZY
#define MX(x, sz)       (x&(sz-1))
#define RAND(mx)        (rand() % mx)

#define CRITTER_MASK     0x7FFF
#define OBSTACLE_MASK    0x8000

//class Brain;
//class World;

class World {
public:
	byte sz_x, sz_y;
	int entity[WORLD_SZX][WORLD_SZY];
	void Init() { memset(&entity, 0, sizeof(entity)); }
	void SetSize(byte X, byte Y) { sz_x = MX(X, WSX); sz_y = MX(Y, WSY); }
	int EntityAt(byte x, byte y);
	void SetEntityAt(byte x, byte y, int E) { entity[MX(x, WSX)][MX(y, WSY)] = E; }
	bool IsCritter(int E) { return (E & 0x0FFF) != 0; }
	int CritterID(int E) { return E & 0x0FFF; }
	int CritterAt(byte x, byte y) { return CritterID(EntityAt(y, y)); }
	bool inBounds(byte x, byte y) { return CritterID(EntityAt(y, y)); }
};

class Critter
{
public:
	int id, health;
	byte x, y, lX, lY, heading;
	void SetHeading(byte d) { heading = d & 0x07; doOutput(heading, 0);  }
	CONN_T connection[512];
	void CreateRandom(int ID);
	void CreateDescendent(Critter *parent);
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
	static Brain theBrain;
	int numConnections;
	int numHidden;
	NEURON_T input[MAX_INPUT];
	NEURON_T hidden[MAX_HIDDEN];
	NEURON_T output[MAX_OUTPUT];

	void Init(int numH, int numC);
	void OneStep(Critter *critter);
	//void doInput(Critter * critter, CONN_T *conn);
	//void doOutput(Critter * critter, CONN_T *conn);
	// byte getRandomNeuronID(bool isInput);
	//void createRandomConnection(CONN_T *pC);
	void DumpConnectons(Critter* c);
};

extern World* TheWorld();
extern Brain* TheBrain();
extern Critter critters[MAX_CRITTERS];
extern int numCritters;
extern Critter* CritterAt(int index);

extern int CopyBit(int bit, int bitPos);
extern int CopyBits(unsigned long bits, int num);
extern void CopyConnection(CONN_T* f, CONN_T* t);
extern void createRandomConnection(CONN_T* pC);
extern bool isCritterAt(byte x, byte y);
extern void selectCritters(int which);
extern void nextGeneration();
