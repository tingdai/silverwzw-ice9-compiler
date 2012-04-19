#include <cassert>
#include "memmgr.h"


std::map<Varname, MemOffset> globalVar;

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
	faNum += 2;
	return faNum - 2;
}

void ARMgr::popFa(Varname) {
	faNum -= 2;
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

MemOffset GlobalMgr::insert(Varname) {
	assert(global.find(n) == global.end());
	global[n] = localTmpOffset();
	localNum ++;
	return global[n];
}

virtual MemOffset ARMgr::insertArray(Varname n, std::vector<unsigned> d) {
	assert(localVar.find(n) == localVar.end());
	unsigned i,max,slot;
	localVar[n] = localTmpOffset();
	slot = 1;
	max = d.size();
	for (i = 0; i < max; i++) {
		slot *= d[i];
	}
	localNum += slot;
	return localVar[n];
}

virtual MemOffset GlobalMgr::insertArray(Varname n, std::vector<unsigned> d) {
	assert(global.find(n) == global.end());
	global[n] = localTmpOffset();
	unsigned i,max,slot;
	slot = 1;
	max = d.size();
	for (i = 0; i < max; i++) {
		slot *= d[i];
	}
	localNum + =slot;
	return global[n];
}
