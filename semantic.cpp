#include <cstdlib>
#include <cassert>
#include "semantic.h"
extern "C" {
#include "ice9.tab.h"
#include "parse.h"
}

bool SemanticNode::isfree = false;

SemanticNode::SemanticNode(AST *d) {
	data = d;
}

SemanticNode::SemanticNode() {
	data = NULL;
}

Nodetype SemanticNode::type() {
	assert(isfree == false);
	return data -> sym;
}

bool SemanticNode::isFree() {
	return isfree;
}

bool SemanticNode::isNull() {
	assert(isfree == false);
	return data == NULL;
}

unsigned SemanticNode::getChildCount() {
	assert(isfree == false && data != NULL);
	if (data -> child == NULL) {
		return 0;
	}
	unsigned n = 1;
	AST *ptr;
	ptr = data -> child;
	while (ptr -> brother != NULL) {
		ptr = ptr -> brother;
		n++;
	}
	return n;
}

unsigned SemanticNode::getChildCount(NodeType tp) {
	assert(isfree == false && data != NULL);
	if (data -> child == NULL) {
		return 0;
	}
	unsigned n;
	AST *ptr;
	ptr = data -> child;
	n = (ptr -> sym == tp) ? 1 : 0;
	while (ptr -> brother != NULL) {
		ptr = ptr -> brother;
		if (ptr -> sym == tp) {
			n++;
		}
	}
	return n;
}

SemanticNode SemanticNode::getChild(unsigned i) {
	assert(isfree == false && data != NULL && data -> child != NULL);
	unsigned j = 0;
	AST *ptr;
	ptr = data -> child;
	for (j = 0; j < i; j++) {
		ptr = ptr -> brother;
		assert(ptr != NULL);
	}
	SemanticNode r(ptr);
	return r;
}

SemanticNode SemanticNode::getChildByType(NodeType tp, unsigned i) {
	assert(isfree == false && data != NULL && data -> child != NULL);
	unsigned j = 0;
	AST *ptr;
	ptr = data -> child;
	j = 0;
	while ( j <= i ) {
		ptr = ptr -> brother;
		assert(ptr != NULL);
		if(ptr -> sym == tp) {
			j++;
		}
	}
	SemanticNode r(ptr);
	return r;
}
/*
class SemanticNode {
protected:
	static bool isfree;
	AST *data;
	SemanticNode(AST *d);
public:
	SemanticNode();
	NodeType type();
	bool isFree();
	bool isNull();
	unsigned getChildCount();
	unsigned getChildCount(NodeType tp);
	SemanticNode getChild(unsigned i);
	SemanticNode getChildByType(NodeType t, unsigned i);
};
*/
SemanticTree::SemanticTree() : SemanticNode(&ASTroot) {
	;
}

void ASTfree(AST * nd) {
	if (nd == NULL) {
		return;
	}

	ASTfree( nd->child );
	ASTfree( nd->brother );

	if (nd == &ASTroot) {
		return;
	}

	free(nd);
}

void SemanticTree::freeTree() {
	assert(isfree == false);
	ASTfree(&ASTroot);
	isfree = true;
}

/*
class SemanticTree : public SemanticNode {
public:
	SemanticTree();
	void freeTree();
};
*/

/*
enum Ice9BaseType {
	ice9int,
	ice9str,
	ice9bool
};

struct Ice9Type {
	Ice9BaseType base;
	unsigned dim;
};

class Ice9Proc {
private:
	char *name;
	struct _procArgTab {
		Ice9Type procArgs;
		_procArgTab *next;
	} *procArgs;
	struct _procReturn {
		Ice9Type type;
		bool isvoid;
	} procReturn;
	SemanticNode declarNode;
	SemanticNode defineNode;
	Ice9Proc(char * name);
public:
	~Ice9Proc();
	void addArgs(Ice9Type arg);
	Ice9Type getArg(unsigned i);
	unsigned argCount();
	SemanticNode declar();
	SemanticNode define();
	void setDeclar(SemanticNode n);
	void setDefine(SemanticNode n);
	bool typeEq(SemanticNode n);
	friend class ProcTab;
};
*/
/*
class ProcTab {
private:
	struct _procTab {
		Ice9Proc proc;
		_procTab *next;
	} *tab;
public:
	ProcTab();
	~ProcTab();
	Ice9Proc &newProc(char *name);
	bool isConflict(char *name);
};
*/
/*
class VarTypeTab {
private:
	struct _typeTab {
		struct _visibleStack {
			Ice9Type type;
			_visibleStack *next;
			SemanticNode visibleWithin;
		} visibleStack;
		char *name;
		_typeTab *next;
	} *tab;
public:
	VarTypeTab();
	~VarTypeTab();
	bool isConflict(char *s);
	void push(char *s, Ice9Type tp, SemanticNode node);
	void popCorresponding(SemanticNode leaving);
	bool check(char *s, Ice9Type tp);
};
*/
int semanticCheck(SemanticTree tr) {
	VarTypeTab typeTab,varTab;
	ProcTab procTab;
	return 0;
}
