#include <cassert>
#include <map>
#include <iostream>
#include <string>
#include <string>
#include "semantic.h"
#include "memmgr.h"
#include "instruct.h"
#include "constStr.h"
#include "AST2TM.h"

std::map<std::string, ProcBlock> procBlocks;
std::vector<Block> gBlocks;
std::vector<unsigned> currentBreakTarget;

unsigned buildBlock(ARMgr &, SemanticNode, std::vector<Block> &);

void AST2TM(std::ostream &os) {
	typename std::map<std::string, ProcBlock>::iterator iter;
	SemanticTree tr;
	unsigned i, maxi, addr, en, exitIM;
	Block emptyBlock, tmpBlock;
	GlobalARMgr gARMgr;

	buildConstTable();

	tmpBlock = emptyBlock;
	tmpBlock.add(imMgr.newIM(LDC, 6, ARhead(), 0));
	tmpBlock.add(imMgr.newIM(LDC, 5, 0, 0));
	tmpBlock.add(imMgr.newIM(LD , 5, 0, 5));
	exitIM = imMgr.newIM(HALT,0, 0, 0);

	en = buildMain(gARMgr, tr, gBlocks, exitIM);
	tmpBlock.add(imMgr.newIM(LDA,4,2,7));
	tmpBlock.add(imMgr.newIM(ST,4,gARMgr.savedRegOffset()+7,6));

	tmpBlock.add(imMgr.newIM(en));
	tmpBlock.add(exitIM);
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

void buildProcBlock(SemanticNode nd) {
	;
}

void attachStmsBlock(ARMgr & arMgr, SemanticNode nd, Block &currentBlock, std::vector<Block> &currentBlockVector) {
	currentBlock.add(imMgr.newIM(LDA, 4, 0, 4));
	unsigned i, max, ifExit;

	max = nd.getChildCount(ASTN_stm);

	for( i = 0; i < max; i++) {
		SemanticNode sn2,sn3;
		unsigned j,k;
		sn2 = nd.getChild(ASTN_stm, i).getChild(0);
		switch(sn2.type()) {
		case ASTN_if:
			ifExit = imMgr.newIM(LDA,4 ,0 , 4);
			k = sn2.getChildCount(ASTN_branch);
			for( j = 0; j < k; j++) {
				sn3 = sn2.getChild(ASTN_branch, j); //each branch
				if (sn3.getChildCount(ASTN_exp) != 0) {
					attachExpIM(arMgr, sn3.getChild(ASTN_exp, 0), currentBlock, currentBlockVector);
					
					Block branchBlock;
					branchBlock.add(imMgr.newIM(LDA,4,0,4));
					currentBlock.add(imMgr.newIM(LD, 4, arMgr.lookupExp(sn3.getChild_exp, 0),6));
					arMgr.freeTmp();
					currentBlcok.add(imMgr.newIM(JEQ,4,1,7));
					currentBlock.add(imMgr.newIM(branchBlock.entrance()));
					attachStmsBlock(arMgr, sn3.getChild(ASTN_stms,0),branchBlock,currentBlockVector);
					branchBlock.add(imMgr.newIM(ifExit));
					currentBlockVector.push_back(branchBlock);
				}
				else {
					attachStmsBlocks(arMgr, sn3.getChild(ASTN_stms, 0), currentBlock, currentBlockVector);
				}
			}
			currentBlock.add(ifExit);
			break;
		case ASTN_fa:
		case ASTN_do:
			attachLoopIM(arMgr,SemanticNode sn2, currentBlock, currentBlockVector);
			break;
		default: 
			attachStmIM(arMgr,SemanticNode sn2, currentBlock, currentBlockVector);
			break;
		}
	}
}

void attachStmIM(ARMgr & arMgr, SemanticNode nd, Block &currentBlock, std::vector<Block> &currentBlockVector) {
	if (nd.getChildCount() == 0) {
		currentBlock.add(imMgr.newIM(LDA,4,0,4));
		return;
	}
	SemanticNode sn, lvl;
	unsigned i,j,max,k;
	sn = nd.getChild(0);
	switch(sn.type()) {
	case ASTN_exp:
		attachExpIM(arMgr, sn, currentBlock, currentBlockVector);
		arMgr.freeTmp();
		break;
	case ASTN_break:
		currentBlock.add(imMgr.newIM(currentBreakTarget[currentBreakTarget.size() - 1]));
		break;
	case ASTN_exit:
		currentBlock.add(imMgr.newIM(HALT,0,0,0));
		break;
	case ASTN_return:
		currentBlock.add(imMgr.newIM(LD, 7,arMgr.savedRegOffset() + 7, 6));
		break;
	case ASTN_write:
	case ASTN_writes:
		attachExpIM(arMgr, sn.getChild(ASTN_exp,0), currentBlock, currentBlockVector);
		currentBlock.add(imMgr.newIM(LD, 0, arMgr.lookupExp(sn.getChild(ASTN_exp, 0)), 6));
		arMgr.freeTmp();
		if (sn.reload() == ice9int) {
			currentBlock.add(imMgr.newIM(OUT,0,0,0));
		}
		else {
			currentBlock.add(imMgr.newIM(LD , 1,0,0));
			currentBlock.add(imMgr.newIM(ADD, 1,1,0));
			currentBlock.add(imMgr.newIM(LDA, 0,1,0));

			unsigned label,ex;
			ex = imMgr.newIM(LDA, 0, 0, 0);
			label = imMgr.newIM(SUB, 2, 1, 0);
			currentBlock.add(label);
			currentBlock.add(imMgr.newIM(JGE , 2, 1, 7));
			currentBlock.add(imMgr.newIM(ex));
			currentBlock.add(imMgr.newIM(LD , 3, 0, 0));
			currentBlock.add(imMgr.newIM(OUTC, 3, 0, 0));
			currentBlock.add(imMgr.newIM(LDA,0,1,0));
			currentBlock.add(imMgr.newIM(label));
			currentBlock.add(ex);
		}
		if (sn.type() == ASTN_writes) {
			currentBlock(imMgr.newIM(OUTNL,0,0,0));
		}
		break;
	case ASTN_assign: {
		std::vector<unsigned> v;
		lvl = sn.getChild(ASTN_lvalue, 0);
		v = varJumper.lookupDim(lvl.getChild(ASTN_L_id, 0).idValue());
		currentBlock.add(imMgr.newIM(LDA, 4, arMgr.lookupVar(lvl.getChild(ASTN_L_id, 0).idValue()),6));
		unsigned i,j,max,k,size;
		size = 1;
		max = v.size();
		for (i = 0; i < max; i++) {
			size *= v[i];
		}
		max = lvl.getChildCount(ASTN_exp);
		unsigned of;
		of = arMgr.insert(lvl);
		for (i = 0; i < max; i++) {
			size /= v[i];
			currentBlock.add(imMgr.newIM(ST,4,of,6));
			attachExpIM(arMgr, lvl.getChild(ASTN_exp, i), currentBlock, currentBlockVector);
			currentBlock.add(imMgr.newIM(LD, 4, of, 6));
			currentBlock.add(imMgr.newIM(LD, 3, arMgr.lookupExp(lvl.getChild(ASTN_exp, i)), 6));
			currentBlock.add(imMgr.newIM(LDC,2,size,0));
			currentBlock.add(imMgr.newIM(MUL,3,3,2));
			currentBlock.add(imMgr.newIM(ADD,4,4,3));
		}
		currentBlock.add(imMgr.newIM(ST,4,of,6));
		attachExpIM(arMgr,sn.getChild(ASTN_exp, 0), currentBlock, currentBlockVector);
		currentBlock.add(imMgr.newIM(LD, 0, arMgr.lookupExp(sn.getChild(ASTN_exp, 0)), 6));
		currentBlock.add(imMgr.newIM(LD, 4, of, 6));
		arMgr.freeTmp();
		currentBlock.add(imMgr.newIM(ST,0,0,4));
		break;
	}
	}
}

void attachLoopIM(ARMgr & arMgr, SemanticNode nd, Block &currentBlock, std::vector<Block> &currentBlockVector) {
	unsigned loopExit, faOffset;
	Block loopBlock;
	loopBlock.add(imMgr.newIM(LDA,4,0,4));
	loopExit = imMgr.newIM(LDA,4,0,4);
	currentBreakTarget.push_back(loopExit);
	switch(nd.type()) {
	case ASTN_do:
		attachExpIM(arMgr, nd.getChild(ASTN_exp, 0), loopBlock, currentBlockVector);
		loopBlock.add(imMgr.newIM(LD,4,arMgr.lookupExp(nd.getChild(ASTN_exp, 0)),6));
		arMgr.freeTmp();
		loopBlock.add(imMgr.newIM(JNE,4,1,7));
		loopBlock.add(imMgr.newIM(loopExit));
		if(nd.getChildCount(ASTN_stms) >= 0) {
			attachStmsBlock(arMgr, nd.getChild(ASTN_stms, 0), loopBlock, currentBlockVector);
		}
		loopBlock.add(imMgr.newIM(loopBlock.entrance()));
		break;
	case ASTN_fa:
		faOffset = arMgr.pushFa(nd.getChild(ASTN_L_id, 0).idValue());
		attachExpIM(arMgr, nd.getChild(ASTN_exp, 0), currentBlock,currentBlockVector);
		currentBlock.add(imMgr.newIM(LD, 3, arMgr.lookupExp(nd.getChild(ASTN_exp), 0), 6));
		arMgr.freeTmp();
		attachExpIM(arMgr, nd.getChild(ASTN_exp, 1), currentBlock,currentBlockVector);
		currentBlock.add(imMgr.newIM(LD, 4, arMgr.lookupExp(nd.getChild(ASTN_exp), 1), 6));
		arMgr.freeTmp();
		currentBlock.add(imMgr.newIM(ST, 3, -(int)faOffset, 5));
		currentBlock.add(imMgr.newIM(ST, 4, -(int)(faOffset + 1), 5));
		currentBlock.add(imMgr.newIM(loopBlock.entrance()));
		loopBlock.add(imMgr.newIM(LD, 3, -(int)faOffset, 5));
		loopBlock.add(imMgr.newIM(LD, 4, -(int)(faOffset + 1), 5));
		loopBlock.add(imMgr.newIM(SUB,0,4,3));
		loopBlock.add(imMgr.newIM(JGE,0,1,7));
		loopBlock.add(imMgr.newIM(loopExit));
		if(nd.getChildCount(ASTN_stms) >= 0) {
			attachStmsBlock(arMgr, nd.getChild(ASTN_stms, 0), loopBlock, currentBlockVector);
		}
		loopBlock.add(imMgr.newIM(LD, 3, -(int)faOffset, 5));
		loopBlock.add(imMgr.newIM(LDA,3, 1, 3));
		loopBlock.add(imMgr.newIM(ST, 3, -(int)faOffset, 5));
		loopBlock.add(imMgr.newIM(loopBlock.entrance()));
		arMgr.popFa();
		break;
	}
	currentBreakTarget.pop_back();
	currentBlockVector.push_back(loopBlock);
	currentBlock.add(loopExit);
}

void attachExpIM(ARMgr &arMgr, SemanticNode nd, Block &currentBlock, std::vector<Block> &currentBlockVector) {
	assert(nd.type() == ASTN_exp);
	SemanticNode sn;
	MemOffset m;
	sn = nd.getChild(0);
	m = arMgr.insertExp(nd);
	switch(sn.type()) {
	case ASTN_lvalue: 
		if (arMgr.isFa(sn.getChild(ASTN_L_id))) {
			currentBlock.add(imMgr.newIM(LD, 4,));
		}
		else {
			std::vector<unsigned> v;
			v = varJumper.lookupDim(sn.getChild(ASTN_L_id, 0).idValue());
			currentBlock.add(imMgr.newIM(LDA, 4, arMgr.lookupVar(sn.getChild(ASTN_L_id, 0).idValue()),6));
			unsigned i,j,max,k,size;
			size = 1;
			max = v.size();
			for (i = 0; i < max; i++) {
				size *= v[i];
			}
			of = arMgr.insert(sn);
			max = sn.getChildCount(ASTN_exp);
			for (i = 0; i < max; i++) {
				size /= v[i];
				currentBlock.add(imMgr.newIM(ST,4,of,6));
				attachExpIM(arMgr, sn.getChild(ASTN_exp, i), currentBlock, currentBlockVector);
				currentBlock.add(imMgr.newIM(LD,4,of,6));
				currentBlock.add(imMgr.newIM(LD, 3, arMgr.lookupExp(sn.getChild(ASTN_exp, i)), 6));
				currentBlock.add(imMgr.newIM(LDC,2,size,0));
				currentBlock.add(imMgr.newIM(MUL,3,3,2));
				currentBlock.add(imMgr.newIM(ADD,4,4,3));
			}
		}
		currentBlock.add(imMgr.newIM(LD,4,0,4));
		currentBlock.add(imMgr.newIM(ST,4,m,6));
		break;
	case ASTN_L_int:
		currentBlock.add(imMgr.newIM(LDC,0,sn.intValue(),0));
		currentBlock.add(imMgr.newIM(ST,0,m,6));
		break;
	case ASTN_L_bool:
		currentBlock.add(imMgr.newIM(LDC,0,sn.boolValue()?1:0,0));
		currentBlock.add(imMgr.newIM(ST,0,m,6));
		break;
	case ASTN_L_string:
		currentBlock.add(imMgr.newIM(LDC,0,lookupStr(sn.strValue()),0));
		currentBlock.add(imMgr.newIM(ST,0,m,6));
		break;
	case ASTN_umin:
		attachExpIM(arMgr,sn.getChild(ASTN_exp,0),currentBlock, currentBlockVector);
		currentBlock.add(imMgr.newIM(LD ,1,arMgr.lookupExp(sn.getChild(ASTN_exp,0)),0));
		if(sn.reload() == ice9int) {
			currentBlock.add(imMgr.newIM(LDC,0,0,0));
			currentBlock.add(imMgr.newIM(SUB,0,0,1));
		}
		else {
			currentBlock.add(imMgr.newIM(LDC,0,1,0));
			currentBlock.add(imMgr.newIM(JEQ,1,1,7));
			currentBlock.add(imMgr.newIM(LDC,0,0,0));
		}
		currentBlock.add(imMgr.newIM(ST,0,m,6));
		break;
	case ASTN_quest:
		attachExpIM(arMgr,sn.getChild(ASTN_exp,0),currentBlock, currentBlockVector);
		currentBlock.add(imMgr.newIM(LD ,1,arMgr.lookupExp(sn.getChild(ASTN_exp,0)),0));
		currentBlock.add(imMgr.newIM(ST,1,m,6));
		break;
	case ASTN_proccall:{
		ProcBlock &pb = procBlocks[sn.getChild(ASTN_L_id,0).idValue()];
		unsigned i,max,j,k,newAROffset;
		max = sn.getChildCount(ASTN_exp);
		for(i = 0; i < max; i++) {
			attachExpIM(arMgr, sn.getChild(ASTN_exp,i), currentBlock, currentBlockVector);
		}
		newAROffset = arMgr.length();
		currentBlock.add(imMgr.newIM(LDA,0,newAROffset,6));//reg[6] is base addr of caller AR, reg[0] is base addr of callee AR
		for(i = 0; i < max; i++) {
			currentBlock.add(imMgr.newIM(LD,2,arMgr.lookupExp(sn.getChild(ASTN_exp,i)),6));
			currentBlock.add(imMgr.newIM(ST,2,pb.arMgr.parametersOffset() + i,0));
		}
		for(i = 0; i < 7; i++) {
			currentBlock.add(imMgr.newIM(ST ,i,pb.arMgr.savedRegOffset() + i,0));
		}
		currentBlock.add(imMgr.newIM(LDA,5,arMgr.currentForTop(),5));
		currentBlock.add(imMgr.newIM(LDA,6,0,0));
		currentBlock.add(imMgr.newIM(LDA,2,2,7));
		currentBlock.add(imMgr.newIM(ST ,2,pb.arMgr.savedRegOffset() + 7,0));
		currentBlock.add(imMgr.newIM(pb.entrance()));
		for(i = 0; i < 7; i++) {
			currentBlock.add(imMgr.newIM(LD ,i,pb.arMgr.savedRegOffset() + i,6)); //when return, reg[6] is base addr of callee AR
		}
		//after LD reg[6] will recover and become base addr of caller AR
		//and reg[0] will become callee AR
		currentBlock.add(imMgr.newIM(LD,4,pb.arMgr.returnValueOffset(),0));
		currentBlock.add(imMgr.newIM(ST,4,m,6));
		break;
	}
	case ASTN_plus:{
		attachExpIM(arMgr,sn.getChild(ASTN_exp,0),currentBlock, currentBlockVector);
		unsigned ex;
		ex = imMgr.newIM(ST,3,m,6);
		if (sn.reload() == ice9int) {
			attachExpIM(arMgr,sn.getChild(ASTN_exp,1),currentBlock, currentBlockVector);
			currentBlock.add(imMgr.newIM(LD,0,arMgr.lookupExp(sn.getChild(ASTN_exp,0)),6));
			currentBlock.add(imMgr.newIM(LD,1,arMgr.lookupExp(sn.getChild(ASTN_exp,1)),6));
			currentBlock.add(imMgr.newIM(ADD,3,1,0));
		}
		else { //short cut is used
			currentBlock.add(imMgr.newIM(LD,0,arMgr.lookupExp(sn.getChild(ASTN_exp,0)),6));
			currentBlock.add(imMgr.newIM(JEQ,0,2,7));
			currentBlock.add(imMgr.newIM(LDC,3,1,0));
			currentBlock.add(imMgr.newIM(ex));
			attachExpIM(arMgr,sn.getChild(ASTN_exp,1),currentBlock, currentBlockVector);
			currentBlock.add(imMgr.newIM(LD,0,arMgr.lookupExp(sn.getChild(ASTN_exp,1)),6));
			currentBlock.add(imMgr.newIM(JEQ,0,2,7));
			currentBlock.add(imMgr.newIM(LDC,3,1,0));
			currentBlock.add(imMgr.newIM(ex));
			currentBlock.add(imMgr.newIM(LDC,3,0,0));
		}
		currentBlock.add(ex);
		break;
	}
	case ASTN_minus:
		attachExpIM(arMgr,sn.getChild(ASTN_exp,0),currentBlock, currentBlockVector);
		attachExpIM(arMgr,sn.getChild(ASTN_exp,1),currentBlock, currentBlockVector);
		currentBlock.add(imMgr.newIM(LD,0,arMgr.lookupExp(sn.getChild(ASTN_exp,0)),6));
		currentBlock.add(imMgr.newIM(LD,1,arMgr.lookupExp(sn.getChild(ASTN_exp,1)),6));
		currentBlock.add(imMgr.newIM(SUB,2,0,1));
		currentBlock.add(imMgr.newIM(ST,2,m,6));
		break;
	case ASTN_mod:
		attachExpIM(arMgr,sn.getChild(ASTN_exp,0),currentBlock, currentBlockVector);
		attachExpIM(arMgr,sn.getChild(ASTN_exp,1),currentBlock, currentBlockVector);
		currentBlock.add(imMgr.newIM(DIV,2,0,1));
		currentBlock.add(imMgr.newIM(MUL,3,2,1));
		currentBlock.add(imMgr.newIM(SUB,3,0,3));
		currentBlock.add(imMgr.newIM(ST ,3,m,6));
		break;
	case ASTN_star:{
		attachExpIM(arMgr,sn.getChild(ASTN_exp,0),currentBlock, currentBlockVector);
		unsigned ex;
		ex = imMgr.newIM(ST,3,m,6);
		if (sn.reload() == ice9int) {
			attachExpIM(arMgr,sn.getChild(ASTN_exp,1),currentBlock, currentBlockVector);
			currentBlock.add(imMgr.newIM(LD,0,arMgr.lookupExp(sn.getChild(ASTN_exp,0)),6));
			currentBlock.add(imMgr.newIM(LD,1,arMgr.lookupExp(sn.getChild(ASTN_exp,1)),6));
			currentBlock.add(imMgr.newIM(MUL,3,1,0));
		}
		else { //short cut is used
			currentBlock.add(imMgr.newIM(LD,0,arMgr.lookupExp(sn.getChild(ASTN_exp,0)),6));
			currentBlock.add(imMgr.newIM(JNE,0,2,7));
			currentBlock.add(imMgr.newIM(LDC,3,0,0));
			currentBlock.add(imMgr.newIM(ex));
			attachExpIM(arMgr,sn.getChild(ASTN_exp,1),currentBlock, currentBlockVector);
			currentBlock.add(imMgr.newIM(LD,0,arMgr.lookupExp(sn.getChild(ASTN_exp,1)),6));
			currentBlock.add(imMgr.newIM(JNE,0,2,7));
			currentBlock.add(imMgr.newIM(LDC,3,0,0));
			currentBlock.add(imMgr.newIM(ex));
			currentBlock.add(imMgr.newIM(LDC,3,1,0));
		}
		currentBlock.add(ex);
		break;
	}
	case ASTN_div:
		attachExpIM(arMgr,sn.getChild(ASTN_exp,0),currentBlock, currentBlockVector);
		attachExpIM(arMgr,sn.getChild(ASTN_exp,1),currentBlock, currentBlockVector);
		currentBlock.add(imMgr.newIM(LD,0,arMgr.lookupExp(sn.getChild(ASTN_exp,0)),6));
		currentBlock.add(imMgr.newIM(LD,1,arMgr.lookupExp(sn.getChild(ASTN_exp,1)),6));
		currentBlock.add(imMgr.newIM(DIV,2,0,1));
		currentBlock.add(imMgr.newIM(ST,2,m,6));
		break;
	case ASTN_eq:
		attachExpIM(arMgr,sn.getChild(ASTN_exp,0),currentBlock, currentBlockVector);
		attachExpIM(arMgr,sn.getChild(ASTN_exp,1),currentBlock, currentBlockVector);
		currentBlock.add(imMgr.newIM(LD,0,arMgr.lookupExp(sn.getChild(ASTN_exp,0)),6));
		currentBlock.add(imMgr.newIM(LD,1,arMgr.lookupExp(sn.getChild(ASTN_exp,1)),6));
		currentBlock.add(imMgr.newIM(SUB,2,0,1));
		currentBlock.add(imMgr.newIM(LDC,3,1,0));
		currentBlock.add(imMgr.newIM(JEQ,2,1,7));
		currentBlock.add(imMgr.newIM(LDC,3,0,0));
		currentBlock.add(imMgr.newIM(ST,3,m,6));
		break;
	case ASTN_neq:
		attachExpIM(arMgr,sn.getChild(ASTN_exp,0),currentBlock, currentBlockVector);
		attachExpIM(arMgr,sn.getChild(ASTN_exp,1),currentBlock, currentBlockVector);
		currentBlock.add(imMgr.newIM(LD,0,arMgr.lookupExp(sn.getChild(ASTN_exp,0)),6));
		currentBlock.add(imMgr.newIM(LD,1,arMgr.lookupExp(sn.getChild(ASTN_exp,1)),6));
		currentBlock.add(imMgr.newIM(SUB,2,0,1));
		currentBlock.add(imMgr.newIM(LDC,3,0,0));
		currentBlock.add(imMgr.newIM(JEQ,2,1,7));
		currentBlock.add(imMgr.newIM(LDC,3,1,0));
		currentBlock.add(imMgr.newIM(ST,3,m,6));
		break;
	case ASTN_gt:
		attachExpIM(arMgr,sn.getChild(ASTN_exp,0),currentBlock, currentBlockVector);
		attachExpIM(arMgr,sn.getChild(ASTN_exp,1),currentBlock, currentBlockVector);
		currentBlock.add(imMgr.newIM(LD,0,arMgr.lookupExp(sn.getChild(ASTN_exp,0)),6));
		currentBlock.add(imMgr.newIM(LD,1,arMgr.lookupExp(sn.getChild(ASTN_exp,1)),6));
		currentBlock.add(imMgr.newIM(SUB,2,0,1));
		currentBlock.add(imMgr.newIM(LDC,3,1,0));
		currentBlock.add(imMgr.newIM(JGT,2,1,7));
		currentBlock.add(imMgr.newIM(LDC,3,0,0));
		currentBlock.add(imMgr.newIM(ST,3,m,6));
		break;
	case ASTN_ge:
		attachExpIM(arMgr,sn.getChild(ASTN_exp,0),currentBlock, currentBlockVector);
		attachExpIM(arMgr,sn.getChild(ASTN_exp,1),currentBlock, currentBlockVector);
		currentBlock.add(imMgr.newIM(LD,0,arMgr.lookupExp(sn.getChild(ASTN_exp,0)),6));
		currentBlock.add(imMgr.newIM(LD,1,arMgr.lookupExp(sn.getChild(ASTN_exp,1)),6));
		currentBlock.add(imMgr.newIM(SUB,2,0,1));
		currentBlock.add(imMgr.newIM(LDC,3,1,0));
		currentBlock.add(imMgr.newIM(JGE,2,1,7));
		currentBlock.add(imMgr.newIM(LDC,3,0,0));
		currentBlock.add(imMgr.newIM(ST,3,m,6));
		break;
	case ASTN_lt:
		attachExpIM(arMgr,sn.getChild(ASTN_exp,0),currentBlock, currentBlockVector);
		attachExpIM(arMgr,sn.getChild(ASTN_exp,1),currentBlock, currentBlockVector);
		currentBlock.add(imMgr.newIM(LD,0,arMgr.lookupExp(sn.getChild(ASTN_exp,0)),6));
		currentBlock.add(imMgr.newIM(LD,1,arMgr.lookupExp(sn.getChild(ASTN_exp,1)),6));
		currentBlock.add(imMgr.newIM(SUB,2,0,1));
		currentBlock.add(imMgr.newIM(LDC,3,1,0));
		currentBlock.add(imMgr.newIM(JLT,2,1,7));
		currentBlock.add(imMgr.newIM(LDC,3,0,0));
		currentBlock.add(imMgr.newIM(ST,3,m,6));
		break;
	case ASTN_le:
		attachExpIM(arMgr,sn.getChild(ASTN_exp,0),currentBlock, currentBlockVector);
		attachExpIM(arMgr,sn.getChild(ASTN_exp,1),currentBlock, currentBlockVector);
		currentBlock.add(imMgr.newIM(LD,0,arMgr.lookupExp(sn.getChild(ASTN_exp,0)),6));
		currentBlock.add(imMgr.newIM(LD,1,arMgr.lookupExp(sn.getChild(ASTN_exp,1)),6));
		currentBlock.add(imMgr.newIM(SUB,2,0,1));
		currentBlock.add(imMgr.newIM(LDC,3,1,0));
		currentBlock.add(imMgr.newIM(JLE,2,1,7));
		currentBlock.add(imMgr.newIM(LDC,3,0,0));
		currentBlock.add(imMgr.newIM(ST,3,m,6));
		break;
	}
}

std::vector<unsigned> typeChecker(std::string currentProc,SemanticNode varlistnd) {
	char *tpName;
	std::vector<unsigned> ret, baseTypeDim;
	SemanticNode typedesnd;
	unsigned i, max;
	typedesnd = varlistamMgrnd.getChild(typedesnd, 0);
	max = typedesnd.getChildCount(ASTN_L_int);
	for (i = 0; i < max; i++) {
		//for each dim
		ret.push_back(typedesnd.getChild(ASTN_L_int, i).intValue());
	}
	
	tpName = typedesnd.getChild(ASTN_L_id, 0).idValue();
	if (strcmp(tpName, "int") != 0 && strcmp(tpName, "bool") !=0 && strcmp(tpName, "string") != 0) {
		// not basic type
		baseTypeDim = typeJumper.lookupDim(currentProc, tpName);
		max = baseTypeDim.size();
		for(i = 0; i < max; i++) {
			ret.push_back(baseTypeDim[i]);
		}
	}
	return ret;
}

unsigned buildMain(ARMgr & arMgr, SemanticNode nd, std::vector<Block> & currentBlockVector, unsigned exitIM) {
	Block tmpBlock;
	unsigned i,j,max,k,a,b;

	if (nd.getChildCount(ASTN_decs) != 0) {
		SemanticNode decsnd;
		decsnd = nd.getChild(ASTN_decs, 0);
		max = decsnd.getChildCount();

		for(i = 0; i < max; i++) {
			SemanticNode sn;
			sn = decsnd.getChild(i);
			switch(sn.type()) {
			case ASTN_var:
				k = sn.getChildCount();
				for(j = 0; j < k; j++) {  //each varlist
					std::vector<unsigned> dim;
					SemanticNode sn1;
					sn1 = sn.getChild(ASTN_varlist, j);
					dim = typeChecker("0", sn1);
					b = sn1.getChild(ASTN_idlist).getChildCount(id);
					for(a = 0; a < b; a++) {	//each var
						char *varName;         //update AR structure
						varName = sn1.getChild(ASTN_idlist).getChild(id, a);
						if(dim.empty() || dim.size() == 0) {
							arMgr.insert(varName);
						}
						else {
							arMgr.insertArray(varName,dim);
						}
					}
				}
				break;
			case ASTN_proc:
				buildProcBlock(sn);
				break;
			case ASTN_forward:
				buildProcBlock(sn);
				break;
			}
		}
	}
	if (nd.getChildCount(ASTN_stms) == 0){
		tmpBlock.add(imMgr.newIM(LDA, 4, 0, 4));
	}
	else {
		attachStmsBlock(arMgr, nd.getChild(ASTN_stms, 0), tmpBlock, currentBlockVector);
	}
	tmpBlock.add(exitIM);
	currentBlockVector.push_back(tmpBlock);
	return tmpBlock.entrance();
}
