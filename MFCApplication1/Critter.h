#pragma once
typedef struct {
	byte type;
	byte from;
	byte to;
	byte weight;
} CONN_T;

class Critter
{
	static Critter critters[10000];
	static int numCritters;
	int numConnections;
public:
	int x, y, heading;
	CONN_T connection[64];
	void Think();
	int getInput(int type);
	void doOutput(int type);
};
