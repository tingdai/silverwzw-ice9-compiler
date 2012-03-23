%{
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "ice9.tab.h"
int fileno(FILE *stream);
char * iddmp(char * s) {
	char *str;
	unsigned i;
	for (i = 0; *(s + i) != '\0'; i++) {
		;
	}
	str = malloc(i + 1);
	while (*(s + i) != '\0') {
		*(str + i) = *(s + i);
		i++;
	}
	*(str + i) = '\0';
	return str;
}
char * strdmp(char * s) {
	char *str;
	unsigned i;
	assert(*s == '\'' || *s == '"');
	s++;
	for (i = 0; *(s + i) != '\0'; i++) {
		;
	}
	str = malloc(i);
	while (*(s + i) != '\0') {
		*(str + i) = *(s + i);
		i++;
	}
	assert(*(s + i - 1) == '"' || *(s + i - 1) == '\'');
	*(str + i - 1) = '\0';
	return str;
}

extern void yyerror();
int yynewlines=1;


%}

%option noyywrap
%option nounput

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

[0-9]+		{yylval.intt = atoi(yytext);return TK_INT;}
{string}	{yylval.str = strdmp(yytext);return TK_SLIT;}
[a-zA-Z][a-zA-Z0-9_]*	{yylval.str = iddmp(yytext);return TK_ID;}

[0-9]+[a-zA-Z][a-zA-Z0-9_]*	{yyerror("Illegal token");exit(-1);}

.	{yyerror("Illegal charactor");exit(-1);}

%%
