%{
#include <stdio.h>
#include <stdlib.h>
#include "ast.h"
#define YYDEBUG 1
extern int yynewlines;
extern char *yytext;

int yylex(void); /* function prototype */

void yyerror(char *s)
{
  if ( *yytext == '\0' )
    fprintf(stderr, "line %d: %s near end of file\n", 
	    yynewlines,s);
  else
    fprintf(stderr, "line %d: %s near %s\n",
	    yynewlines, s, yytext);
  exit(yynewlines);
}

%}

%union {
  int intt;
  char *str;
  AST *node;
}

%token <intt> TK_IF
%token TK_FI
%token TK_ELSE
%token <intt> TK_DO
%token TK_OD
%token <intt> TK_FA
%token TK_AF
%token TK_TO
%token <intt> TK_PROC
%token TK_END
%token TK_RETURN
%token <intt> TK_FORWARD
%token <intt> TK_VAR
%token <intt> TK_TYPE
%token TK_BREAK
%token TK_EXIT
%token TK_TRUE
%token TK_FALSE
%token TK_WRITE
%token TK_WRITES
%token <intt> TK_READ
%token <intt> TK_BOX
%token TK_ARROW
%token TK_LPAREN
%token TK_RPAREN
%token TK_LBRACK
%token TK_RBRACK
%token <intt> TK_COLON
%token <intt> TK_SEMI
%token TK_ASSIGN
%token TK_COMMA

%left <intt> TK_PLUS TK_MINUS

%left <intt> TK_STAR TK_SLASH TK_MOD

%right <intt> TK_QUEST UMINUS

%nonassoc <intt> TK_EQ TK_NEQ TK_GT TK_LT TK_GE TK_LE

%token <str> TK_SLIT
%token <intt> TK_INT
%token <str> TK_ID

%type <node> NT_stms
%type <node> NT_dec
%type <node> NT_decs
%type <node> NT_stm
%type <node> NT_if
%type <node> NT_branches
%type <node> NT_do
%type <node> NT_fa
%type <node> NT_proc
%type <node> NT_dec1
%type <node> NT_dec1s
%type <node> NT_idlist
%type <node> NT_cids
%type <node> NT_var
%type <node> NT_varlist
%type <node> NT_dim
%type <node> NT_indexs
%type <node> NT_varlists
%type <node> NT_forward
%type <node> NT_type
%type <node> NT_declistx
%type <node> NT_lvalue
%type <node> NT_cexps
%type <node> NT_exp

%start program

%%

program: /* empty */ 
	{
		ASTroot.child = NULL;
		ASTroot.brother = NULL;
		ASTroot.sym = ASTN_program;
		ASTroot.lineno = 0;
	}
	| NT_stms
	{
		ASTroot.child = NULL;
		ASTroot.brother = NULL;
		ASTroot.sym = ASTN_program;
		appendChild(&ASTroot, $1);
		ASTroot.lineno = $1 -> lineno;
	}
        | NT_decs
	{
		ASTroot.child = NULL;
		ASTroot.brother = NULL;
		ASTroot.sym = ASTN_program;
		appendChild(&ASTroot, $1);
		ASTroot.lineno = $1 -> lineno;
	}
	| NT_decs NT_stms
	{
		ASTroot.child = NULL;
		ASTroot.brother = NULL;
		ASTroot.sym = ASTN_program;
		appendChild(&ASTroot, $1);
		appendChild(&ASTroot, $2);
		ASTroot.lineno = $1 -> lineno;
	}
	;

NT_stms:  NT_stm
	{
		$$ = newNode(ASTN_stms, $1->lineno);
		appendChild($$, $1);
	}
	| NT_stms NT_stm
	{
		appendChild($1, $2);
		$$ = $1; 
	}
	;	

NT_dec:	  NT_var
	{
		$$ = $1;
	}
	| NT_type
	{
		$$ = $1;
	}
	| NT_forward
	{
		$$ = $1;
	}
	| NT_proc
	{
		$$ = $1;
	}
	;

