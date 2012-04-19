#ifndef _TYPEJUMPER_H_
#define _TYPEJUMPER_H_ 1
#include <map>
#include <vector>
#include <string>
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
#endif

