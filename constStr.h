#ifndef _CONSTSTR_H_
#define _CONSTSTR_H
#include "semantic.h"
#include <string>
#include <iostream>
void buildConstTable();
unsigned lookupStr(std::string);
void constTable2TM(std::ostream&);
unsigned ARhead();
#endif