NT_decs:  NT_dec
	{
		$$ = newNode(ASTN_decs, $1 -> lineno);
		appendChild($$, $1);
	}
       	| NT_dec NT_decs
	{
		insertChild($2, $1);
		$$ = $2;
		$$ -> lineno = $1 -> lineno;
	}
	;

NT_stm:	  NT_if
	{
		$$ = newNode(ASTN_stm, $1->lineno);
		appendChild($$, $1);
	}
      	| NT_do
	{
		$$ = newNode(ASTN_stm, $1->lineno);
		appendChild($$, $1);
	}
	| NT_fa
	{
		$$ = newNode(ASTN_stm, $1->lineno);
		appendChild($$, $1);
	}
	| TK_BREAK TK_SEMI
	{
		$$ = newNode(ASTN_stm, $2);
		appendChild($$, newNode(ASTN_L_break, $2));
	}
	| TK_EXIT TK_SEMI
	{
		$$ = newNode(ASTN_stm, $2);
		appendChild($$, newNode(ASTN_L_exit, $2));
	}
	| TK_RETURN TK_SEMI
	{
		$$ = newNode(ASTN_stm, $2);
		appendChild($$, newNode(ASTN_L_return, $2));
	}
	| NT_lvalue TK_ASSIGN NT_exp TK_SEMI
	{
		$$ = newNode(ASTN_stm, $4);
		appendChild($$ ,appendChild(appendChild(newNode(ASTN_assign, $1->lineno), $1), $3));
	}
	| TK_WRITE NT_exp TK_SEMI
	{
		$$ = newNode(ASTN_stm, $3);
		appendChild($$, appendChild(newNode(ASTN_write, $3),$2));
	}
	| TK_WRITES NT_exp TK_SEMI
	{
		$$ = newNode(ASTN_stm, $3);
		appendChild($$, appendChild(newNode(ASTN_writes, $3),$2));
	}
	| NT_exp TK_SEMI
	{
		$$ = newNode(ASTN_stm, $2);
		appendChild($$, $1); 
	}
	| TK_SEMI
	{
		$$ = newNode(ASTN_stm, $1);
	}
	;

NT_if:	  TK_IF NT_exp TK_ARROW NT_stms NT_branches TK_FI
	{
		$$ = newNode(ASTN_if, $1);
		appendBrother($2, $4);
		appendChild($$, appendChild(newNode(ASTN_branch, $2->lineno),$2));
		appendChild($$, $5);
	}
     	| TK_IF NT_exp TK_ARROW NT_stms TK_FI
	{
		$$ = newNode(ASTN_if, $1);
		appendBrother($2, $4);
		appendChild($$, appendChild(newNode(ASTN_branch, $2->lineno), $2));
	}
	| TK_IF NT_exp TK_ARROW NT_stms TK_BOX TK_ELSE TK_ARROW NT_stms TK_FI
	{
		$$ = newNode(ASTN_if, $1);
		appendBrother($2, $4);
		appendChild($$, appendChild(newNode(ASTN_branch, $2->lineno), $2));
		appendChild($$, appendChild(newNode(ASTN_branch, $5), $8));
	}
	| TK_IF NT_exp TK_ARROW NT_stms NT_branches TK_BOX TK_ELSE TK_ARROW NT_stms TK_FI
	{
		$$ = newNode(ASTN_if, $1);
		appendBrother($2, $4);
		appendChild($$, appendChild(newNode(ASTN_branch, $2->lineno), $2));
		appendChild($$, $5);
		appendChild($$, appendChild(newNode(ASTN_branch, $6), $9));
	}
	;

NT_branches: NT_branches TK_BOX NT_exp TK_ARROW NT_stms
	{
		appendBrother($3, $5);
		appendBrotherAtLast($1, appendChild(newNode(ASTN_branch, $2), $3));
		$$ = $1;
	}
	| TK_BOX NT_exp TK_ARROW NT_stms
	{
		$$ = newNode(ASTN_branch, $1);
		appendChild($$, $2);
		appendChild($$, $4);
	}
	;

