#include <cassert>
#include "instruct.h"

/*
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
*/

IMMgr imMgr;

unsigned IMMgr::newIM(insCode i, int d1, int d2, int d3) {
	IMData imtmp;

	assert(i != GOTO);

	imtmp.absoluteAddr = 0xFFFF;
	imtmp.ins = i;
	imtmp.para[0] = d1;
	imtmp.para[1] = d2;
	imtmp.para[2] = d3;
	imtmp.gotoIM = 0xFFFF;

	imdata.push_back(imtmp);
}

unsigned IMMgr::newIM(unsigned im_index) {
	IMData imtmp;

	imtmp.absoluteAddr = 0xFFFF;
	imtmp.ins = GOTO;
	imtmp.para[0] = 0xFFFF;
	imtmp.para[1] = 0xFFFF;
	imtmp.para[2] = 0xFFFF;
	imtmp.gotoIM = im_index;

	imdata.push_back(imtmp);
}

void IMMgr::assignAbsoluteAddr(unsigned index, unsigned addr) {
	imdata[index].absoluteAddr = addr;
}

void IMMgr::toTM(unsigned index, std::ostream &os) {
	IMData im;

	im = imdata[index];

	os << ((unsigned)im.absoluteAddr) << ":\t";
	if (im.ins == GOTO) {
		os << "LDC  7, " << ((unsigned)imdata[im.gotoIM].absoluteAddr) << "(0)\n" ;
		return;
	}
	switch (im.ins) {
	case HALT:
		os << "HALT ";
		break;
	case IN:
		os << "IN   ";
		break;
	case OUT:
		os << "OUT  ";
		break;
	case INB:
		os << "INB  ";
		break;
	case OUTB:
		os << "OUTB ";
		break;
	case OUTC:
		os << "OUTC ";
		break;
	case OUTNL:
		os << "OUTNL";
		break;
	case ADD:
		os << "ADD  ";
		break;
	case MUL:
		os << "MUL  ";
		break;
	case DIV:
		os << "DIV  ";
		break;
	case SUB:
		os << "SUB  ";
		break;
	case LDC:
		os << "LDC  ";
		break;
	case LDA:
		os << "LDA  ";
		break;
	case LD:
		os << "LD   ";
		break;
	case ST:
		os << "ST   ";
		break;
	case JLT:
		os << "JLT  ";
		break;
	case JLE:
		os << "JLE  ";
		break;
	case JEQ:
		os << "JEQ  ";
		break;
	case JNE:
		os << "JNE  ";
		break;
	case JGE:
		os << "JGE  ";
		break;
	case JGT:
		os << "JGT  ";
		break;
	default:
		assert(false);
	}
	switch (im.ins) {
	case HALT:
	case OUTNL:
		break;
	case IN:
	case OUT:
	case INB:
	case OUTB:
	case OUTC:
		os << ((int)im.para[0]);
		break;
	case ADD:
	case SUB:
	case MUL:
	case DIV:
		os << ((int)im.para[0]) << ", ";
		os << ((int)im.para[1]) << ", ";
		os << ((int)im.para[2]);
		break;
	case LDC:
	case LDA:
	case LD:
	case ST:
	case JLT:
	case JLE:
	case JEQ:
	case JNE:
	case JGE:
	case JGT:
		os << ((int)im.para[0]) << ", ";
		os << ((int)im.para[1]) << '(' << ((int)im.para[2]) <<')';
		break;
	default:
		assert(false);
	}
	os << '\n';
}
/*
class Block {
public:
	std::vector<unsigned> im;
	void add(unsigned i);
	unsigned entrance();
	void toTM(std::ostream&);
	unsigned assignAbsoluteAddr(unsigned Offset); // return the next available ins slot
};
*/

void Block::add(unsigned i) {
	im.push_back(i);
}

unsigned Block::entrance() {
	assert(!im.empty());
	return im[0];
}

void Block::toTM(std::ostream &os) {
	unsigned i,max;
	max = im.size();

	for(i = 0; i < max; i++) {
		imMgr.toTM(im[i], os);
	}
}

unsigned Block::assignAbsoluteAddr(unsigned Offset) {
	unsigned i,max;
	max = im.size();

	for(i = 0; i < max; i++) {
		imMgr.assignAbsoluteAddr(im[i], Offset + i);
	}

	return Offset + i;
}
/*
class ProcBlock {
public:
	std::vector<Block> blocks;
	unsigned entrance();
	void add(Block b);
	void toTM(std::ostream&);
	unsigned assignAbsoluteAddr(unsigned Offset);
};
*/

unsigned ProcBlock::entrance() {
	assert(!blocks.empty());
	return blocks[0].entrance();
}

void ProcBlock::add(Block b) {
	blocks.push_back(b);
}

void ProcBlock::toTM(std::ostream &os) {
	unsigned i,max;
	max = blocks.size();

	for(i = 0; i < max; i++) {
		blocks[i].toTM(os);
	}
}

unsigned ProcBlock::assignAbsoluteAddr(unsigned Offset) {
	unsigned i,max;
	max = blocks.size();

	for(i = 0; i < max; i++) {
		Offset = blocks[i].assignAbsoluteAddr(Offset);
	}

	return Offset;
}
