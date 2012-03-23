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

%token TK_IF
%token TK_FI
%token TK_ELSE
%token TK_DO
%token TK_OD
%token TK_FA
%token TK_AF
%token TK_TO
%token TK_PROC
%token TK_END
%token TK_RETURN
%token TK_FORWARD
%token TK_VAR
%token TK_TYPE
%token TK_BREAK
%token TK_EXIT
%token TK_TRUE
%token TK_FALSE
%token TK_WRITE
%token TK_WRITES
%token TK_READ
%token TK_BOX
%token TK_ARROW
%token TK_LPAREN
%token TK_RPAREN
%token TK_LBRACK
%token TK_RBRACK
%token TK_COLON
%token TK_SEMI
%token TK_ASSIGN
%token TK_COMMA

%left TK_PLUS TK_MINUS

%left TK_STAR TK_SLASH TK_MOD

%right TK_QUEST UMINUS

%nonassoc TK_EQ TK_NEQ TK_GT TK_LT TK_GE TK_LE

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
	}
	| NT_stms
	{
		ASTroot.child = NULL;
		ASTroot.brother = NULL;
		ASTroot.sym = ASTN_program;
		appendChild(&ASTroot, $1);
	}
        | NT_decs
	{
		ASTroot.child = NULL;
		ASTroot.brother = NULL;
		ASTroot.sym = ASTN_program;
		appendChild(&ASTroot, $1);
	}
	| NT_decs NT_stms
	{
		ASTroot.child = NULL;
		ASTroot.brother = NULL;
		ASTroot.sym = ASTN_program;
		appendChild(&ASTroot, $1);
		appendChild(&ASTroot, $2);
	}
	;

NT_stms:  NT_stm
	{
		$$ = newNode(ASTN_stms);
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
		$$ = newNode(ASTN_dec);
		appendChild($$, $1);
	}
	| NT_type
	{
		$$ = newNode(ASTN_dec);
		appendChild($$, $1);
	}
	| NT_forward
	{
		$$ = newNode(ASTN_dec);
		appendChild($$, $1);
	}
	| NT_proc
	{
		$$ = newNode(ASTN_dec);
		appendChild($$, $1);
	}
	;

NT_decs:  NT_dec
	{
		$$ = newNode(ASTN_decs);
		appendChild($$, $1);
	}
       	| NT_dec NT_decs
	{
		insertChild($2, $1);
		$$ = $2;
	}
	;

NT_stm:	  NT_if
	{
		$$ = newNode(ASTN_stm);
		appendChild($$, $1);
	}
      	| NT_do
	{
		$$ = newNode(ASTN_stm);
		appendChild($$, $1);
	}
	| NT_fa
	{
		$$ = newNode(ASTN_stm);
		appendChild($$, $1);
	}
	| TK_BREAK TK_SEMI
	{
		$$ = newNode(ASTN_stm);
		appendChild($$, newNode(ASTN_L_break));
	}
	| TK_EXIT TK_SEMI
	{
		$$ = newNode(ASTN_stm);
		appendChild($$, newNode(ASTN_L_exit));
	}
	| TK_RETURN TK_SEMI
	{
		$$ = newNode(ASTN_stm);
		appendChild($$, newNode(ASTN_L_return));
	}
	| NT_lvalue TK_ASSIGN NT_exp TK_SEMI
	{
		$$ = newNode(ASTN_stm);
		appendChild($$ ,appendChild(appendChild(newNode(ASTN_assign), $1), $3));
	}
	| TK_WRITE NT_exp TK_SEMI
	{
		$$ = newNode(ASTN_stm);
		appendChild($$, appendChild(newNode(ASTN_write),$2));
	}
	| TK_WRITES NT_exp TK_SEMI
	{
		$$ = newNode(ASTN_stm);
		appendChild($$, appendChild(newNode(ASTN_writes),$2));
	}
	| NT_exp TK_SEMI
	{
		$$ = newNode(ASTN_stm);
		appendChild($$, $1); 
	}
	| TK_SEMI
	{
		$$ = newNode(ASTN_stm);
	}
	;

NT_if:	  TK_IF NT_exp TK_ARROW NT_stms NT_branches TK_FI
	{
		$$ = newNode(ASTN_if);
		appendBrother($2, $4);
		appendChild($$, appendChild(newNode(ASTN_branch),$2));
		appendChild($$, $5);
	}
     	| TK_IF NT_exp TK_ARROW NT_stms TK_FI
	{
		$$ = newNode(ASTN_if);
		appendBrother($2, $4);
		appendChild($$, appendChild(newNode(ASTN_branch), $2));
	}
	| TK_IF NT_exp TK_ARROW NT_stms TK_BOX TK_ELSE TK_ARROW NT_stms TK_FI
	{
		$$ = newNode(ASTN_if);
		appendBrother($2, $4);
		appendChild($$, appendChild(newNode(ASTN_branch), $2));
		appendChild($$, appendChild(newNode(ASTN_branch), $8));
	}
	| TK_IF NT_exp TK_ARROW NT_stms NT_branches TK_BOX TK_ELSE TK_ARROW NT_stms TK_FI
	{
		$$ = newNode(ASTN_if);
		appendBrother($2, $4);
		appendChild($$, appendChild(newNode(ASTN_branch), $2));
		appendChild($$, $5);
		appendChild($$, appendChild(newNode(ASTN_branch), $9));
	}
	;

