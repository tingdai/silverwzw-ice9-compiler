#include <vector>
#include <cassert>
#include "constStr.h"

unsigned AROffset = 0;

unsigned ARhead() {
	return AROffset;
}

struct strConst {
	unsigned offset;
	std::string str;
};

std::vector<strConst> strTab; 

void nodecheck(SemanticNode nd) {
	unsigned max,i;
	max = nd.getChildCount();
	if (max == 0) {
		if(nd.type() == ASTN_L_string) {
			strConst tmp;
			tmp.str = nd.strValue();
			tmp.offset = AROffset;
			AROffset += tmp.str.size();
			strTab.push_back(tmp);
		}
	}
	else {
		for(i = 0; i < max; i++) {
			nodecheck(nd.getChild(i));
		}
	}
}

void buildConstTable() {
	SemanticTree tr;
	nodecheck(tr);
}

unsigned lookupStr(std::string s) {
	unsigned max,i;
	max = strTab.size();
	for (i = 0; i < max; i++) {
		if (strTab[i].str == s) {
			return strTab[i].offset;
		}
	}
	assert(false);
}

void constTable2TM(std::ostream os) {
	unsigned max,i;
	max = strTab.size();
	for (i = 0; i < max; i++) {
		os << ".SDATA \"" << (strTab[i].str) << "\"\n";
	}
}