NT_do:	  TK_DO NT_exp TK_ARROW TK_OD
	{
		$$ = newNode(ASTN_do, $1);
		appendChild($$, $2);
	}
     	| TK_DO NT_exp TK_ARROW NT_stms TK_OD
	{
		$$ = newNode(ASTN_do, $1);
		appendChild($$, $2);
		appendChild($$, $4);
	}
	;

NT_fa:	  TK_FA TK_ID TK_ASSIGN NT_exp TK_TO NT_exp TK_ARROW TK_AF
	{
		$$ = newNode(ASTN_fa, $1);
		appendBrother($4, $6);
		appendChild($$, $4);
	}
     	| TK_FA TK_ID TK_ASSIGN NT_exp TK_TO NT_exp TK_ARROW NT_stms TK_AF
	{
		$$ = newNode(ASTN_fa, $1);
		appendBrother($4, $6);
		appendBrother($6, $8);
		appendChild($$, $4);
	}
	;

NT_proc:  TK_PROC TK_ID TK_LBRACK NT_declistx TK_RBRACK NT_dec1s NT_stms TK_END
	{
		$$ = newNode(ASTN_proc, $1);
		appendChild($$, newIdNode($2));
		appendBrother($4, $6);
		appendBrother($6, $7);
		appendChild($$, $4);
	}
       	| TK_PROC TK_ID TK_LBRACK NT_declistx TK_RBRACK NT_dec1s TK_END
	{
		$$ = newNode(ASTN_proc, $1);
		appendChild($$, newIdNode($2));
		appendBrother($4, $6);
		appendChild($$, $4);
	}
	| TK_PROC TK_ID TK_LBRACK NT_declistx TK_RBRACK NT_stms TK_END
	{
		$$ = newNode(ASTN_proc, $1);
		appendChild($$, newIdNode($2));
		appendBrother($4, $6);
		appendChild($$, $4);
	}
	| TK_PROC TK_ID TK_LBRACK NT_declistx TK_RBRACK TK_END
	{
		$$ = newNode(ASTN_proc, $1);
		appendChild($$, newIdNode($2));
		appendChild($$, $4);
	}
	| TK_PROC TK_ID TK_LBRACK NT_declistx TK_RBRACK TK_COLON TK_ID NT_dec1s NT_stms TK_END
	{
		$$ = newNode(ASTN_proc, $1);
		appendChild($$, newIdNode($2));
		appendBrother($4, newIdNode($7));
		appendChild($$, $4);
		appendBrother($8, $9);
		appendChild($$, $8);
	}
	| TK_PROC TK_ID TK_LBRACK NT_declistx TK_RBRACK TK_COLON TK_ID NT_dec1s TK_END
	{
		$$ = newNode(ASTN_proc, $1);
		appendChild($$, newIdNode($2));
		appendBrother($4, newIdNode($7));
		appendChild($$, $4);
		appendChild($$, $8);
	}
	| TK_PROC TK_ID TK_LBRACK NT_declistx TK_RBRACK TK_COLON TK_ID NT_stms TK_END
	{
		$$ = newNode(ASTN_proc, $1);
		appendChild($$, newIdNode($2));
		appendBrother($4, newIdNode($7));
		appendChild($$, $4);
		appendChild($$, $8);
	}
	| TK_PROC TK_ID TK_LBRACK NT_declistx TK_RBRACK TK_COLON TK_ID TK_END
	{
		$$ = newNode(ASTN_proc, $1);
		appendChild($$, newIdNode($2));
		appendBrother($4, newIdNode($7));
		appendChild($$, $4);
	}
	| TK_PROC TK_ID TK_LBRACK TK_RBRACK NT_dec1s NT_stms TK_END
	{
		$$ = newNode(ASTN_proc, $1);
		appendChild($$, newIdNode($2));
		appendBrother($5, $6);
		appendChild($$, $5);
	}
       	| TK_PROC TK_ID TK_LBRACK TK_RBRACK NT_dec1s TK_END
	{
		$$ = newNode(ASTN_proc, $1);
		appendChild($$, newIdNode($2));
		appendChild($$, $5);
	}
	| TK_PROC TK_ID TK_LBRACK TK_RBRACK NT_stms TK_END
	{
		$$ = newNode(ASTN_proc, $1);
		appendChild($$, newIdNode($2));
		appendChild($$, $5);
	}
	| TK_PROC TK_ID TK_LBRACK TK_RBRACK TK_END
	{
		$$ = newNode(ASTN_proc, $1);
		appendChild($$, newIdNode($2));
	}
	| TK_PROC TK_ID TK_LBRACK TK_RBRACK TK_COLON TK_ID NT_dec1s NT_stms TK_END
	{
		$$ = newNode(ASTN_proc, $1);
		appendChild($$, newIdNode($2));
		appendChild($$, newIdNode($6));
		appendBrother($7, $8);
		appendChild($$, $7);
	}
	| TK_PROC TK_ID TK_LBRACK TK_RBRACK TK_COLON TK_ID NT_dec1s TK_END
	{
		$$ = newNode(ASTN_proc, $1);
		appendChild($$, newIdNode($2));
		appendChild($$, newIdNode($6));
		appendChild($$, $7);
	}
	| TK_PROC TK_ID TK_LBRACK TK_RBRACK TK_COLON TK_ID NT_stms TK_END
	{
		$$ = newNode(ASTN_proc, $1);
		appendChild($$, newIdNode($2));
		appendChild($$, newIdNode($6));
		appendChild($$, $7);
	}
	| TK_PROC TK_ID TK_LBRACK TK_RBRACK TK_COLON TK_ID TK_END
	{
		$$ = newNode(ASTN_proc, $1);
		appendChild($$, newIdNode($2));
		appendChild($$, newIdNode($6));
	}
	;

