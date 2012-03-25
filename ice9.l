%{
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "ast.h"
#include "ice9.tab.h"

int fileno(FILE *stream);
char * iddmp(char * s) {
	// has hard copy @ semantic.cpp
	char *str;
	unsigned i;
	for (i = 0; *(s + i) != '\0'; i++) {
		;
	}
	str = malloc(i + 1);
	i = 0;
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
	i = 0;
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

"if"		{yylval.intt=yynewlines;return TK_IF;}
"fi"		{return TK_FI;}
"else"		{return TK_ELSE;}
"fa"		{yylval.intt=yynewlines;return TK_FA;}
"af"		{return TK_AF;}
"to"		{return TK_TO;}
"do"		{yylval.intt=yynewlines;return TK_DO;}
"od"		{return TK_OD;}
"proc"		{yylval.intt=yynewlines;return TK_PROC;}
"end"		{return TK_END;}
"return"	{return TK_RETURN;}
"forward"	{yylval.intt=yynewlines;return TK_FORWARD;}
"var"		{yylval.intt=yynewlines;return TK_VAR;}
"type"		{yylval.intt=yynewlines;return TK_TYPE;}
"break"		{return TK_BREAK;}
"exit"		{return TK_EXIT;}
"true"		{return TK_TRUE;}
"false"		{return TK_FALSE;}
"write"		{return TK_WRITE;}
"writes"	{return TK_WRITES;}
"read"		{yylval.intt=yynewlines;return TK_READ;}
"[]"		{yylval.intt=yynewlines;return TK_BOX;}
"->"		{return TK_ARROW;}
"("		{return TK_LBRACK;}
")"		{return TK_RBRACK;}
"["		{return TK_LPAREN;}
"]"		{return TK_RPAREN;}
":"		{yylval.intt=yynewlines;return TK_COLON;}
";"		{yylval.intt=yynewlines;return TK_SEMI;}
"="		{yylval.intt=yynewlines;return TK_EQ;}
":="		{return TK_ASSIGN;}
"?"		{yylval.intt=yynewlines;return TK_QUEST;}
","		{return TK_COMMA;}
"+"		{yylval.intt=yynewlines;return TK_PLUS;}
"-"		{yylval.intt=yynewlines;return TK_MINUS;}
"*"		{yylval.intt=yynewlines;return TK_STAR;}
"/"		{yylval.intt=yynewlines;return TK_SLASH;}
"%"		{yylval.intt=yynewlines;return TK_MOD;}
"!="		{yylval.intt=yynewlines;return TK_NEQ;}
">"		{yylval.intt=yynewlines;return TK_GT;}
"<"		{yylval.intt=yynewlines;return TK_LT;}
">="		{yylval.intt=yynewlines;return TK_GE;}
"<="		{yylval.intt=yynewlines;return TK_LE;}

[0-9]+		{yylval.intt = atoi(yytext);return TK_INT;}
{string}	{yylval.str = strdmp(yytext);return TK_SLIT;}
[a-zA-Z][a-zA-Z0-9_]*	{yylval.str = iddmp(yytext);return TK_ID;}

[0-9]+[a-zA-Z][a-zA-Z0-9_]*	{yyerror("Illegal token");exit(-1);}

.	{yyerror("Illegal charactor");exit(-1);}

%%
