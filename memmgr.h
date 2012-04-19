#ifndef _MEMMGR_H_
#define _MEMMGR_H_ 1
#include <vector>
#include <map>
#include <string>
#include "semantic.h"

typedef unsigned MemOffset;
typedef std::string Varname;

struct VMPair {
	Varname var;
	MemOffset offset;
};

extern std::map<Varname, MemOffset> globalVar;

class ARMgr {
private:
	unsigned ARlength;
	unsigned paraNum;
	unsigned localNum;
	unsigned faNum;
	std::map<Varname, MemOffset> &global;
	std::map<Varname, MemOffset>  localVar;
	std::map<SemanticNode, MemOffset> localTmp;
	std::vector<VMPair> forStack;
public:
	ARMgr();
	unsigned length();
	MemOffset returnValueOffset();
	MemOffset parametersOffset();
	MemOffset savedRegOffset();
	MemOffset localVarOffset();
	MemOffset localTmpOffset();
	MemOffset lookupVar(Varname);
	MemOffset lookupExp(SemanticNode exp);
	MemOffset lookupFa(Varname);
	MemOffset pushPara(Varname);
	virtual MemOffset insert(Varname);
	virtual MemOffset insertArray(Varname, std::vector<unsigned>);
	MemOffset insert(SemanticNode exp);
	MemOffset pushFa(Varname);
	void popFa(Varname);
	void freeTmp();
	bool isFa(Varname);
};

class GlobalARMgr : public ARMgr {
public:
	virtual MemOffset insert(Varnamr);
	virtual MemOffset insertArray(Varname, std::vector<unsigned>);
};

#endif
