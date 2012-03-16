%{
#include <stdio.h>
#include "ice9.tab.h"

extern void yyerror();
int yynewlines=1;
%}

%option noyywrap

string	(\"[^\"\n]*\")|(\'[^\'\n]*\')

%%
[ \t]+		{;}
\n		{yynewlines++;}	
#.*		{;}

"if"		{return TK_IF;}
"fi"		{return TK_FI;}
"else"		{return TK_ELSE;}
"fa"		{return TK_FA;}
"af"		{return TK_AF;}
"to"		{return TK_TO;}
"do"		{return TK_DO;}
"od"		{return TK_OD;}
"proc"		{return TK_PROC;}
"end"		{return TK_END;}
"return"	{return TK_RETURN;}
"forward"	{return TK_FORWARD;}
"var"		{return TK_VAR;}
"type"		{return TK_TYPE;}
"break"		{return TK_BREAK;}
"exit"		{return TK_EXIT;}
"true"		{return TK_TRUE;}
"false"		{return TK_FALSE;}
"write"		{return TK_WRITE;}
"writes"	{return TK_WRITES;}
"read"		{return TK_READ;}
"[]"		{return TK_BOX;}
"->"		{return TK_ARROW;}
"("		{return TK_LBRACK;}
")"		{return TK_RBRACK;}
"["		{return TK_LPAREN;}
"]"		{return TK_RPAREN;}
":"		{return TK_COLON;}
";"		{return TK_SEMI;}
"="		{return TK_EQ;}
":="		{return TK_ASSIGN;}
"?"		{return TK_QUEST;}
","		{return TK_COMMA;}
"+"		{return TK_PLUS;}
"-"		{return TK_MINUS;}
"*"		{return TK_STAR;}
"/"		{return TK_SLASH;}
"%"		{return TK_MOD;}
"!="		{return TK_NEQ;}
">"		{return TK_GT;}
"<"		{return TK_LT;}
">="		{return TK_GE;}
"<="		{return TK_LE;}

[0-9]+		{return TK_INT;}
{string}	{return TK_SLIT;}
[a-zA-Z][a-zA-Z0-9_]*	{return TK_ID;}

[0-9]+[a-zA-Z][a-zA-Z0-9_]*	{yyerror("Illegal token");exit(-1);}

.	{yyerror("Illegal charactor");exit(-1);}

%%
