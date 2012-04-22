#include <cassert>
#include <map>
#include <iostream>
#include <string>
#include <string>
#include <sstream>
#include "semantic.h"
#include "memmgr.h"
#include "instruct.h"
#include "constStr.h"
#include "typejumper.h"
#include "AST2TM.h"
#define CPN (pN(currentProcName))

std::map<std::string, ProcBlock> procBlocks;
std::vector<Block> gBlocks;
std::vector<unsigned> currentBreakTarget;
std::string currentProcName;

inline std::string pN(std::string n) {
	return (n == "0")?"main":n;
}

inline std::string nd2line(SemanticNode nd) {
	std::stringstream ss;
	ss << nd.line();
	return ss.str();
}

void attachStmIM(ARMgr & arMgr, SemanticNode nd, Block &currentBlock, std::vector<Block> &currentBlockVector);
void attachLoopIM(ARMgr & arMgr, SemanticNode nd, Block &currentBlock, std::vector<Block> &currentBlockVector);


void attachStmsBlock(ARMgr & arMgr, SemanticNode nd, Block &currentBlock, std::vector<Block> &currentBlockVector);
std::vector<unsigned> typeChecker(std::string currentProc,SemanticNode varlistnd);
void attachExpIM(ARMgr &arMgr, SemanticNode nd, Block &currentBlock, std::vector<Block> &currentBlockVector);

void buildProcBlock(SemanticNode nd) {
	Block tmpBlock,entranceBlock;
	unsigned i,j,max,k,a,b;

	if (nd.type() == ASTN_forward) {
		ProcBlock &pb = procBlocks[nd.getChild(ASTN_L_id, 0).idValue()];
		tmpBlock.add(imMgr.newIM(LDA,5,0,5,CPN+"(b)@"+nd2line(nd)+" NOOP, entrance of proc, build by forward node"));
		pb.add(tmpBlock);
		pb.arMgr.forceInsert(nd.getChild(ASTN_L_id,0).idValue(), 0);
		if (nd.getChildCount(ASTN_declistx) != 0) {
			SemanticNode declistxnd;
			unsigned i,j,max,k;
			declistxnd = nd.getChild(ASTN_declistx, 0);
			max = declistxnd.getChildCount(ASTN_declist);
			for ( i = 0; i < max; i++) {// for each declist
				SemanticNode idlistnd;
				idlistnd = declistxnd.getChild(ASTN_declist, i).getChild(ASTN_idlist,0);
				k = idlistnd.getChildCount(ASTN_L_id);
				for(j = 0; j < k; j++) {
					pb.arMgr.pushPara(idlistnd.getChild(ASTN_L_id, j).idValue());
				}
			}
			
		}
		return;
	}

	//if proc node:
	if (procBlocks.find(nd.getChild(ASTN_L_id,0).idValue()) == procBlocks.end()) { // if no forward, build AR structure
		ProcBlock &pb = procBlocks[nd.getChild(ASTN_L_id, 0).idValue()];
		pb.arMgr.forceInsert(nd.getChild(ASTN_L_id,0).idValue(), 0);
		if (nd.getChildCount(ASTN_declistx) != 0) {
			SemanticNode declistxnd;
			unsigned i,j,max,k;
			declistxnd = nd.getChild(ASTN_declistx, 0);
			max = declistxnd.getChildCount(ASTN_declist);
			for ( i = 0; i < max; i++) {// for each declist
				SemanticNode idlistnd;
				idlistnd = declistxnd.getChild(ASTN_declist, i).getChild(ASTN_idlist,0);
				k = idlistnd.getChildCount(ASTN_L_id);
				for(j = 0; j < k; j++) {
					pb.arMgr.pushPara(idlistnd.getChild(ASTN_L_id, j).idValue());
				}
			}
			
		}
	}

	ProcBlock &pb = procBlocks[nd.getChild(ASTN_L_id,0).idValue()];
	currentProcName = nd.getChild(ASTN_L_id, 0).idValue();

	if (nd.getChildCount(ASTN_dec1s) != 0) {
		char *procName;
		SemanticNode decsnd;
		decsnd = nd.getChild(ASTN_dec1s, 0);
		max = decsnd.getChildCount(ASTN_varlist);
		procName = nd.getChild(ASTN_L_id,0).idValue();

		for(i = 0; i < max; i++) {
			SemanticNode sn1;
			std::vector<unsigned> dim;
			sn1 = decsnd.getChild(ASTN_varlist, i);
			dim = typeChecker(procName, sn1);
			b = sn1.getChild(ASTN_idlist, 0).getChildCount(ASTN_L_id);
			for(a = 0; a < b; a++) {	//each var
				char *varName;         //update AR structure
				varName = sn1.getChild(ASTN_idlist, 0).getChild(ASTN_L_id, a).idValue();
				if(dim.empty() || dim.size() == 0) {
					pb.arMgr.insert(varName);
				}
				else {
					pb.arMgr.insertArray(varName,dim);
				}
			}
		}
	}
	if (nd.getChildCount(ASTN_stms) == 0){
		tmpBlock.add(imMgr.newIM(LDA, 4, 0, 4,CPN+"(b)@"+nd2line(nd)+" NOOP, entrance of stms in proc"));
	}
	else {
		tmpBlock.add(imMgr.newIM(LDA, 4, 0, 4,CPN+"(b)@"+nd2line(nd.getChild(ASTN_stms,0))+" NOOP, entrance of stms in proc"));
		entranceBlock.add(imMgr.newIM(tmpBlock.entrance(),CPN+"(b)@"+nd2line(nd.getChild(ASTN_stms,0))+" goto entance of stms block (main body of proc)"));
		pb.add(entranceBlock);
		attachStmsBlock(pb.arMgr, nd.getChild(ASTN_stms, 0), tmpBlock, pb.blocks);
	}
	tmpBlock.add(imMgr.newIM(LD,7,pb.arMgr.savedRegOffset() + 7,6,CPN+"(b)@"+nd2line(nd.getChild(nd.getChildCount()))+" return, recover R7 from AR"));
	pb.add(tmpBlock);
	currentProcName = "0";
}

