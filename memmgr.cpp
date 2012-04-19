#include <cassert>
#include "memmgr.h"


std::map<Varname, MemOffset> globalVar;
std::map<SemanticNode, MemOffset> globalTmp;

ARMgr::ARMgr():global(globalVar) {
	ARlength = 9;
	paraNum = 0;
	localNum = 0;
	faNum = 0;
}

MemOffset ARMgr::pushFa(Varname n) {
	VMPair vmpt;

	vmpt.var = n;
	vmpt.offset = faNum;
	forStack.push_back(vmpt);
	return faNum++;
}

void ARMgr::popFa(Varname) {
	faNum --;
	forStack.pop_back();
}

MemOffset ARMgr::pushPara(Varname n) {
	assert(localVar.find(n) == localVar.end());
	paraNum++;
	return (localVar[n] = parametersOffset() + paraNum - 1);
}

MemOffset ARMgr::lookupVar(Varname n) {
	if (!(localVar.find(n) == localVar.end())) {
		return localVar[n];
	}
	else if (!(global.find(n) == global.end())) {
		return global[n];
	}
	assert(false);
}


MemOffset ARMgr::lookupExp(SemanticNode nd) {
	if (!(localTmp.find(nd) == localTmp.end())) {
		return localTmp[nd];
	}
	assert(false);
}

MemOffset ARMgr::lookupFa(Varname n) {
	int i;
	for (i = forStack.size() - 1; i >= 0; i--) {
		if (forStack[i].var == n) {
			return forStack[i].offset;
		}
	}
	assert(false);
}

bool ARMgr::isFa(Varname n) {
	int i;
	for (i = forStack.size() - 1; i >= 0; i--) {
		if (forStack[i].var == n) {
			return true;
		}
	}
	return false;
}

MemOffset ARMgr::insert(Varname n) {
	assert(localVar.find(n) == localVar.end());
	localVar[n] = localTmpOffset();
	localNum ++;
	return localVar[n];
}

MemOffset ARMgr::insert(SemanticNode exp) {
	localTmp[exp] = ARlength;
	return ARlength++;
}

void ARMgr::freeTmp() {
	localTmp.clear();
	ARlength = 1 + paraNum + 8 + localNum;
}

unsigned ARMgr::length() {
	return ARlength;
}

MemOffset ARMgr::returnValueOffset() {
	return 0;
}

MemOffset ARMgr::parametersOffset() {
	return 1;
}

MemOffset ARMgr::savedRegOffset() {
	return 1 + paraNum;
}

MemOffset ARMgr::localVarOffset() {
	return 9 + paraNum;
}

MemOffset ARMgr::localTmpOffset() {
	return 9 + paraNum + localNum;
}
