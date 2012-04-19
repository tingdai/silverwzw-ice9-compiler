#ifndef _TYPEJUMPER_H_
#define _TYPEJUMPER_H_ 1
#include <map>
#include <vector>
#include <string>
struct TJ_TP{
	std::string base;
	std::vector<unsigned> dim;
};

typedef std::map<std::string, TJ_TP> _INTER_TYPE1; //this line is for g++ to recognize the defination, stupit g++.......

typedef std::map<std::string,_INTER_TYPE1> IndirectTable;

class TypeJumper {
private:
	IndirectTable indirectTable;
public:
	void addType(std::string procN, std::string typeN, std::string baseTPN);
	void pushDim(std::string procN, std::string tyepN, unsigned d);
	std::vector<unsigned> lookupDim(std::string procN, std::string typeN);
};

extern TypeJumper typeJumper;
#endif

