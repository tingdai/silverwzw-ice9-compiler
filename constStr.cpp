#include <vector>
#include <cassert>
#include "constStr.h"

unsigned AROffset = 1;

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
			AROffset += tmp.str.size() + 1;
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

void constTable2TM(std::ostream &os) {
	unsigned max,i;
	max = strTab.size();
	for (i = 0; i < max; i++) {
		os << ".DATA " << (strTab[i].str.length()) << '\n';
		if (strTab[i].str.find("\"") == std::string::npos) {
			os << ".SDATA \"" << (strTab[i].str) << "\"\n";
		}
		else {
			size_t found;
			std::string tmpstr;
			tmpstr = strTab[i].str;
			while ((found = tmpstr.find("\"")) != std::string::npos) {
				os << ".SDATA \"" << tmpstr.substr(0,found) <<"\"\n";
				os << ".DATA 34\n";
				tmpstr = tmpstr.substr(found+1);
			}
			os << ".SDATA \"" << tmpstr << "\"\n";
		}
	}
}
