#ifndef _SEMANTIC_H_
#define _SEMANTIC_H_

extern "C" {
#include "ast.h"
#include "parse.h"
}

class SemanticNode {
protected:
	AST *data;
	SemanticNode(AST *d);
public:
	static bool isfree;
	bool eq(SemanticNode sn2);
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
	SemanticNode getChild(NodeType t, unsigned i);
	friend void travNode(SemanticNode nd);
};
void travNode(SemanticNode nd);

class SemanticTree : public SemanticNode {
public:
	SemanticTree();
};

enum Ice9BaseType {
	ice9int,
	ice9void,
	ice9str,
	ice9bool
};

struct Ice9Type {
	Ice9BaseType base;
	unsigned dim;
};

bool operator==(Ice9Type t1, Ice9Type t2);
bool operator!=(Ice9Type t1, Ice9Type t2);
bool operator==(SemanticNode sn1, SemanticNode sn2);
bool operator!=(SemanticNode sn1, SemanticNode sn2);

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
	Ice9Type getReturnType();
	Ice9Type getArg(unsigned i);
	unsigned argCount();
	SemanticNode declar();
	SemanticNode define();
	void setDeclar(SemanticNode n);
	void setDefine(SemanticNode n);
	bool typeEq(SemanticNode n);
	friend class ProcTab;
	friend class _procTab;
};

class _procTab {
public:
	_procTab(char *s);
	Ice9Proc proc;
	_procTab *next;
};


class ProcTab {
private:
	_procTab *tab;
public:
	ProcTab();
	~ProcTab();
	Ice9Proc *ProcessProcAndForwardNode(SemanticNode nd);
	void ProcessProccallNode(SemanticNode nd);
	Ice9Proc *getProc(char *name);
	bool exist(char *name);
};

enum _tabType{
	VAR_TABLE,
	TYPE_TABLE
};

struct _visibleStack {
	Ice9Type type;
	_visibleStack *next;
	SemanticNode visibleWithin;
};

struct _typeTab {
	_visibleStack visibleStack;
	char *name;
	_typeTab *next;
};

class VarTypeTab {
private:
	_typeTab *tab;
	_tabType tabType;
public:
	VarTypeTab(_tabType t);
	~VarTypeTab();
	void push(char *s, Ice9Type tp, SemanticNode node, long line);
	void popCorresponding(SemanticNode leaving);
	Ice9Type getType(char *s, long line);
	bool isFaCounter(char *s);
};

int semanticCheck(SemanticTree tree);
Ice9Type getTypeGeneral(SemanticNode nd);
void semanticError(char *errmsg, int lineno);
#endif