void attachStmsBlock(ARMgr & arMgr, SemanticNode nd, Block &currentBlock, std::vector<Block> &currentBlockVector) {
	currentBlock.add(imMgr.newIM(LDA, 4, 0, 4, CPN+"(b)@"+nd2line(nd)+" stms block entrance, this is a NOOP"));
	unsigned i, max, ifExit;

	max = nd.getChildCount(ASTN_stm);

	for( i = 0; i < max; i++) {
		SemanticNode sn2,sn3;
		unsigned j,k;
		sn2 = nd.getChild(ASTN_stm, i).getChild(0);
		switch(sn2.type()) {
		case ASTN_if:
			ifExit = imMgr.newIM(LDA,4 ,0 ,4, CPN+"(b)@"+nd2line(sn2)+" NOOP, exit of if.");
			k = sn2.getChildCount(ASTN_branch);
			for( j = 0; j < k; j++) {
				sn3 = sn2.getChild(ASTN_branch, j); //each branch
				if (sn3.getChildCount(ASTN_exp) != 0) {
					attachExpIM(arMgr, sn3.getChild(ASTN_exp, 0), currentBlock, currentBlockVector);
					
					Block branchBlock;
					branchBlock.add(imMgr.newIM(LDA,4,0,4,CPN+"(b)@"+nd2line(sn3)+" NOOP, branch entrance"));
					currentBlock.add(imMgr.newIM(LD, 4, arMgr.lookupExp(sn3.getChild(ASTN_exp, 0)),6,CPN+"(b)@"+nd2line(sn3)+" R4=value of branch condition"));
					arMgr.freeTmp();
					currentBlock.add(imMgr.newIM(JEQ,4,1,7,CPN+"(b)@"+nd2line(sn3)+" skip next if R4 == false"));
					currentBlock.add(imMgr.newIM(branchBlock.entrance(),CPN+"(b)@"+nd2line(sn3)+" goto branch stms"));
					attachStmsBlock(arMgr, sn3.getChild(ASTN_stms,0),branchBlock,currentBlockVector);
					branchBlock.add(imMgr.newIM(ifExit)),CPN+"(b)@"+nd2line(sn3)+" last ins of branch, go to exit of if stm";
					currentBlockVector.push_back(branchBlock);
				}
				else {
					attachStmsBlock(arMgr, sn3.getChild(ASTN_stms, 0), currentBlock, currentBlockVector);
				}
			}
			currentBlock.add(ifExit);
			break;
		case ASTN_fa:
		case ASTN_do:
			attachLoopIM(arMgr,sn2, currentBlock, currentBlockVector);
			break;
		default: 
			attachStmIM(arMgr,nd.getChild(ASTN_stm, i), currentBlock, currentBlockVector);
			break;
		}
	}
}

