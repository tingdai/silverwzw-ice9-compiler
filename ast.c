#include <stdlib.h>
#include <assert.h>

#include "ast.h"
extern int yynewlines;

AST *newNode(NodeType t, int l) {
	AST *p;
	p = (AST *)malloc(sizeof(AST));
	p -> child = NULL;
	p -> sym = t;
	p -> brother = NULL;
	p -> lineno = l;
	return p;
}
AST *appendChild(AST *parent, AST * childNd) {
	assert(parent != NULL && childNd != NULL);
	AST *ptr;
	if (parent -> child == NULL) {
		parent -> child = childNd;
		return parent;
	}
	ptr = parent -> child;
	while (ptr -> brother != NULL) {
		ptr = ptr -> brother;
	}
	ptr -> brother = childNd;
	return parent;
}
AST *insertChild(AST *parent, AST * childNd) {
	assert(parent != NULL && childNd != NULL);
	childNd -> brother = parent -> child;
	parent -> child = childNd;
	return parent;
}

AST *appendBrother(AST *leftNode, AST *tobeappend) {
	assert(leftNode != NULL && tobeappend != NULL);
	tobeappend -> brother = leftNode -> brother;
	leftNode -> brother = tobeappend;
	return leftNode;
}

AST *appendBrotherAtLast(AST *leftNode, AST *tobeappend) {
	AST *ptr;
	assert(leftNode != NULL && tobeappend != NULL);
	ptr = leftNode;
	while( ptr -> brother != NULL) {
		ptr = ptr -> brother;
	}
	tobeappend -> brother = ptr -> brother;
	ptr -> brother = tobeappend;
	return leftNode;
}

AST *newStrNode(char *va) {
	AST *node;
	node = newNode(ASTN_L_string,-1);
	node -> value.str = va;
	return node;
}

AST *newIntNode(int va) {
	AST *node;
	node = newNode(ASTN_L_int,-1);
	node -> value.num = va;
	return node;
}

AST *newBoolNode(BOOL va) {
	AST *node;
	node = newNode(ASTN_L_bool,-1);
	node -> value.b = va;
	return node;
}

AST *newIdNode(char *va) {
	AST *node;
	node = newNode(ASTN_L_id,-1);
	node -> value.str = va;
	return node;
}
