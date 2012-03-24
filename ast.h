#ifndef _AST_H_
#define _AST_H_

typedef enum {
	ASTN_stms,
	ASTN_decs,
	ASTN_stm,
	ASTN_if,
	ASTN_branch,
	ASTN_do,
	ASTN_fa,
	ASTN_proc,
	ASTN_dec1s,
	ASTN_idlist,
	ASTN_cids,
	ASTN_var,
	ASTN_varlist,
	ASTN_dim,
	ASTN_indexs,
	ASTN_varlists,
	ASTN_forward,
	ASTN_type,
	ASTN_declistx,
	ASTN_declist,
	ASTN_lvalue,
	ASTN_exp,
	ASTN_program,
	ASTN_L_break,
	ASTN_L_exit,
	ASTN_L_return,
	ASTN_assign,
	ASTN_write,
	ASTN_writes,
	ASTN_L_id,
	ASTN_L_int,
	ASTN_L_string,
	ASTN_L_bool,
	ASTN_typedes,
	ASTN_read,
	ASTN_umin,
	ASTN_plus,
	ASTN_minus,
	ASTN_star,
	ASTN_div,
	ASTN_quest,
	ASTN_mod,
	ASTN_eq,
	ASTN_neq,
	ASTN_ge,
	ASTN_le,
	ASTN_lt,
	ASTN_gt,
	ASTN_proccall
} NodeType;

typedef enum {TRUE, FALSE} BOOL;

typedef struct _AST {
	struct _AST *child;
	struct _AST *brother;
	NodeType sym;
	long lineno;
	union {
		char *str;
		int num;
		BOOL b;
	} value;
} AST;

AST *newNode(NodeType t, int l);
AST *appendChild(AST *parent, AST * child);
AST *insertChild(AST *parent, AST * child);
AST *appendBrother(AST *leftNode, AST *tobeappend);
AST *appendBrotherAtLast(AST *leftNode, AST *tobeappend);

AST *newIdNode(char *va);
AST *newIntNode(int va);
AST *newBoolNode(BOOL va);
AST *newStrNode(char *va);

#endif
