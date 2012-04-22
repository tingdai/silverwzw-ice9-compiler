#include <fstream>
#include "semantic.h"
#include "AST2TM.h"
int main(int argc, char *argv[]) {
	parse();
	SemanticTree semanticTree;
	semanticCheck(semanticTree);
	std::ofstream os(argv[1]);
	AST2TM(os);
	return 0;
}
