#pragma once
typedef struct {
	byte type;
	byte from;
	byte to;
	byte weight;
} CONN_T;

class Critter
{
public:
	int numConnections;
	CONN_T connection[64];
	void Think();
	int getInput(int type);
	void doOutput(int type);
};