void attachStmIM(ARMgr & arMgr, SemanticNode nd, Block &currentBlock, std::vector<Block> &currentBlockVector) {
	if (nd.getChildCount() == 0) {
		currentBlock.add(imMgr.newIM(LDA,4,0,4,CPN+"(b)@"+nd2line(nd)+" NOOP, for empty stm"));
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
	case ASTN_L_break:
		currentBlock.add(imMgr.newIM(currentBreakTarget[currentBreakTarget.size() - 1],CPN+"(b)@"+nd2line(sn)+" break, goto last instruction of the inner most loop"));
		break;
	case ASTN_L_exit:
		currentBlock.add(imMgr.newIM(HALT,0,0,0,CPN+"(b)@"+nd2line(sn)+" exit, program ends"));
		break;
	case ASTN_L_return:
		currentBlock.add(imMgr.newIM(LD, 7,arMgr.savedRegOffset() + 7, 6,CPN+"(b)@"+nd2line(sn)+" return, recover R7 from AR"));
		break;
	case ASTN_write:
	case ASTN_writes:
		attachExpIM(arMgr, sn.getChild(ASTN_exp,0), currentBlock, currentBlockVector);
		currentBlock.add(imMgr.newIM(LD, 0, arMgr.lookupExp(sn.getChild(ASTN_exp, 0)), 6,CPN+"(b)@"+nd2line(sn)+" write(s), R0=value/pointer to be wrote"));
		arMgr.freeTmp();
		if (sn.reload() == ice9int) {
			currentBlock.add(imMgr.newIM(OUT,0,0,0,CPN+"(b)@"+nd2line(sn)+" write(s), Output the integer in R0"));
		}
		else {
			currentBlock.add(imMgr.newIM(LD , 1,0,0,CPN+"(b)@"+nd2line(sn)+" write(s), R1=dMem[R0], R0=ptr to str data, R1=string length"));
			currentBlock.add(imMgr.newIM(ADD, 1,1,0,CPN+"(b)@"+nd2line(sn)+" write(s), R1=ptr to last char"));
			currentBlock.add(imMgr.newIM(LDA, 0,1,0,CPN+"(b)@"+nd2line(sn)+" write(s), R0=ptr to first char"));

			unsigned label,ex;
			ex = imMgr.newIM(LDA, 0, 0, 0,CPN+"(b)@"+nd2line(sn)+" write(s), NOOP, exit of output char loop");
			label = imMgr.newIM(SUB, 2, 1, 0,CPN+"(b)@"+nd2line(sn)+" write(s), R2 = R1 - R0, remaining char to output, also entrance of output char loop");
			currentBlock.add(label);
			currentBlock.add(imMgr.newIM(JGE , 2, 1, 7,CPN+"(b)@"+nd2line(sn)+" write(s), if R2>0, still have char to output, skip next instruction"));
			currentBlock.add(imMgr.newIM(ex,CPN+"(b)@"+nd2line(sn)+" write(s), go to exit of output char loop"));
			currentBlock.add(imMgr.newIM(LD , 3, 0, 0,CPN+"(b)@"+nd2line(sn)+" write(s), R3=dMem[R0], load the outputing char to R3"));
			currentBlock.add(imMgr.newIM(OUTC, 3, 0, 0,CPN+"(b)@"+nd2line(sn)+" write(s), output R3"));
			currentBlock.add(imMgr.newIM(LDA,0,1,0,CPN+"(b)@"+nd2line(sn)+" write(s), INC R0, move pointer to next char"));
			currentBlock.add(imMgr.newIM(label,CPN+"(b)@"+nd2line(sn)+" write(s), go back to output loop entrance"));
			currentBlock.add(ex);
		}
		if (sn.type() == ASTN_writes) {
			currentBlock.add(imMgr.newIM(OUTNL,0,0,0,CPN+"(b)@"+nd2line(sn)+" writes, output a newline"));
		}
		break;
	case ASTN_assign: {
		std::vector<unsigned> v;
		lvl = sn.getChild(ASTN_lvalue, 0);
		v = varJumper.lookupDim(currentProcName, lvl.getChild(ASTN_L_id, 0).idValue());
		currentBlock.add(imMgr.newIM(LDA, 4, arMgr.lookupVar(lvl.getChild(ASTN_L_id, 0).idValue()),6,CPN+"(b)@"+nd2line(lvl)+" assign:LHS, load the value/pointer of LHS id to R4"));
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
			currentBlock.add(imMgr.newIM(ST,4,of,6,CPN+"(b)@"+nd2line(lvl)+" assign:LHS, about to evalueate the subscript, protect the pointer R4"));
			attachExpIM(arMgr, lvl.getChild(ASTN_exp, i), currentBlock, currentBlockVector);
			currentBlock.add(imMgr.newIM(LD, 4, of, 6,CPN+"(b)@"+nd2line(lvl)+" assign:LHS, subscript evaluation finished, recover R4 the pointer"));
			currentBlock.add(imMgr.newIM(LD, 3, arMgr.lookupExp(lvl.getChild(ASTN_exp, i)), 6,CPN+"(b)@"+nd2line(lvl)+" assign:LHS, load the value of subscript to R3"));
			currentBlock.add(imMgr.newIM(LDC,2,size,0,CPN+"(b)@"+nd2line(lvl)+" assign:LHS, load the \"range\" of the pointer to R2"));
			currentBlock.add(imMgr.newIM(MUL,3,3,2,CPN+"(b)@"+nd2line(lvl)+" assign:LHS, R3 = R3*R2, calcuate the offset to next position"));
			currentBlock.add(imMgr.newIM(ADD,4,4,3,CPN+"(b)@"+nd2line(lvl)+" assign:LHS, R4=R4+R3, move R4 the pointer to next position"));
		}
		currentBlock.add(imMgr.newIM(ST,4,of,6,CPN+"(b)@"+nd2line(sn)+" assign:RHS, aboute to evaluate RHS exp, protect R4, pointer to LHS memory"));
		attachExpIM(arMgr,sn.getChild(ASTN_exp, 0), currentBlock, currentBlockVector);
		currentBlock.add(imMgr.newIM(LD, 0, arMgr.lookupExp(sn.getChild(ASTN_exp, 0)), 6,CPN+"(b)@"+nd2line(sn)+" assign:RHS, load the value of RHS to R0"));
		currentBlock.add(imMgr.newIM(LD, 4, of, 6,CPN+"(b)@"+nd2line(sn)+" assign:RHS, RHS exp evaluation finish, recover R4, pointer to LHS memory"));
		arMgr.freeTmp();
		currentBlock.add(imMgr.newIM(ST,0,0,4,CPN+"(b)@"+nd2line(sn)+" assign, dMem[R4]=R0, store RHS to RHS"));
		break;
	}
	}
}

