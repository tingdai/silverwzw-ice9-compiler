#include <cstdlib>
#include <cassert>
#include <cstring>
#include <iostream>
#include "typejumper.h"
#include "semantic.h"
extern "C" {
#include "ice9.tab.h"
#include "ast.h"
}
#include <new>

const Ice9Type INT={ice9int,0}; 
const Ice9Type BOOLEAN={ice9bool,0}; 
const Ice9Type STR={ice9str,0}; 

bool SemanticNode::isfree = false;

ProcTab procTab;// the order is important!
VarTypeTab typeTab(TYPE_TABLE) ,varTab(VAR_TABLE);
unsigned loop_counter = 0;
SemanticNode current_scope;

void nodeCheckOut(SemanticNode nd) {
	switch(nd.type()) {
	case ASTN_proc:{
		SemanticTree tr;
		current_scope = *(SemanticNode *)&tr;
	}
		varTab.popCorresponding(nd);
		typeTab.popCorresponding(nd);
		break;
	case ASTN_fa:
		varTab.popCorresponding(nd);
		typeTab.popCorresponding(nd);
		//no break here
	case ASTN_do:
		assert(loop_counter > 0);
		loop_counter--;
		break;
	default:
		break;
	}
	return;
}

void nodeCheckIn(SemanticNode nd) {
	NodeType ndtp;
	unsigned i,j;
	ndtp = nd.type();
	if (ndtp == ASTN_proc) {
		current_scope = nd;
	}
	switch(ndtp) {
	case ASTN_forward:
		procTab.ProcessProcAndForwardNode(nd);
		break;
	case ASTN_stm: // only for stm -> exp ';'
		if (nd.getChildCount() == 1 && nd.getChildCount(ASTN_exp) == 1) { 
			getTypeGeneral(nd.getChild(0));
		}
		break;
	case ASTN_proc:
		procTab.ProcessProcAndForwardNode(nd);
		//current scope changes at the beginning of the function.
		break;
	case ASTN_L_break:
		if (loop_counter == 0) {
			semanticError("statement break cannot be used outside a loop",nd.line());
		}
		break;
	case ASTN_fa:
		i = nd.getChildCount(ASTN_L_id);
		j = nd.getChildCount(ASTN_exp);
		assert(i == 1 && j == 2);
		if (getTypeGeneral(nd.getChild(ASTN_exp, 0)) != INT || getTypeGeneral(nd.getChild(ASTN_exp,1)) != INT) {
			semanticError("start and end expression in fa should be int", nd.line());
		}
		varTab.push(nd.getChild(ASTN_L_id,0).idValue(), INT, nd, nd.line());
		loop_counter++;
		break;
	case ASTN_do:
		if(getTypeGeneral(nd.getChild(ASTN_exp, 0)) != BOOLEAN) {
			semanticError("condition expression in do should be bool", nd.line());
		}
		loop_counter++;
		break;
	case ASTN_assign: {
		i = nd.getChildCount(ASTN_lvalue);
		j = nd.getChildCount(ASTN_exp);
		assert(i == 1 && j == 1);
		SemanticNode lvl;
		lvl = nd.getChild(ASTN_lvalue, 0);
		i = lvl.getChildCount(ASTN_L_id);
		j = lvl.getChildCount(ASTN_exp);
		assert(i == 1);
		Ice9Type lvlIdType;
		lvlIdType = varTab.getType(lvl.getChild(ASTN_L_id,0).idValue(), nd.line());
		if (varTab.isFaCounter(lvl.getChild(ASTN_L_id, 0).idValue())) {
			semanticError("making assignment to fa counter is not allowed.", nd.line());
		}
		if (lvlIdType.dim > j) {
			semanticError("Array is not a valid LHS in assign statement",nd.line());
		}
		if (lvlIdType.dim < j) {
			semanticError("Too many subscript for array", nd.line());
		}
		for (i = 0; i < j; i++) {
			if (getTypeGeneral(lvl.getChild(ASTN_exp, i)) != INT) {
				semanticError("Type of subscript should be int",nd.line());
			}
		}
		lvlIdType.dim = 0;
		if (lvlIdType != getTypeGeneral(nd.getChild(ASTN_exp, 0))) {
			semanticError("Type mismatch between LHS and RHS in assign statement",nd.line());
		}
		break;
	}
	case ASTN_write: //merge this two
	case ASTN_writes: {//merge this two 
		Ice9Type tp;
		tp = getTypeGeneral(nd.getChild(ASTN_exp, 0));
		if (tp != INT && tp != STR) {
			semanticError("Only int and string is valid type in write statement",nd.line());
		}
		if (tp == INT) {
			nd.reload(ice9int);
		}
		}
		break;
	case ASTN_varlist:{
		SemanticNode idlst;
		Ice9Type tp;
		SemanticNode typedesnd;
		typedesnd = nd.getChild(ASTN_typedes, 0);
		idlst = nd.getChild(ASTN_idlist, 0);
		tp = getTypeGeneral(typedesnd);
		j = idlst.getChildCount(ASTN_L_id);
		for (i = 0; i < j; i++) {
			char *s,*idn;
			varTab.push(idlst.getChild(ASTN_L_id, i).idValue(), tp, current_scope, nd.line());
			if (current_scope.type() == ASTN_proc) {
				s = current_scope.getChild(ASTN_L_id, 0).idValue();
			}
			else{
				s = "0";
			}
			idn = idlst.getChild(ASTN_L_id, i).idValue();
			if (typeTab.getType(typedesnd.getChild(ASTN_L_id, 0),nd.line()).dim != 0) {
				varJumper.addType(s, idn, typedesnd.getChild(ASTN_L_id, 0).idValue());
			}
			else {
				varJumper.addType(s, idn, "0");
			}
			unsigned counter, maxcounter;
			maxcounter = typedesnd.getChildCount(ASTN_L_int);
			for (counter = 0; counter < maxcounter; counter++) {
				varJumper.pushDim(s, idn, typedesnd.getChild(ASTN_L_int, counter).intValue());
			}
		}
		break;
	}
	case ASTN_type: {
		Ice9Type tp;
		char *s,*idV;
		SemanticNode tpdes;
		unsigned i,max;
		tpdes = nd.getChild(ASTN_typedes, 0);
		tp = getTypeGeneral(tpdes);
		idV = nd.getChild(ASTN_L_id).idValue();
		typeTab.push(idV, tp, current_scope, nd.line());
		if (current_scope.type() == ASTN_proc) {
			s = current_scope.getChild(ASTN_L_id,0).idValue();
		}
		else {
			s = "0";
		}
		tp = getTypeGeneral(tpdes.getChild(ASTN_L_id, 0));
		if (tp == INT || tp == BOOLEAN || tp == STR) {
			typeJumper.addType(s, idV, "0");
		}
		else {
			typeJumper.addType(s, idV, tpdes.getChild(ASTN_L_id,0).idValue());
		}
		max = tpdes.getChildCount(ASTN_L_int);
		for (i = 0; i < max; i++) {
			typeJumper.pushDim(s, idV, tpdes.getChild(ASTN_L_int, i).intValue());
		}
		break;
	}
	case ASTN_branch:
		i = nd.getChildCount(ASTN_exp);
		assert(i == 1 || i == 0);
		if (i == 1){
			if(getTypeGeneral(nd.getChild(ASTN_exp, 0)) != BOOLEAN) {
				semanticError("branch condition should return a bool value", nd.line());
			}
		}
		break;
	default:
		break;
	}
	return;
}

