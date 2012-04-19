#include <cassert>
#include <map>
#include <iostream>
#include <string>
#include "semantic.h"
#include "memmgr.h"
#include "instruct.h"
#include "constStr.h"
#include "AST2TM.h"

std::map<std::string, ProcBlock> procBlocks;
std::vector<Block> gBlocks;

unsigned buildBlock(AMMgr &, SemanticNode, std::vector<Block> &);

void AST2TM(std::ostream &os) {
	typename std::map<std::string, ProcBlock>::iterator iter;
	SemanticTree tr;
	unsigned i, maxi, addr, en;
	Block emptyBlock, tmpBlock;
	GlobalARMgr gARMgr;

	buildConstTable();

	tmpBlock = emptyBlock;
	tmpBlock.add(imMgr.newIM(LDC, 6, ARhead(), 0));
	tmpBlock.add(imMgr.newIM(LDC, 5, 0, 0));
	tmpBlock.add(imMgr.newIM(LD , 5, 0, 5));

	en = buildBlock(gARMgr, tr, gBlocks);

	tmpBlock.add(imMgr.newIM(en));
	tmpBlock.add(imMgr.newIM(HALT,0, 0, 0));
	gBlocks.push_back(tmpBlock);

	max = gBlocks.size();
	addr = 0;
	for (i = 0; i < max; i++) {
		addr = gBlocks[i].assignAbsoluteAddr(addr);
	}
	for (iter = procBlocks.begin(); iter != procBlocks.end(); iter++) {
		addr = (*iter).assignAbsoluteAddr(addr);
	}

	constTable2TM(os);
	max = gBlocks.size();
	for (i = 0; i < max; i++) {
		gBlocks[i].toTM(os);
	}
	for (iter = procBlocks.begin(); iter != procBlocks.end(); iter++) {
		(*iter).toTM(os);
	}
}


unsigned buildBlock(AMMgr & amMgr, SemanticNode nd, std::vector<Block> & currentBlockVector) {
	Block tmpBlock;

	currentBlockVector.push_back(tmpBlock);
	return tmpBlock.entrance();
}
