#include "semantic.h"
int main() {
	parse();
	SemanticTree semanticTree;
	semanticCheck(semanticTree);
	semanticTree.freeTree();
	return 0;
}
