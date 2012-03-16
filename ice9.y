%{

#include <stdio.h>
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


%start program

%%

program: /* empty */
	| NT_stms
        | NT_decs
	| NT_decs NT_stms
	;

NT_stms:  NT_stm
	| NT_stms NT_stm
	;	

NT_dec:	  NT_var
	| NT_type
	| NT_forward
	| NT_proc
	;

NT_decs:  NT_dec
       	| NT_dec NT_decs
	;

NT_stm:	  NT_if
      	| NT_do
	| NT_fa
	| TK_BREAK TK_SEMI
	| TK_EXIT TK_SEMI
	| TK_RETURN TK_SEMI
	| NT_lvalue TK_ASSIGN NT_exp TK_SEMI
	| TK_WRITE NT_exp TK_SEMI
	| TK_WRITES NT_exp TK_SEMI
	| NT_exp TK_SEMI
	| TK_SEMI
	;

NT_if:	  TK_IF NT_exp TK_ARROW NT_stms NT_branches TK_FI
     	| TK_IF NT_exp TK_ARROW NT_stms TK_FI
	| TK_IF NT_exp TK_ARROW NT_stms TK_BOX TK_ELSE TK_ARROW NT_stms TK_FI
	| TK_IF NT_exp TK_ARROW NT_stms NT_branches TK_BOX TK_ELSE TK_ARROW NT_stms TK_FI
	;

NT_branches: NT_branches TK_BOX NT_exp TK_ARROW NT_stms
	| TK_BOX NT_exp TK_ARROW NT_stms
	;

NT_do:	  TK_DO NT_exp TK_ARROW TK_OD
     	| TK_DO NT_exp TK_ARROW NT_stms TK_OD
	;

NT_fa:	  TK_FA TK_ID TK_ASSIGN NT_exp TK_TO NT_exp TK_ARROW TK_AF
     	| TK_FA TK_ID TK_ASSIGN NT_exp TK_TO NT_exp TK_ARROW NT_stms TK_AF
	;

NT_proc:  TK_PROC TK_ID TK_LBRACK NT_declistx TK_RBRACK NT_dec1s NT_stms TK_END
       	| TK_PROC TK_ID TK_LBRACK NT_declistx TK_RBRACK NT_dec1s TK_END
	| TK_PROC TK_ID TK_LBRACK NT_declistx TK_RBRACK NT_stms TK_END
	| TK_PROC TK_ID TK_LBRACK NT_declistx TK_RBRACK TK_END
	| TK_PROC TK_ID TK_LBRACK NT_declistx TK_RBRACK TK_COLON TK_ID NT_dec1s NT_stms TK_END
	| TK_PROC TK_ID TK_LBRACK NT_declistx TK_RBRACK TK_COLON TK_ID NT_dec1s TK_END
	| TK_PROC TK_ID TK_LBRACK NT_declistx TK_RBRACK TK_COLON TK_ID NT_stms TK_END
	| TK_PROC TK_ID TK_LBRACK NT_declistx TK_RBRACK TK_COLON TK_ID TK_END
	| TK_PROC TK_ID TK_LBRACK TK_RBRACK NT_dec1s NT_stms TK_END
       	| TK_PROC TK_ID TK_LBRACK TK_RBRACK NT_dec1s TK_END
	| TK_PROC TK_ID TK_LBRACK TK_RBRACK NT_stms TK_END
	| TK_PROC TK_ID TK_LBRACK TK_RBRACK TK_END
	| TK_PROC TK_ID TK_LBRACK TK_RBRACK TK_COLON TK_ID NT_dec1s NT_stms TK_END
	| TK_PROC TK_ID TK_LBRACK TK_RBRACK TK_COLON TK_ID NT_dec1s TK_END
	| TK_PROC TK_ID TK_LBRACK TK_RBRACK TK_COLON TK_ID NT_stms TK_END
	| TK_PROC TK_ID TK_LBRACK TK_RBRACK TK_COLON TK_ID TK_END
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
