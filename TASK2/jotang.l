%{
#include <stdlib.h>
void yyerror(char *);
#include "y.tab.h"
%}

%%

-?[0-9]+                  { yylval.int_value = atoi(yytext); return num_INT; }
-?[0-9]+\.[0-9]+          { yylval.float_value = atof(yytext); return num_FLOAT; }
"+"                     { return Y_ADD; }
"-"                     { return Y_SUB; }
"*"                     { return Y_MUL; }
"/"                     { return Y_DIV; }
"++"                    { return Y_INC; }
"&*"					{ return Y_AND_MUL; }
"|*" 				  { return Y_OR_MUL; }
"("					 { return Y_LPAR; }
")"					 { return Y_RPAR; }
[ \t] ; 	     /* skip whitespace */
. yyerror("invalid character");

%%

int yywrap(void) 
{
	return 1;
}
