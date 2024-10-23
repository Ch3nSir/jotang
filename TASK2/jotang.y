%{

#include <stdio.h>

void yyerror(char *);
int yylex(void);
extern char* yytext;

%}
%union{
	int		int_value;
    float   float_value;
};
%token <int_value> num_INT
%token <float_value> num_FLOAT
%token Y_ADD Y_SUB Y_MUL Y_DIV Y_INC Y_AND_MUL Y_OR_MUL Y_LPAR Y_RPAR
%left Y_ADD Y_SUB Y_MUL Y_DIV Y_INC Y_AND_MUL Y_OR_MUL
%type <int_value> expr factor term
%type <float_value> expr_f factor_f term_f
%%

program:  expr       { printf("%d\n",  $1); }
	  expr_f     { printf("%f\n",  (double)$1); }	;

expr: factor			  {$$ = $1;}
	| expr Y_ADD factor     {$$ = $1 + $3;}
	| expr Y_SUB factor     {$$ = $1 - $3;}
	| expr Y_INC factor    {$$ = $1 + $3 + 1;}
    | Y_LPAR expr Y_RPAR    {$$ = $2;}
	;
	
factor: term
	| factor Y_MUL term    {$$ = $1 * $3;} 
	| factor Y_DIV term    {$$ = $1 / $3;}
	| factor Y_AND_MUL term    {$$ = ($1 & $3) * $3;}
	| factor Y_OR_MUL term    {$$ = ($1 | $3) * $3;}
	| Y_LPAR expr Y_RPAR    {$$ = $2;}
	;
	
term:  num_INT           {$$ = $1; }
	;

	expr_f: factor_f			  {$$ = $1;}
		| expr_f Y_ADD factor_f     {$$ = $1 + $3;}
		| expr_f Y_SUB factor_f     {$$ = $1 - $3;}
		| expr_f Y_INC factor_f    {$$ = $1 + $3 + 1;}
		| Y_LPAR expr_f Y_RPAR    {$$ = $2;}
		;
		
	factor_f: term_f
		| factor_f Y_MUL term_f    {$$ = $1 * $3;} 
		| factor_f Y_DIV term_f    {$$ = $1 / $3;}
		| Y_LPAR expr_f Y_RPAR    {$$ = $2;}
		;
		
	term_f: num_FLOAT            {$$ = $1;}
		;

%%
void yyerror(char *s) 
{
	fprintf(stderr, "%s\n", s);
}
int main(void) 
{
    yyparse();
    return 0;
}