NT_dec1:  NT_var
	{
		$$ = $1;
	}
       	| NT_type
	{
		$$ = $1;
	}
	;

NT_dec1s: NT_dec1
	{
		$$ = newNode(ASTN_dec1s, $1->lineno);
		appendChild($$, $1);
	}
	| NT_dec1s NT_dec1
	{
		appendChild($1, $2);
		$$ = $1;
	}
	;

NT_idlist: TK_ID 
	{
		$$ = newNode(ASTN_idlist, yynewlines);
		appendChild($$, newIdNode($1));
	}
	| TK_ID NT_cids
	{
		$$ = newNode(ASTN_idlist, yynewlines);
		appendChild($$, newIdNode($1));
		appendChild($$, $2);
	}
	;

NT_cids:  TK_COMMA TK_ID
	{
		$$ = newIdNode($2);
	}
     	| NT_cids TK_COMMA TK_ID
	{
		appendBrotherAtLast($1, newIdNode($3));
		$$ = $1;
	}
	;

NT_var:	  TK_VAR NT_varlists TK_SEMI 
	{
		$$ = $2;
	}
      	;

NT_varlist: NT_idlist TK_COLON TK_ID NT_dim
	{
		$$ = newNode(ASTN_varlist, $2);
		appendChild($$, $1);
		appendChild($$, appendChild(appendChild(newNode(ASTN_typedes, $2), newIdNode($3)), $4));
	}
	| NT_idlist TK_COLON TK_ID
	{
		$$ = newNode(ASTN_varlist, $2);
		appendChild($$, $1);
		appendChild($$, appendChild(newNode(ASTN_typedes, $2), newIdNode($3)));
	}
	;

NT_dim: TK_LPAREN TK_INT TK_RPAREN
	{
		$$ = newIntNode($2);
	}
	| NT_dim TK_LPAREN TK_INT TK_RPAREN
	{
		$$ = $1;
		appendBrotherAtLast($$, newIntNode($3));
	}
	;