NT_branches: NT_branches TK_BOX NT_exp TK_ARROW NT_stms
	{
		appendBrother($3, $5);
		appendBrotherAtLast($1, appendChild(newNode(ASTN_branch), $3));
		$$ = $1;
	}
	| TK_BOX NT_exp TK_ARROW NT_stms
	{
		$$ = newNode(ASTN_branch);
		appendChild($$, $2);
		appendChild($$, $4);
	}
	;

NT_do:	  TK_DO NT_exp TK_ARROW TK_OD
	{
		$$ = newNode(ASTN_do);
		appendChild($$, $2);
	}
     	| TK_DO NT_exp TK_ARROW NT_stms TK_OD
	{
		$$ = newNode(ASTN_do);
		appendChild($$, $2);
		appendChild($$, $4);
	}
	;

NT_fa:	  TK_FA TK_ID TK_ASSIGN NT_exp TK_TO NT_exp TK_ARROW TK_AF
	{
		$$ = newNode(ASTN_fa);
		appendBrother($4, $6);
		appendChild($$, $4);
	}
     	| TK_FA TK_ID TK_ASSIGN NT_exp TK_TO NT_exp TK_ARROW NT_stms TK_AF
	{
		$$ = newNode(ASTN_fa);
		appendBrother($4, $6);
		appendBrother($6, $8);
		appendChild($$, $4);
	}
	;

NT_proc:  TK_PROC TK_ID TK_LBRACK NT_declistx TK_RBRACK NT_dec1s NT_stms TK_END
	{
		$$ = newNode(ASTN_proc);
		appendChild($$, newIdNode($2));
		appendBrother($4, $6);
		appendBrother($6, $7);
		appendChild($$, $4);
	}
       	| TK_PROC TK_ID TK_LBRACK NT_declistx TK_RBRACK NT_dec1s TK_END
	{
		$$ = newNode(ASTN_proc);
		appendChild($$, newIdNode($2));
		appendBrother($4, $6);
		appendChild($$, $4);
	}
	| TK_PROC TK_ID TK_LBRACK NT_declistx TK_RBRACK NT_stms TK_END
	{
		$$ = newNode(ASTN_proc);
		appendChild($$, newIdNode($2));
		appendBrother($4, $6);
		appendChild($$, $4);
	}
	| TK_PROC TK_ID TK_LBRACK NT_declistx TK_RBRACK TK_END
	{
		$$ = newNode(ASTN_proc);
		appendChild($$, newIdNode($2));
		appendChild($$, $4);
	}
	| TK_PROC TK_ID TK_LBRACK NT_declistx TK_RBRACK TK_COLON TK_ID NT_dec1s NT_stms TK_END
	{
		$$ = newNode(ASTN_proc);
		appendChild($$, newIdNode($2));
		appendBrother($4, newIdNode($7));
		appendChild($$, $4);
		appendBrother($8, $9);
		appendChild($$, $8);
	}
	| TK_PROC TK_ID TK_LBRACK NT_declistx TK_RBRACK TK_COLON TK_ID NT_dec1s TK_END
	{
		$$ = newNode(ASTN_proc);
		appendChild($$, newIdNode($2));
		appendBrother($4, newIdNode($7));
		appendChild($$, $4);
		appendChild($$, $8);
	}
	| TK_PROC TK_ID TK_LBRACK NT_declistx TK_RBRACK TK_COLON TK_ID NT_stms TK_END
	{
		$$ = newNode(ASTN_proc);
		appendChild($$, newIdNode($2));
		appendBrother($4, newIdNode($7));
		appendChild($$, $4);
		appendChild($$, $8);
	}
	| TK_PROC TK_ID TK_LBRACK NT_declistx TK_RBRACK TK_COLON TK_ID TK_END
	{
		$$ = newNode(ASTN_proc);
		appendChild($$, newIdNode($2));
		appendBrother($4, newIdNode($7));
		appendChild($$, $4);
	}
	| TK_PROC TK_ID TK_LBRACK TK_RBRACK NT_dec1s NT_stms TK_END
	{
		$$ = newNode(ASTN_proc);
		appendChild($$, newIdNode($2));
		appendBrother($5, $6);
		appendChild($$, $5);
	}
       	| TK_PROC TK_ID TK_LBRACK TK_RBRACK NT_dec1s TK_END
	{
		$$ = newNode(ASTN_proc);
		appendChild($$, newIdNode($2));
		appendChild($$, $5);
	}
	| TK_PROC TK_ID TK_LBRACK TK_RBRACK NT_stms TK_END
	{
		$$ = newNode(ASTN_proc);
		appendChild($$, newIdNode($2));
		appendChild($$, $5);
	}
	| TK_PROC TK_ID TK_LBRACK TK_RBRACK TK_END
	{
		$$ = newNode(ASTN_proc);
		appendChild($$, newIdNode($2));
	}
	| TK_PROC TK_ID TK_LBRACK TK_RBRACK TK_COLON TK_ID NT_dec1s NT_stms TK_END
	{
		$$ = newNode(ASTN_proc);
		appendChild($$, newIdNode($2));
		appendChild($$, newIdNode($6));
		appendBrother($7, $8);
		appendChild($$, $7);
	}
	| TK_PROC TK_ID TK_LBRACK TK_RBRACK TK_COLON TK_ID NT_dec1s TK_END
	{
		$$ = newNode(ASTN_proc);
		appendChild($$, newIdNode($2));
		appendChild($$, newIdNode($6));
		appendChild($$, $7);
	}
	| TK_PROC TK_ID TK_LBRACK TK_RBRACK TK_COLON TK_ID NT_stms TK_END
	{
		$$ = newNode(ASTN_proc);
		appendChild($$, newIdNode($2));
		appendChild($$, newIdNode($6));
		appendChild($$, $7);
	}
	| TK_PROC TK_ID TK_LBRACK TK_RBRACK TK_COLON TK_ID TK_END
	{
		$$ = newNode(ASTN_proc);
		appendChild($$, newIdNode($2));
		appendChild($$, newIdNode($6));
	}
	;