bool operator==(Ice9Type t1, Ice9Type t2) {
	return (t1.dim == t2.dim && t1.base == t2.base);
}

bool operator!=(Ice9Type t1, Ice9Type t2) {
	return !(t1 == t2);
}

void semanticError(char *errmsg, int lineno) {
	std::cerr << "line " << lineno << ": " << errmsg << std::endl;
	exit(1);
}

SemanticNode::SemanticNode(AST *d) {
	data = d;
}

SemanticNode::SemanticNode() {
	data = NULL;
}

NodeType SemanticNode::type() {
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
	assert(ptr != NULL);
	for (j = 1; j < i; j++) {
		ptr = ptr -> brother;
		assert(ptr != NULL);
	}
	SemanticNode r(ptr);
	return r;
}

SemanticNode SemanticNode::getChild(NodeType tp, unsigned i) {
	assert(isfree == false && data != NULL && data -> child != NULL);
	unsigned j = 0;
	AST *ptr;
	ptr = data -> child;
	assert(ptr != NULL);
	if(ptr -> sym == tp) {
		j++;
	}
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
	SemanticNode getChild(NodeType t, unsigned i);
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
	procReturn.type.base = ice9void;
	procReturn.type.dim = 0;
	procReturn.isvoid = true;
}

Ice9Proc::~Ice9Proc() {
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
	procReturn.isvoid = false;
}