NT_indexs: TK_LPAREN NT_exp TK_RPAREN
	{
		$$ = $2;
	}
	| NT_indexs TK_LPAREN NT_exp TK_RPAREN
	{
		$$ = $1;
		appendBrotherAtLast($$, $3);
	}
	;

NT_varlists: NT_varlist
	{
		$$ = $1;
	}
	| NT_varlists TK_COMMA NT_varlist
	{
		$$ = $1;
		appendBrotherAtLast($$, $3);
	}
	;

NT_forward: TK_FORWARD TK_ID TK_LBRACK NT_declistx TK_RBRACK TK_SEMI
	{
		$$ = newNode(ASTN_forward, $1);
		appendChild($$, newIdNode($2));
		appendChild($$, $4);
	}
	| TK_FORWARD TK_ID TK_LBRACK NT_declistx TK_RBRACK TK_COLON TK_ID TK_SEMI
	{
		$$ = newNode(ASTN_forward, $1);
		appendChild($$, newIdNode($2));
		appendChild($$, $4);
		appendChild($$, newIdNode($7));
	}
	| TK_FORWARD TK_ID TK_LBRACK TK_RBRACK TK_SEMI
	{
		$$ = newNode(ASTN_forward, $1);
		appendChild($$, newIdNode($2));
	}
	| TK_FORWARD TK_ID TK_LBRACK TK_RBRACK TK_COLON TK_ID TK_SEMI
	{
		$$ = newNode(ASTN_forward, $1);
		appendChild($$, newIdNode($2));
		appendChild($$, newIdNode($6));
	}
	;

NT_type:  TK_TYPE TK_ID TK_EQ TK_ID NT_dim TK_SEMI
	{
		$$ = newNode(ASTN_type, $1);
		appendChild($$, newIdNode($2));
		appendChild($$, appendChild(appendChild(newNode(ASTN_typedes, -1), newIdNode($4)),$5));
	}
	| TK_TYPE TK_ID TK_EQ TK_ID TK_SEMI
	{
		$$ = newNode(ASTN_type, $1);
		appendChild($$, newIdNode($2));
		appendChild($$, appendChild(newNode(ASTN_typedes, -1), newIdNode($4)));
	}
	;

NT_declistx: NT_idlist TK_COLON TK_ID
	{
		$$ = newNode(ASTN_declistx, $2);
		appendChild($$, appendChild(appendChild(newNode(ASTN_declist, $2), $1), newIdNode($3)));
	}
	| NT_declistx TK_COMMA NT_idlist TK_COLON TK_ID
	{
		appendChild($1, appendChild(appendChild(newNode(ASTN_declist, $4), $3), newIdNode($5)));
		$$ = $1;
	}
	;

NT_lvalue: TK_ID
	{
		$$ = newNode(ASTN_lvalue, yynewlines);
		appendChild($$, newIdNode($1));
	}
	| TK_ID NT_indexs
	{
		$$ = newNode(ASTN_lvalue, yynewlines);
		appendChild($$, newIdNode($1));
		appendChild($$, $2);
	}
	;

NT_cexps: TK_COMMA NT_exp
	{
		$$ = $2;
	}
	| NT_cexps TK_COMMA NT_exp
	{
		$$ = $1;
		appendBrotherAtLast($$, $3);
	}
	;

