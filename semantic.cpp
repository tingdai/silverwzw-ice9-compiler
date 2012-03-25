#include <cstdlib>
#include <cassert>
#include <cstring>
#include "semantic.h"
extern "C" {
#include "ice9.tab.h"
#include "parse.h"
}
#include <new>

bool SemanticNode::isfree = false;
VarTypeTab typeTab,varTab;
ProcTab procTab;
unsigned loop_counter = 0;
SemanticNode current_scope;

void semanticError(char *errmsg, int lineno) {
	std::cerr << "line " << lineno << ':' << errmsg << std::endl;
	exit(1)
}

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

long SemanticNode::line() {
	assert(isfree == false && data != NULL);
	return data -> lineno;
}

char *SemanticNode::idValue() {
	assert(isfree == false && data != NULL && data -> sym == ASTN_L_id);
	return data -> value.str;
}

char *SemanticNode::strValue() {
	assert(isfree == false && data != NULL && data -> sym == ASTN_L_id);
	return data -> value.str;
}

bool SemanticNode::boolValue() {
	assert(isfree == false && data != NULL && data -> sym == ASTN_L_id);
	return (data -> value.b == TRUE) ? true : false;
}

int SemanticNode::intValue() {
	assert(isfree == false && data != NULL && data -> sym == ASTN_L_id);
	return data -> value.num;
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
*/
SemanticTree::SemanticTree() : SemanticNode(&ASTroot) {
	;
}

void ASTfree(AST * nd) {
	if (nd == NULL) {
		return;
	}

	if (nd -> sym == ASTN_L_id || nd -> sym == ASTN_L_string) {
		free(nd -> value.str);
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
char *strdmp(char * s) {
	unsigned i;
	char *cpy;

	for(i = 0; *(s + i) != '\0'; i++) {
		;
	}

	i++;
	cpy = new char[i];
	memcpy(cpy, s, i);
	return cpy;
}

Ice9Proc::Ice9Proc(char * s) {
	name = strdmp(s);
	procArgs = NULL;
	procReturn.type.base = ice9int;
	procReturn.type.dim = 0;
	procReturn.isvoid = true;
}

Ice9Proc::~Ice9Proc(char * s) {
	_procArgTab *tmp;
	if (name != NULL) {
		delete []name;
	}
	tmp = procArgs;
	while(tmp != NULL) {
		procArgs = tmp -> next;
		delete tmp;
		tmp = procArgs;
	}
}

void Ice9Proc::addArg(Ice9Type arg) {
	if (procArgs ==  NULL) {
		procArgs = new _procArgTab;
		procArgs -> next = NULL;
		procArgs -> procArgType = arg;
		return;
	}
	_procArgTab *ptr;
	ptr = procArgs;
	while(ptr -> next != NULL) {
		ptr = ptr -> next;
	}
	ptr -> next = new _procArgTab;
	ptr -> next -> next = NULL;
	ptr -> next -> procArgType =arg;
}

void Ice9Proc::setReturn(Ice9Type arg) {
	assert(procReturn.isvoid == true);
	procReturn.type = arg;
	procReturn.isvoid == false;
}

Ice9Type Ice9Proc::getArg(unsigned i) {
	unsigned j = 0;
	_procReturn *ptr;
	ptr = procArgs;

	for (j = 0; j < i; j++) {
		ptr = ptr -> next;
		assert(ptr != NULL);
	}

	return ptr -> procArgType;
}

unsigned Ice9Proc::argCount() {
	unsigned j = 0;
	_procReturn *ptr;
	ptr = procArgs;

	while(ptr != NULL) {
		j++;
		ptr = ptr -> next;
	}
	return j;
}


SemanticNode Ice9Proc::declar() {
	return declarNode;
}

SemanticNode Ice9Proc::define() {
	return defineNode;
}

void Ice9Proc::setDeclar(SemanticNode n) {
	declarNode = n;
}

void Ice9Proc::setDefine(SemanticNode n) {
	defineNode = n;
}

bool Ice9Proc::typeEq(SemanticNode n) {
	NodeType ndtp;
	ndtp = n.type();
	assert(ndtp == ASTN_proc || ndtp == ASTN_proccall);
	switch (ndtp) {
	case ASTN_proc:
//check return type
		if (n.getChildCount(ASTN_L_id) != ((procReturn.isvoid) ? 1 : 2)) {
			return false;
		}
		if (n.getChildCount(ASTN_L_id) == 2) { // has return type
			if (typeTab.getType(n.getChild(ASTN_L_id, 1).idValue()) != procReturn.type) {
				return false;
			}
		}
//check arguments
		if (n.getChildCount(ASTN_declistx) == 0 && procArgs != NULL) { // if declars no args but define has args
			return false;
		}
		if (n.getChildCount(ASTN_declistx) > 0) { 
			SemanticNode N_declistx;
			unsigned i = 0, j,k,argCountByDec;
			argCountByDec = argCount();
			N_declistx = n.getChildByType(ASTN_declistx, 0);
			j = N_declistx.getChildCount(ASTN_declist);
			for ( k = 0; k < j; k++) {
				unsigned ids, l;
				Ice9Type declistType;
				SemanticNode N_declist;

				N_declist = tmpnd.getChild(ASTN_declist, k);

				declistType = typeTab.getType(N_declist.getChild(ASTN_L_id).idValue());
				ids = N_declist.getChildCount(ASTN_idlist).getChildCount(ASTN_L_id);

				for (l = 0; l < ids; l++){
					if ( j == argCountByDec) {
						return false;
					}
					if (getArg(j) != declistType) {
						return false;
					}
					j++;
				}
			}
			if ( j != argCountByDec ) {
				return false;
			}
		}
		break;
	case ASTN_proccall: // only check Args no return type checking
		unsigned argNum, i;
		argNum = argCount();
		if (n.getChildCount(ASTN_exp) != argNum) { // if arg number not match
			return false;
		}
		for (i = 0; i < argNum; i++) { // check every arg
			if ( getArg(i) != getTypeGeneral(n.getChild(ASTN_exp, i))) {
				return false;
			}
		}
		break;
	}
	return true;
}
/*
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
	Ice9Proc(char * name);
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
enum Ice9BaseType {
	ice9int,
	ice9str,
	ice9bool
};
struct Ice9Type {
	Ice9BaseType base;
	unsigned dim;
};
*/

ProcTab::procTab() {
	tab = NULL;
}

ProcTab::~procTab() {
	_procTab *ptr;
	while( tab != NULL) {
		ptr = tab;
		tab = tab -> next;
		if (ptr -> proc.declar().isNull() == true) {
			semanticError("procedure declared but not defined!", ptr -> proc.define().line());
		}
		delete ptr;
	}
}

Ice9Proc &ProcTab::ProcessProcAndForwardNode(SemanticNode nd) {
	NodeType ndtp;
	char *procName;

	ndtp = nd.type();
	assert(ndtp == ASTN_forward || ndtp == ASTN_proc);
	procName = nd.getChild(ASTN_L_id, 0).idValue();
	if (ndtp == ASTN_proc) {
		current_scope = nd;
	}

	if (exist(procName)) {
		Ice9Proc *theProc = NULL;
		if (ndtp == ASTN_forward) {
			semanticError("procedure already declared!",nd.line());
		}
		if (ndtp == ASTN_proc) {
			theProc = getProc(procName);
			if (theProc -> define().isNull() == false) {
				semanticError("procedure already defined!", nd.line());
			}
			theProc -> setDefine(nd);
			if (theProc -> typeEq(nd)) {
				semanticError("procedure return type or argument types do not match previous declaration", nd.line());
			}
			if (theProc -> procReturn.isvoid == false) {
				varTab.push(procName, theProc->procReturn.type, nd, nd.line());
			}
			if (nd.getChildCount(ASTN_declistx) > 0) {
				SemanticNode N_declistx;
				N_declistx = nd.getChild(ASTN_declistx, 0);
				unsigned num_declist,i;
				num_declist = N_declistx.getChildCount(ASTN_declist);
				for (i = 0; i < num_declist; i++) {
					SemanticNode N_declist, N_idlist;
					N_declist = N_declistx.getChild(ASTN_declist, i);
					N_idlist = N_declist.getChild(ASTN_idlist,0);
					Ice9Type currentType;
					currentType = typeTab.getType(N_declist.getChild(ASTN_L_id, 0).idValue());
					unsigned j, num_id;
					num_id = N_idlist.getChildCount(ASTN_L_id);
					for (j = 0; j < num_id; j++) {
						SemanticNode N_id;
						N_id = N_idlist.getChild(ASTN_L_id, j);
						varTab.push(N_id.idValue(), currentType, nd, N_idlist.line());
					}
				}
			}
		}
		return theProc;
	}
	Ice9Proc *newProc;
	if (tab == NULL) {
		tab = new Ice9Proc(procName);
		newProc = tab;
	}
	else {
		newProc = tab;
		while(newProc -> next != NULL) {
			newProc = newProc -> next;
		}
		newProc -> next = new Ice9Proc(procName);
		newProc = newProc -> next;
	}
	switch(ndtp) {
	case ASTN_forward:
		newProc -> setDeclar(nd);
		if (nd.getChildCount(ASTN_L_id) > 1) {
			newProc -> setReturn(typeTab.getType(nd.getChild(ASTN_L_id, 1).idValue()));
			if (newProc -> procReturn.type.dim != 0) {
				semanticError("procedure cannot return an array!",nd.line());
			}
		}
		if (nd.getChildCount(ASTN_declistx) > 0) {
			SemanticNode N_declistx;
			N_declistx = nd.getChild(ASTN_declistx, 0);
			unsigned current_declist, num_declist;
			num_declist = N_declistx.getChildCount(ASTN_declist);
			for (current_declist = 0; current_declist < num_declist; current_declist++) {
				SemanticNode N_declist;
				N_declist = N_declistx.getChild(ASTN_declist, current_declist);
				Ice9Type currentType;
				currentType = typeTab.getType(N_declist.getChild(ASTN_L_id, 0).idValue());
				unsigned counter, num_idlist;
				num_idlist = N_declist.getChild(ASTN_idlist, 0).getChildCount(ASTN_L_id);
				for (counter = 0; counter < num_idlist; counter ++) {
					newProc -> addArg(currentType);
				}
			}
		}
		break;
	case ASTN_proc:
		newProc -> setDeclar(nd);
		newProc -> setDefine(nd);
		if (nd.getChildCount(ASTN_L_id) > 1) {
			newProc -> setReturn(typeTab.getType(nd.getChild(ASTN_L_id,1).idValue()));
			if (newProc -> procReturn.type.dim != 0) {
				semanticError("procedure cannot return an array!",nd.line());
			}
		}
		if (theProc -> procReturn.isvoid == false) {
			varTab.push(procName, theProc->procReturn.type, nd, nd.line());
		}
		if (nd.getChildCount(ASTN_declistx) > 0) {
			SemanticNode N_declistx;
			N_declistx = nd.getChild(ASTN_declistx, 0);
			unsigned num_declist,i;
			num_declist = N_declistx.getChildCount(ASTN_declist);
			for (i = 0; i < num_declist; i++) {
				SemanticNode N_declist, N_idlist;
				N_declist = N_declistx.getChild(ASTN_declist, i);
				N_idlist = N_declist.getChild(ASTN_idlist,0);
				Ice9Type currentType;
				currentType = typeTab.getType(N_declist.getChild(ASTN_L_id, 0).idValue());
				unsigned j, num_id;
				num_id = N_idlist.getChildCount(ASTN_L_id);
				for (j = 0; j < num_id; j++) {
					SemanticNode N_id;
					N_id = N_idlist.getChild(ASTN_L_id, j);
					varTab.push(N_id.idValue(), currentType, nd, N_idlist.line());
					newProc -> addArg(currentType);
				}
			}
		}
		break;
	}
	return newProc;
}
/*
class ProcTab {
private:
	struct _procTab {
		Ice9Proc proc;
		_procTab *next;
	} *tab;
public:
	ProcTab();
	~ProcTab();  // also check if all proc is defined;
	Ice9Proc *ProcessProcAndForwardNode(SemanticNode nd);
	void ProcessProccallNode(SemanticNode nd);
	Ice9Proc *getProc(char *name);
	bool exist(char *name);
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
	bool isConflict(char *s, SemanticNode visibleNode);
	void push(char *s, Ice9Type tp, SemanticNode node, long line);
	void popCorresponding(SemanticNode leaving);
	Ice9Type getType(char *s);
};
*/
int semanticCheck(SemanticTree tr) {
	current_scope = *(SemanticNode *)&tr;
	return 0;
}
Ice9Type getTypeGeneral(SemanticNode nd) {
}