void attachLoopIM(ARMgr & arMgr, SemanticNode nd, Block &currentBlock, std::vector<Block> &currentBlockVector) {
	unsigned loopExit, faOffset;
	Block loopBlock;
	loopBlock.add(imMgr.newIM(LDA,4,0,4,CPN+"(b)@"+nd2line(nd)+" loop, NOOP, this is the entrance of loop block"));
	loopExit = imMgr.newIM(LDA,4,0,4,CPN+"(b)@"+nd2line(nd)+" loop, NOOP, this is the exit of loop block");
	currentBreakTarget.push_back(loopExit);
	switch(nd.type()) {
	case ASTN_do:
		attachExpIM(arMgr, nd.getChild(ASTN_exp, 0), loopBlock, currentBlockVector);
		loopBlock.add(imMgr.newIM(LD,4,arMgr.lookupExp(nd.getChild(ASTN_exp, 0)),6,CPN+"(b)@"+nd2line(nd)+" loop:do, load the value of the loop condition to R4"));
		arMgr.freeTmp();
		loopBlock.add(imMgr.newIM(JNE,4,1,7,CPN+"(b)@"+nd2line(nd)+" loop:do, if it is true(not 0), skip next instruction"));
		loopBlock.add(imMgr.newIM(loopExit,CPN+"(b)@"+nd2line(nd)+" loop:do, go to loop exit"));
		if(nd.getChildCount(ASTN_stms) >= 0) {
			attachStmsBlock(arMgr, nd.getChild(ASTN_stms, 0), loopBlock, currentBlockVector);
		}
		loopBlock.add(imMgr.newIM(loopBlock.entrance(),CPN+"(b)@"+nd2line(nd.getChild(nd.getChildCount()-1))+" loop:do, go to entrance of the loop block"));
		break;
	case ASTN_fa:
		faOffset = arMgr.pushFa(nd.getChild(ASTN_L_id, 0).idValue());
		attachExpIM(arMgr, nd.getChild(ASTN_exp, 0), currentBlock,currentBlockVector);
		attachExpIM(arMgr, nd.getChild(ASTN_exp, 1), currentBlock,currentBlockVector);
		currentBlock.add(imMgr.newIM(LD, 3, arMgr.lookupExp(nd.getChild(ASTN_exp, 0)), 6,CPN+"(b)@"+nd2line(nd)+" loop:fa, load the 1st exp condition to R3"));
		currentBlock.add(imMgr.newIM(LD, 4, arMgr.lookupExp(nd.getChild(ASTN_exp, 1)), 6,CPN+"(b)@"+nd2line(nd)+" loop:fa, load te 2nd exp condition to R4"));
		arMgr.freeTmp();
		currentBlock.add(imMgr.newIM(ST, 3, -(int)faOffset, 5,CPN+"(b)@"+nd2line(nd)+" loop:fa, push the start value R3 to Fa Stack"));
		currentBlock.add(imMgr.newIM(ST, 4, -(int)(faOffset + 1), 5,CPN+"(b)@"+nd2line(nd)+" loop:fa, push the end value R4 to Fa Stack"));
		currentBlock.add(imMgr.newIM(loopBlock.entrance(),CPN+"(b)@"+nd2line(nd)+" loop:fa, goto loop entrance."));
		loopBlock.add(imMgr.newIM(LD, 3, -(int)faOffset, 5,CPN+"(b)@"+nd2line(nd)+" loop:fa, load fa counter from Fa Stack to R3"));
		loopBlock.add(imMgr.newIM(LD, 4, -(int)(faOffset + 1), 5,CPN+"(b)@"+nd2line(nd)+" loop:fa, load end value from Fa stack to R4"));
		loopBlock.add(imMgr.newIM(SUB,0,4,3,CPN+"(b)@"+nd2line(nd)+" loop:fa, R0=R4-R3, R0= the difference betweeen the Fa counter with end value"));
		loopBlock.add(imMgr.newIM(JGE,0,1,7,CPN+"(b)@"+nd2line(nd)+" loop:fa, if R0>=0, skip the next instruction"));
		loopBlock.add(imMgr.newIM(loopExit,CPN+"(b)@"+nd2line(nd)+" loop:fa, goto loop exit"));
		if(nd.getChildCount(ASTN_stms) >= 0) {
			attachStmsBlock(arMgr, nd.getChild(ASTN_stms, 0), loopBlock, currentBlockVector);
		}
		loopBlock.add(imMgr.newIM(LD, 3, -(int)faOffset, 5,CPN+"(b)@"+nd2line(nd)+" loop:fa, load the Fa Counter to R3"));
		loopBlock.add(imMgr.newIM(LDA,3, 1, 3,CPN+"(b)@"+nd2line(nd)+" loop:fa, INC R3"));
		loopBlock.add(imMgr.newIM(ST, 3, -(int)faOffset, 5,CPN+"(b)@"+nd2line(nd)+" loop:fa, store R3 back to Fa Stack"));
		loopBlock.add(imMgr.newIM(loopBlock.entrance(),CPN+"(b)@"+nd2line(nd)+" loop:fa, go back to loop entrance"));
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
	m = arMgr.insert(nd);
	switch(sn.type()) {
	case ASTN_lvalue: 
		if (arMgr.isFa(sn.getChild(ASTN_L_id,0).idValue())) {
			currentBlock.add(imMgr.newIM(LD, 4,arMgr.lookupVar(sn.getChild(ASTN_L_id,0).idValue()),5,CPN+"(b)@"+nd2line(sn)+" exp:lvl, load the value of this var (which is a Fa Counter) to R4"));
			currentBlock.add(imMgr.newIM(ST,4,m,6,CPN+"(b)@"+nd2line(sn)+" exp:lvl, store the value of exp to dMem"));
		}
		else {
			std::vector<unsigned> v;
			v = varJumper.lookupDim(currentProcName, sn.getChild(ASTN_L_id, 0).idValue());
			currentBlock.add(imMgr.newIM(LDA, 4, arMgr.lookupVar(sn.getChild(ASTN_L_id, 0).idValue()),6,CPN+"(b)@"+nd2line(sn)+" exp:lvl, load the pointer"));
			unsigned i,j,max,k,size,of;
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
			currentBlock.add(imMgr.newIM(LD,4,0,4));
			currentBlock.add(imMgr.newIM(ST,4,m,6));
		}
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
	case ASTN_read:
		currentBlock.add(imMgr.newIM(IN,0,0,0));
		currentBlock.add(imMgr.newIM(ST,0,m,6));
	}
}

std::vector<unsigned> typeChecker(std::string currentProc,SemanticNode varlistnd) {
	char *tpName;
	std::vector<unsigned> ret, baseTypeDim;
	SemanticNode typedesnd;
	unsigned i, max;
	typedesnd = varlistnd.getChild(ASTN_typedes, 0);
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
			case ASTN_varlist: {
				std::vector<unsigned> dim;
				dim = typeChecker("0", sn);
				b = sn.getChild(ASTN_idlist, 0).getChildCount(ASTN_L_id);
				for(a = 0; a < b; a++) {	//each var
					char *varName;         //update AR structure
					varName = sn.getChild(ASTN_idlist, 0).getChild(ASTN_L_id, a).idValue();
					if(dim.empty() || dim.size() == 0) {
						arMgr.insert(varName);
					}
					else {
						arMgr.insertArray(varName,dim);
					}
				}
				break;
			}
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
		tmpBlock.add(imMgr.newIM(LDA, 4, 0, 4, "main(c): entrance. this is a NOOP"));
	}
	else {
		attachStmsBlock(arMgr, nd.getChild(ASTN_stms, 0), tmpBlock, currentBlockVector);
	}
	tmpBlock.add(imMgr.newIM(exitIM, "main(c): reached end of main body, goto exit"));
	currentBlockVector.push_back(tmpBlock);
	return tmpBlock.entrance();
}