NT_exp:	  NT_lvalue
	{
		$$ = newNode(ASTN_exp, $1 -> lineno);
		appendChild($$, $1);
	}
      	| TK_INT
	{
		$$ = newNode(ASTN_exp, yynewlines);
		appendChild($$, newIntNode($1));
	}
	| TK_TRUE
	{
		$$ = newNode(ASTN_exp, yynewlines);
		appendChild($$, newBoolNode(TRUE));
	}
	| TK_FALSE
	{
		$$ = newNode(ASTN_exp, yynewlines);
		appendChild($$, newBoolNode(FALSE));
	}
	| TK_SLIT
	{
		$$ = newNode(ASTN_exp, yynewlines);
		appendChild($$, newStrNode($1));
	}
	| TK_READ
	{
		$$ = newNode(ASTN_exp, $1);
		appendChild($$, newNode(ASTN_read, $1));
	}
	| TK_MINUS NT_exp %prec UMINUS
	{
		$$ = newNode(ASTN_exp, $1);
		appendChild($$, appendChild(newNode(ASTN_umin, $1), $2));
	}
	| TK_QUEST NT_exp
	{
		$$ = newNode(ASTN_exp, $1);
		appendChild($$, appendChild(newNode(ASTN_quest, $1), $2));
	}
	| TK_ID TK_LBRACK TK_RBRACK
	{
		$$ = newNode(ASTN_exp, yynewlines);
		appendChild($$, appendChild(newNode(ASTN_proccall, yynewlines),newIdNode($1)));
	}
	| TK_ID TK_LBRACK NT_exp NT_cexps TK_RBRACK
	{
		$$ = newNode(ASTN_exp, yynewlines);
		appendBrother($3, $4);
		appendChild($$, appendChild(appendChild(newNode(ASTN_proccall, yynewlines),newIdNode($1)),$3));
	}
	| TK_ID TK_LBRACK NT_exp TK_RBRACK
	{
		$$ = newNode(ASTN_exp, yynewlines);
		appendChild($$, appendChild(appendChild(newNode(ASTN_proccall, yynewlines),newIdNode($1)),$3));
	}
	| NT_exp TK_PLUS NT_exp
	{
		$$ = newNode(ASTN_exp, $2);
		appendChild($$, appendChild(appendChild(newNode(ASTN_plus, $2), $1), $3));
	}
	| NT_exp TK_MINUS NT_exp
	{
		$$ = newNode(ASTN_exp, $2);
		appendChild($$, appendChild(appendChild(newNode(ASTN_minus, $2), $1), $3));
	}
	| NT_exp TK_STAR NT_exp
	{
		$$ = newNode(ASTN_exp, $2);
		appendChild($$, appendChild(appendChild(newNode(ASTN_star, $2), $1), $3));
	}
	| NT_exp TK_SLASH NT_exp
	{
		$$ = newNode(ASTN_exp, $2);
		appendChild($$, appendChild(appendChild(newNode(ASTN_div, $2), $1), $3));
	}
	| NT_exp TK_MOD NT_exp
	{
		$$ = newNode(ASTN_exp, $2);
		appendChild($$, appendChild(appendChild(newNode(ASTN_mod, $2), $1), $3));
	}
	| NT_exp TK_EQ NT_exp
	{
		$$ = newNode(ASTN_exp, $2);
		appendChild($$, appendChild(appendChild(newNode(ASTN_eq, $2), $1), $3));
	}
	| NT_exp TK_NEQ NT_exp
	{
		$$ = newNode(ASTN_exp, $2);
		appendChild($$, appendChild(appendChild(newNode(ASTN_neq, $2), $1), $3));
	}
	| NT_exp TK_GT NT_exp
	{
		$$ = newNode(ASTN_exp, $2);
		appendChild($$, appendChild(appendChild(newNode(ASTN_gt, $2), $1), $3));
	}
	| NT_exp TK_LT NT_exp
	{
		$$ = newNode(ASTN_exp, $2);
		appendChild($$, appendChild(appendChild(newNode(ASTN_lt, $2), $1), $3));
	}
	| NT_exp TK_GE NT_exp
	{
		$$ = newNode(ASTN_exp, $2);
		appendChild($$, appendChild(appendChild(newNode(ASTN_ge, $2), $1), $3));
	}
	| NT_exp TK_LE NT_exp
	{
		$$ = newNode(ASTN_exp, $2);
		appendChild($$, appendChild(appendChild(newNode(ASTN_le, $2), $1), $3));
	}
	| TK_LBRACK NT_exp TK_RBRACK
	{
		$$ = $2;
	}
	;

%%

int parse() {
	return yyparse();
}
