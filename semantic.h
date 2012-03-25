#ifndef _SEMANTIC_H_
#define _SEMANTIC_H_

extern "C" {
#include "ast.h"
}

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
	long line();
	char *idValue();
	char *strValue();
	bool boolValue();
	int intValue();
	unsigned getChildCount();
	unsigned getChildCount(NodeType tp);
	SemanticNode getChild(unsigned i);
	SemanticNode getChildByType(NodeType t, unsigned i);
};

class SemanticTree : public SemanticNode {
public:
	SemanticTree();
	void freeTree();
};

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
		Ice9Type procArgType;
		_procArgTab *next;
	} *procArgs;
	struct _procReturn {
		Ice9Type type;
		bool isvoid;
	} procReturn;
	SemanticNode declarNode;
	SemanticNode defineNode;
	Ice9Proc(char * s);
public:
	~Ice9Proc();
	void addArg(Ice9Type arg);
	void setReturn(Ice9Type arg);
	Ice9Type getArg(unsigned i);
	unsigned argCount();
	SemanticNode declar();
	SemanticNode define();
	void setDeclar(SemanticNode n);
	void setDefine(SemanticNode n);
	bool typeEq(SemanticNode n);
	friend class ProcTab;
};

class ProcTab {
private:
	struct _procTab {
		Ice9Proc proc;
		_procTab *next;
	} *tab;
public:
	ProcTab();
	~ProcTab();
	Ice9Proc *ProcessProcAndForwardNode(SemanticNode nd);
	void ProcessProccallNode(SemanticNode nd);
	Ice9Proc *getProc(char *name);
	bool exist(char *name);
};

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
	bool isConflict(char *s, SemanticNode visibleNode);
public:
	VarTypeTab();
	~VarTypeTab();
	void push(char *s, Ice9Type tp, SemanticNode node, long line);
	void popCorresponding(SemanticNode leaving);
	Ice9Type getType(char *s);
};

int semanticCheck(SemanticTree tree);
Ice9Type getType(SemanticNode nd);
#endif
