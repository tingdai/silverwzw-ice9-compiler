#include "semantic.h"
int main() {
	SemanticTree semanticTree;
	semanticCheck(semanticTree);
	semanticTree.freeTree();
	return 0;
}