Ice9Type Ice9Proc::getArg(unsigned i) {
	unsigned j = 0;
	_procArgTab *ptr;
	ptr = procArgs;

	for (j = 0; j < i; j++) {
		ptr = ptr -> next;
		assert(ptr != NULL);
	}

	return ptr -> procArgType;
}

Ice9Type Ice9Proc::getReturnType() {
	return procReturn.type;
}

unsigned Ice9Proc::argCount() {
	unsigned j = 0;
	_procArgTab *ptr;
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
			if (typeTab.getType(n.getChild(ASTN_L_id, 1).idValue(), n.line()) != procReturn.type) {
				return false;
			}
		}
//check arguments
		if (n.getChildCount(ASTN_declistx) == 0 && argCount() != 0) { // if declar and define has different number of args
			return false;
		}
		if (n.getChildCount(ASTN_declistx) > 0) { 
			SemanticNode N_declistx;
			unsigned i = 0, j,k,argCountByDec;
			argCountByDec = argCount();
			N_declistx = n.getChild(ASTN_declistx, 0);
			j = N_declistx.getChildCount(ASTN_declist);
			for ( k = 0; k < j; k++) {
				unsigned ids, l;
				Ice9Type declistType;
				SemanticNode N_declist;

				N_declist = N_declistx.getChild(ASTN_declist, k);

				declistType = typeTab.getType(N_declist.getChild(ASTN_L_id, 0).idValue(), N_declist.line());
				ids = N_declist.getChild(ASTN_idlist, 0).getChildCount(ASTN_L_id);

				for (l = 0; l < ids; l++){
					if ( i == argCountByDec) {
						return false;
					}
					if (getArg(i) != declistType) {
						return false;
					}
					i++;
				}
			}
			if ( i != argCountByDec ) {
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

ProcTab::ProcTab() {
	tab = NULL;
	checkDef = false;
}

ProcTab::~ProcTab() {
	_procTab *ptr;
	while( tab != NULL) {
		ptr = tab;
		tab = tab -> next;
		if (ptr -> proc.define().isNull() == true) {
			if (checkDef) {
				semanticError("procedure declared but not defined!", ptr -> proc.declar().line());
			}
		}
		delete ptr;
	}
	ASTfree(&ASTroot);
	SemanticNode::isfree = true;
}

_procTab::_procTab(char *s) : proc(s) {
	next = NULL;
}

Ice9Proc *ProcTab::ProcessProcAndForwardNode(SemanticNode nd) {
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
			if (!(theProc -> typeEq(nd))) {
				semanticError("procedure return type or argument types do not match previous declaration", nd.line());
			}
			if (theProc -> procReturn.isvoid == false) {
				varTab.push(procName, theProc->procReturn.type, nd, nd.line());
				varJumper.addType(procName, procName,"0");
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
					currentType = typeTab.getType(N_declist.getChild(ASTN_L_id, 0).idValue(), N_declist.line());
					unsigned j, num_id;
					num_id = N_idlist.getChildCount(ASTN_L_id);
					for (j = 0; j < num_id; j++) {
						SemanticNode N_id;
						N_id = N_idlist.getChild(ASTN_L_id, j);
						varTab.push(N_id.idValue(), currentType, nd, N_idlist.line());
						if (currentType.dim == 0) {
							varJumper.addType(procName, N_id.idValue(),"0");
						}
						else {
							varJumper.addType(procName, N_id.idValue(), N_declist.getChild(ASTN_L_id,0).idValue());
						}
					}
				}
			}
		}
		return theProc;
	}
	_procTab *newProcTab;
	if (tab == NULL) {
		tab = new _procTab(procName);
		newProcTab = tab;
	}
	else {
		newProcTab = tab;
		while(newProcTab -> next != NULL) {
			newProcTab = newProcTab -> next;
		}
		newProcTab -> next = new _procTab(procName);
		newProcTab = newProcTab -> next;
		newProcTab -> next = NULL;
	}
	Ice9Proc *newProc;
	newProc = &(newProcTab -> proc);
	switch(ndtp) {
	case ASTN_forward:
		newProc -> setDeclar(nd);
		if (nd.getChildCount(ASTN_L_id) > 1) {
			newProc -> setReturn(typeTab.getType(nd.getChild(ASTN_L_id, 1).idValue(),nd.line()));
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
				currentType = typeTab.getType(N_declist.getChild(ASTN_L_id, 0).idValue(), N_declist.line());
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
			newProc -> setReturn(typeTab.getType(nd.getChild(ASTN_L_id,1).idValue(),nd.line()));
			if (newProc -> procReturn.type.dim != 0) {
				semanticError("procedure cannot return an array!",nd.line());
			}
		}
		if (newProc -> procReturn.isvoid == false) {
			varTab.push(procName, newProc->procReturn.type, nd, nd.line());
			varJumper.addType(procName, procName, "0");
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
				currentType = typeTab.getType(N_declist.getChild(ASTN_L_id, 0).idValue(), N_declist.line());
				unsigned j, num_id;
				num_id = N_idlist.getChildCount(ASTN_L_id);
				for (j = 0; j < num_id; j++) {
					SemanticNode N_id;
					N_id = N_idlist.getChild(ASTN_L_id, j);
					varTab.push(N_id.idValue(), currentType, nd, N_idlist.line());
					if (currentType.dim == 0) {
						varJumper.addType(procName, N_id.idValue(),"0");
					}
					else {
						varJumper.addType(procName, N_id.idValue(), N_declist.getChild(ASTN_L_id,0).idValue());
					}
					newProc -> addArg(currentType);
				}
			}
		}
		break;
	}
	return newProc;
}

