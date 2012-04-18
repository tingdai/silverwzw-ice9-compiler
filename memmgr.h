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
extern std::map<SemanticNode, MemOffset> globalTmp;

class MemMgr {
private:
	std::map<Varname, MemOffset> &global;
	std::map<Varname, MemOffset>  localVar;
	std::map<SemanticNode, MemOffset> localTmp;
	std::vector<VMPair> forStack;
public:
	MemMgr();
	MemOffset lookup(Varname);
	MemOffset lookup(SemanticNode exp);
};

class ARMgr {
private:
	unsigned ARlength;
	MemOffset ret;
	MenOffset para;
	Memoffset reg;
	MemOffset local;
	MemOffset localTmp;
public:
	ARMgr();
	unsigned length();
	MemOffset allocMen();
	void freeTemp();
};