void AST2TM(std::ostream &os) {
	std::map<std::string, ProcBlock>::iterator iter;
	SemanticTree tr;
	unsigned i, max, addr, en, exitIM;
	Block tmpBlock, entranceBlock;
	GlobalARMgr gARMgr;

	buildConstTable();
	currentProcName = "0";

	tmpBlock.add(imMgr.newIM(LDC, 6, ARhead(), 0, "main(c): Initialize Global AR pointer"));
	entranceBlock.add(imMgr.newIM(tmpBlock.entrance(), "main(c): Goto main block"));
	gBlocks.push_back(entranceBlock);
	tmpBlock.add(imMgr.newIM(LDC, 5, 0, 0, "main(c): R5 points to dMem[0]"));
	tmpBlock.add(imMgr.newIM(LD , 5, 0, 5, "main(c): R5 = dMem[0] (Initialize Fa pointer)"));
	exitIM = imMgr.newIM(HALT,0, 0, 0, "main(c): program exit");

	en = buildMain(gARMgr, tr, gBlocks, exitIM);
	tmpBlock.add(imMgr.newIM(LDA,4,2,7,"main(c): reg[4] now store PC+2 which is exit of the program"));
	tmpBlock.add(imMgr.newIM(ST,4,gARMgr.savedRegOffset()+7,6, "main(c): store R4 to stacked PC. If main returns, poped PC will points to exit"));

	tmpBlock.add(imMgr.newIM(en, "main(c): goto body of main (stms)"));
	tmpBlock.add(exitIM);
	gBlocks.push_back(tmpBlock);

	max = gBlocks.size();
	addr = 0;
	for (i = 0; i < max; i++) {
		addr = gBlocks[i].assignAbsoluteAddr(addr);
	}
	for (iter = procBlocks.begin(); iter != procBlocks.end(); iter++) {
		addr = (*iter).second.assignAbsoluteAddr(addr);
	}

	constTable2TM(os);
	max = gBlocks.size();
	for (i = 0; i < max; i++) {
		gBlocks[i].toTM(os);
	}
	for (iter = procBlocks.begin(); iter != procBlocks.end(); iter++) {
		(*iter).second.toTM(os);
	}
}
