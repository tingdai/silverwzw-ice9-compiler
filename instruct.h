#ifndef _INSTRUCT_H_
#define _INSTRUCT_H_ 1
#include <iostream>
#include <vector>
#include "memmgr.h"

enum insCode {
	IN,
	OUT,
	INB,
	OUTB,
	OUTC,
	OUTNL,
	HALT,
	ADD,
	SUB,
	MUL,
	DIV,

	LDC,
	LDA,
	LD,
	ST,

	JLT,
	JLE,
	JEQ,
	JNE,
	JGE,
	JGT,

	GOTO //dummy instruction, will be replaced later
};


struct IMData {
	unsigned absoluteAddr;
	insCode ins;
	int para[3];
	unsigned gotoIM;
};

class IMMgr {
private:
	std::vector<IMData> imdata;
public:
	unsigned newIM(insCode i, int d1, int d2, int d3);
	unsigned newIM(unsigned im_index); //goto ins
	void assignAbsoluteAddr(unsigned index, unsigned addr);
	void toTM(unsigned index, std::ostream&);
};

extern IMMgr imMgr;

class Block {
private:
	std::vector<unsigned> im;
public:
	unsigned entrance();
	void toTM(std::ostream&);
	void add(unsigned i);
	unsigned assignAbsoluteAddr(unsigned Offset); // return the next available ins slot
};

class ProcBlock {
private:
	std::vector<Block> blocks;
public:
	ARMgr arMgr;
	unsigned entrance();
	void toTM(std::ostream&);
	void add(Block b);
	unsigned assignAbsoluteAddr(unsigned Offset);
};

/*
1. produce intermedia
2. assign index
3. elimate GOTO & to TM
*/
#endif