void ProcTab::ProcessProccallNode(SemanticNode nd) {
	assert(nd.type() == ASTN_proccall);
	if (exist(nd.getChild(ASTN_L_id, 0).idValue()) == false) {
		semanticError("Try to call a procedure which is neither defined nor declared",nd.line());
	}
	if (getProc(nd.getChild(ASTN_L_id, 0).idValue()) -> typeEq(nd) == false) {
		semanticError("Arguments type mismatch between procedure declaration and procedure call",nd.line());
	}
}

Ice9Proc *ProcTab::getProc(char *s) {
	assert(s != NULL);
	_procTab *ptr;
	ptr = tab;
	while (ptr != NULL) {
		if (strcmp( ptr->proc.name, s) == 0) {
			break;
		}
		ptr = ptr -> next;
	}
	return &(ptr->proc);
}

bool ProcTab::exist(char *s) {
	assert(s != NULL);
	_procTab *ptr;
	ptr = tab;
	while (ptr != NULL) {
		if (strcmp( ptr->proc.name, s) == 0) {
			return true;
		}
		ptr = ptr -> next;
	}
	return false;
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


char * namedmp(char * s) {
	// has hard copy @ ice9.l
	char *str;
	unsigned i;
	for (i = 0; *(s + i) != '\0'; i++) {
		;
	}
	str = new char[i + 1];
	i = 0;
	while (*(s + i) != '\0') {
		*(str + i) = *(s + i);
		i++;
	}
	*(str + i) = '\0';
	return str;
}

VarTypeTab::VarTypeTab(_tabType t) {
	tabType = t;
	tab = NULL;
	if (tabType == TYPE_TABLE) {
		SemanticNode outermost;
		Ice9Type ice9type;
		ice9type.base = ice9int;
		ice9type.dim = 0;
		push("int",ice9type,outermost,-1);
		ice9type.base = ice9str;
		push("string",ice9type,outermost,-1);
		ice9type.base = ice9bool;
		push("bool",ice9type,outermost,-1);
	}
}

VarTypeTab::~VarTypeTab() {
	if (tab == NULL) {
		return;
	}
	_typeTab *ptr;
	ptr = tab;

	while( tab != NULL) {
		ptr = tab;
		tab = tab -> next;
		delete [](ptr -> name);
		_visibleStack *vsptr,*ptr2;
		vsptr = ptr2 = ptr -> visibleStack.next;
		while (vsptr != NULL) {
			ptr2 = vsptr;
			vsptr = vsptr -> next;
			delete ptr2;
		}
		delete ptr;
	}
}

bool SemanticNode::eq(SemanticNode sn2) {
	return data == sn2.data;
}

bool operator==(SemanticNode sn1, SemanticNode sn2) {
	return sn1.eq(sn2);
}

bool operator!=(SemanticNode sn1, SemanticNode sn2) {
	return !sn1.eq(sn2);
}

void VarTypeTab::push(char *s, Ice9Type tp, SemanticNode node, long line) {
	_typeTab *ptr = NULL;
	ptr = tab;

	while (ptr != NULL) {
		if (strcmp(s, ptr->name) == 0) {
			if ( node == ptr -> visibleStack.visibleWithin ) {
				if (tabType == VAR_TABLE) {
					semanticError("Dulplicate variable declaration",line);
				}
				else {
					semanticError("Dulplicate type defination",line);
				}
			}
			_visibleStack *vsptr;
			vsptr = new _visibleStack;
			vsptr -> next = ptr -> visibleStack.next;
			vsptr -> visibleWithin = ptr -> visibleStack.visibleWithin;
			vsptr -> type = ptr -> visibleStack.type;
			ptr -> visibleStack.next = vsptr;
			ptr -> visibleStack.type = tp;
			ptr -> visibleStack.visibleWithin = node;
			return;
		}
		ptr = ptr -> next;
	}
	ptr = new _typeTab;
	ptr -> next = tab;
	tab = ptr;
	ptr -> name = namedmp(s);
	ptr -> visibleStack.next = NULL;
	ptr -> visibleStack.visibleWithin = node;
	ptr -> visibleStack.type = tp;
}

void VarTypeTab::popCorresponding(SemanticNode leaving) {
	_typeTab *ptr;
	if (tab == NULL) {
		return;
	}
	ptr = tab -> next;
	if (tab -> visibleStack.visibleWithin == leaving) {
		if (tab -> visibleStack.next == NULL) {
			delete [](tab -> name);
			delete tab;
			tab = ptr;
		}
		else {
			tab -> visibleStack.type = tab -> visibleStack.next -> type;
			tab -> visibleStack.visibleWithin = tab -> visibleStack.next -> visibleWithin;
			_visibleStack *p;
			p = tab -> visibleStack.next -> next;
			delete (tab -> visibleStack.next);
			tab -> visibleStack.next = p;
		}
		return;
	}
	ptr = tab;
	while (ptr -> next != NULL) {
		if (ptr -> next -> visibleStack.visibleWithin == leaving) {
			if (ptr -> next -> visibleStack.next == NULL) {
				delete [](ptr -> next -> name);
				_typeTab *d;
				d = ptr -> next;
				ptr -> next = ptr -> next -> next;
				delete d;
			}
			else {
				ptr -> next -> visibleStack.type = ptr -> next -> visibleStack.next -> type;
				ptr -> next -> visibleStack.visibleWithin = ptr -> next -> visibleStack.next -> visibleWithin;
				_visibleStack *p;
				p = ptr -> next -> visibleStack.next -> next;
				delete (ptr -> next -> visibleStack.next);
				ptr -> next -> visibleStack.next = p;
			}
		}
		ptr = ptr->next;;
	}
}

Ice9Type VarTypeTab::getType(char *s, long line) {
	_typeTab *ptr;
	ptr = tab;
	while (ptr != NULL) {
		if (strcmp(s,ptr -> name) == 0) {
			break;
		}
		ptr = ptr -> next;
	}
	if (ptr == NULL) {
		if (tabType == VAR_TABLE) {
			semanticError("Variable not declared!", line);
		}
		else {
			semanticError("Type not defined!", line);
		}
	}
	return ptr -> visibleStack.type;
}

bool VarTypeTab::isFaCounter(char *s) {
	_typeTab *ptr;
	ptr = tab;
	while (ptr != NULL) {
		if (strcmp(s,ptr -> name) == 0) {
			break;
		}
		ptr = ptr -> next;
	}
	return (ptr -> visibleStack.visibleWithin.type() == ASTN_fa);
}
/*
enum _tabType{
	VAR_TABLE,
	TYPE_TABLE
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
	_tabType tabType;
public:
	VarTypeTab(_tabType t);
	~VarTypeTab();
	void push(char *s, Ice9Type tp, SemanticNode node, long line);
	void popCorresponding(SemanticNode leaving);
	Ice9Type getType(char *s, long line);
};
*/

Ice9Type getTypeGeneral(SemanticNode nd) {
	Ice9Type r,r1,r2;
	unsigned d,i;
#define OPTION_A 1
	switch(nd.type()) {
	case ASTN_exp:
		assert(nd.getChildCount() == 1);
#ifdef OPTION_A
		if (nd.getChild(0).type() == ASTN_L_id) {
			return typeTab.getType(nd.getChild(0).idValue(), nd.line());
		}
#endif
		return getTypeGeneral(nd.getChild(0));
	case ASTN_L_int:
		return INT;
	case ASTN_L_string:
		return STR;
	case ASTN_L_id:
#ifndef OPTION_A
		assert(false); 
		//special case, should be handled with parent ASTN_exp node.
		//so as to show error msg correctly
		//see case ASTN_exp branch.
#endif
		return typeTab.getType(nd.idValue(),nd.line());
	case ASTN_L_bool:
		return BOOLEAN;
	case ASTN_lvalue:
		Ice9Type lvalueIdType;
		lvalueIdType = varTab.getType(nd.getChild(ASTN_L_id, 0).idValue(), nd.line());
		d = nd.getChildCount(ASTN_exp);
		if (lvalueIdType.dim < d) {
			semanticError("Too many subscript for the array", nd.line());
		}
		for(i = 0; i < d; i++) {
			if(getTypeGeneral(nd.getChild(ASTN_exp, i)) != INT) {
				semanticError("Subscript must be integer", nd.line());
			}
		}
		lvalueIdType.dim -= d;
		return lvalueIdType;
	case ASTN_read:
		return INT;
	case ASTN_umin:
		d = nd.getChildCount(ASTN_exp);
		assert(d == 1);	
		r = getTypeGeneral(nd.getChild(ASTN_exp, 0));
		if (r != INT && r != BOOLEAN) {
			semanticError("unary minus only accept int and bool", nd.line());
		}
		if (r == INT) {
			nd.reload(ice9int);
		}
		if (r == BOOLEAN) {
			nd.reload(ice9bool);
		}
		return r;
	case ASTN_plus:
		d = nd.getChildCount(ASTN_exp);
		assert(d == 2);	
		r1 = getTypeGeneral(nd.getChild(ASTN_exp, 0));
		r2 = getTypeGeneral(nd.getChild(ASTN_exp, 1));
		if (r1 != BOOLEAN && r1 != INT) {
			semanticError("Plus operator only accept bool or int", nd.line());
		}
		if (r2 != BOOLEAN && r2 != INT) {
			semanticError("Plus operator only accept bool or int", nd.line());
		}
		if(r1 != r2) {
			semanticError("Plus operator: RHS and LHS type not match", nd.line());
		}
		if (r1 == INT) {
			nd.reload(ice9int);
		}
		if (r1 == BOOLEAN) {
			nd.reload(ice9bool);
		}
		return r1;
	case ASTN_minus:
		d = nd.getChildCount(ASTN_exp);
		assert(d == 2);	
		r1 = getTypeGeneral(nd.getChild(ASTN_exp, 0));
		r2 = getTypeGeneral(nd.getChild(ASTN_exp, 1));
		if (r2 != INT || r1 != INT) {
			semanticError("Minus operator only accept int", nd.line());
		}
		return INT;
	case ASTN_quest:
		if(getTypeGeneral(nd.getChild(ASTN_exp, 0)) != BOOLEAN) {
			 semanticError("Quest operator only accept bool", nd.line());
		}
		return INT;
	case ASTN_proccall:
		procTab.ProcessProccallNode(nd);
		return procTab.getProc(nd.getChild(ASTN_L_id, 0).idValue()) -> getReturnType();
	case ASTN_star:
		d = nd.getChildCount(ASTN_exp);
		assert(d == 2);	
		r1 = getTypeGeneral(nd.getChild(ASTN_exp, 0));
		r2 = getTypeGeneral(nd.getChild(ASTN_exp, 1));
		if (r1 != BOOLEAN && r1 != INT) {
			semanticError("Star operator only accept bool or int", nd.line());
		}
		if (r2 != BOOLEAN && r2 != INT) {
			semanticError("Star operator only accept bool or int", nd.line());
		}
		if(r1 != r2) {
			semanticError("Star operator: RHS and LHS type not match", nd.line());
		}
		if (r1 == INT) {
			nd.reload(ice9int);
		}
		if (r1 == BOOLEAN) {
			nd.reload(ice9bool);
		}
		return r1;
	case ASTN_div:
		d = nd.getChildCount(ASTN_exp);
		assert(d == 2);	
		r1 = getTypeGeneral(nd.getChild(ASTN_exp, 0));
		r2 = getTypeGeneral(nd.getChild(ASTN_exp, 1));
		if (r2 != INT || r1 != INT) {
			semanticError("Div operator only accept int", nd.line());
		}
		return INT;
	case ASTN_mod:
		d = nd.getChildCount(ASTN_exp);
		assert(d == 2);	
		r1 = getTypeGeneral(nd.getChild(ASTN_exp, 0));
		r2 = getTypeGeneral(nd.getChild(ASTN_exp, 1));
		if (r2 != INT || r1 != INT) {
			semanticError("Mod operator only accept int", nd.line());
		}
		return INT;
	case ASTN_eq:
	case ASTN_neq:
		d = nd.getChildCount(ASTN_exp);
		assert(d == 2);	
		r1 = getTypeGeneral(nd.getChild(ASTN_exp, 0));
		r2 = getTypeGeneral(nd.getChild(ASTN_exp, 1));
		if (r1 != BOOLEAN && r1 != INT) {
			semanticError("Equal/Not-equal operator only accept bool or int", nd.line());
		}
		if (r2 != BOOLEAN && r2 != INT) {
			semanticError("Equal/Not-equal operator only accept bool or int", nd.line());
		}
		if(r1 != r2) {
			semanticError("Equal/Not-equal operator: RHS and LHS type not match", nd.line());
		}
		if (r1 == INT) {
			nd.reload(ice9int);
		}
		if (r1 == BOOLEAN) {
			nd.reload(ice9bool);
		}
		return BOOLEAN;
	case ASTN_gt:
	case ASTN_lt:
	case ASTN_ge:
	case ASTN_le:
		d = nd.getChildCount(ASTN_exp);
		assert(d == 2);	
		r1 = getTypeGeneral(nd.getChild(ASTN_exp, 0));
		r2 = getTypeGeneral(nd.getChild(ASTN_exp, 1));
		if (r2 != INT || r1 != INT) {
			semanticError("Comparison operators: < > <= >= only accept int", nd.line());
		}
		return BOOLEAN;
	case ASTN_typedes:
		d = nd.getChildCount(ASTN_L_int);
		r = typeTab.getType(nd.getChild(ASTN_L_id,0).idValue(), nd.line());
		r.dim += d;
		return r;
	default:
		assert(false);
	}
	return r;//dummy return
}



void travNode(SemanticNode nd) {
	nodeCheckIn(nd);
	SemanticNode x;
	x.data = nd.data -> child;
	if (x.data != NULL) {
		travNode(x);
	}
	nodeCheckOut(nd);
	x.data = nd.data -> brother;
	if (x.data != NULL) {
		travNode(x);
	}
}

int semanticCheck(SemanticTree tr) {
	current_scope = *(SemanticNode *)&tr;
	travNode(current_scope);
	procTab.checkDef = true;
	return 0;
}

bool SemanticNode::gt(SemanticNode sn2) {
	return ((unsigned)(data))>((unsigned)(sn2.data));
}

bool operator<=(SemanticNode sn1, SemanticNode sn2) {
	return sn2.gt(sn1) || sn2.eq(sn1);
}

bool operator<(SemanticNode sn1, SemanticNode sn2) {
	return sn2.gt(sn1);
}

bool operator>=(SemanticNode sn1, SemanticNode sn2) {
	return sn1.gt(sn2) || sn1.eq(sn2);
}

bool operator>(SemanticNode sn1, SemanticNode sn2) {
	return sn1.gt(sn2);
}

void SemanticNode::reload(Ice9BaseType tp) {
	if (tp == ice9int) {
		data -> reloadType = RELOAD_INT;
	}
	else if (tp == ice9bool) {
		data -> reloadType = RELOAD_BOOL;
	}
	assert(false);
}

Ice9BaseType SemanticNode::reload() {
	if (data -> reloadType == RELOAD_INT) {
		return ice9int;
	}
	else if (data -> reloadType == RELOAD_BOOL) {
		return ice9bool;
	}
	assert(false);
}
