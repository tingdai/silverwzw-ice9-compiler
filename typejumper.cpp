#include <vector>
#include <string>
#include "typejumper.h"
TypeJumper typeJumper;
TypeJumper varJumper;

void TypeJumper::addType(std::string procN, std::string typeN, std::string baseTypeN) {
	TJ_TP tmp;
	tmp.base = baseTypeN;
	indirectTable[procN][typeN] = tmp;
}

void TypeJumper::pushDim(std::string procN, std::string typeN, unsigned d) {
	indirectTable[procN][typeN].dim.push_back(d);
}

std::vector<unsigned> TypeJumper::lookupDim(std::string currentProc, std::string typeN) {
	if (indirectTable[currentProc].find(typeN) != indirectTable[currentProc].end()) {
		if (indirectTable[currentProc][typeN].base == "0") {
			return indirectTable[currentProc][typeN].dim;
		}
		else {
			std::vector<unsigned> basev, thisv;
			unsigned i,max;
			basev = typeJumper.lookupDim(currentProc, indirectTable[currentProc][typeN].base);
			thisv = indirectTable[currentProc][typeN].dim;
			max = basev.size();
			for (i = 0; i < max; i++) {
				thisv.push_back(basev[i]);
			}
			return thisv;
		}
	}
	else if(currentProc != "0"){
		if (indirectTable["0"][typeN].base == "0") {
			return indirectTable["0"][typeN].dim;
		}
		else {
			std::vector<unsigned> basev, thisv;
			unsigned i,max;
			basev = typeJumper.lookupDim("0", indirectTable["0"][typeN].base);
			thisv = indirectTable["0"][typeN].dim;
			max = basev.size();
			for (i = 0; i < max; i++) {
				thisv.push_back(basev[i]);
			}
			return thisv;
		}
	}
	std::vector<unsigned> emptyv;
	return emptyv;
}



/*
struct TJ_TP{
	std::string base;
	vector<unsigned> dim;
};

class TypeJumper {
private:
	std::map<std::string,std::map<std::string, TJ_TP>> indirectTable;
public:
	void addType(std::string procN, std::string typeN, std::string baseTPN);
	void pushDim(std::string procN, std::string tyepN, unsigned d);
	vector<unsigned> lookupDim(std::string procN, std::string typeN);
};
*/
