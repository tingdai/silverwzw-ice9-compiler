#include "semantic.h"
#include "AST2TM.h"
int main() {
	parse();
	SemanticTree semanticTree;
	semanticCheck(semanticTree);
	AST2TM(std::cout);
	return 0;
}