NT_dec1:  NT_var
       	| NT_type
	;

NT_dec1s: NT_dec1
	| NT_dec1s NT_dec1
	;

NT_idlist: TK_ID 
	| TK_ID NT_cids
	;

NT_cids:  TK_COMMA TK_ID
     	| NT_cids TK_COMMA TK_ID
	;

NT_var:	  TK_VAR NT_varlists TK_SEMI 
      	;

NT_varlist: NT_idlist TK_COLON TK_ID NT_dim
	| NT_idlist TK_COLON TK_ID
	;

NT_dim: TK_LPAREN TK_INT TK_RPAREN
	| NT_dim TK_LPAREN TK_INT TK_RPAREN
	;

NT_indexs: TK_LPAREN NT_exp TK_RPAREN
	| NT_indexs TK_LPAREN NT_exp TK_RPAREN
	;

NT_varlists: NT_varlist
	| NT_varlists TK_COMMA NT_varlist
	;

NT_forward: TK_FORWARD TK_ID TK_LBRACK NT_declistx TK_RBRACK TK_SEMI
	| TK_FORWARD TK_ID TK_LBRACK NT_declistx TK_RBRACK TK_COLON TK_ID TK_SEMI
	| TK_FORWARD TK_ID TK_LBRACK TK_RBRACK TK_SEMI
	| TK_FORWARD TK_ID TK_LBRACK TK_RBRACK TK_COLON TK_ID TK_SEMI
	;

NT_type:  TK_TYPE TK_ID TK_EQ TK_ID NT_dim TK_SEMI
	| TK_TYPE TK_ID TK_EQ TK_ID TK_SEMI
	;

NT_declistx: NT_idlist TK_COLON TK_ID
	| NT_declistx TK_COMMA NT_idlist TK_COLON TK_ID
	;

NT_lvalue: TK_ID
	| TK_ID NT_indexs
	;

NT_cexps: TK_COMMA NT_exp
	| NT_cexps TK_COMMA NT_exp
	;

NT_exp:	  NT_lvalue
      	| TK_INT
	| TK_TRUE
	| TK_FALSE
	| TK_SLIT
	| TK_READ
	| TK_MINUS NT_exp %prec UMINUS
	| TK_QUEST NT_exp
	| TK_ID TK_LBRACK TK_RBRACK
	| TK_ID TK_LBRACK NT_exp NT_cexps TK_RBRACK
	| TK_ID TK_LBRACK NT_exp TK_RBRACK
	| NT_exp TK_PLUS NT_exp
	| NT_exp TK_MINUS NT_exp
	| NT_exp TK_STAR NT_exp
	| NT_exp TK_SLASH NT_exp
	| NT_exp TK_MOD NT_exp
	| NT_exp TK_EQ NT_exp
	| NT_exp TK_NEQ NT_exp
	| NT_exp TK_GT NT_exp
	| NT_exp TK_LT NT_exp
	| NT_exp TK_GE NT_exp
	| NT_exp TK_LE NT_exp
	| TK_LBRACK NT_exp TK_RBRACK
	;

%%

int main(int argc, char **argv) {
	if (argc>1)
		yydebug=1;
	else
		yydebug=0;
	yyparse();
	return 0;
}
