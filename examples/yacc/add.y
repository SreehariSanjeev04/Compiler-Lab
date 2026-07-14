%{
    #include <stdio.h>
    void yyerror(const char *s);
    int yylex();
%}

%token NUMBER
%token PLUS

%%

start: expr '\n' { printf("Result: %d\n", $1); }
    | expr { printf("Result: %d\n", $1); }
     ;

expr: expr PLUS NUMBER { $$ = $1 + $3; }
    | NUMBER { $$ = $1; }
    ;
%%

void yyerror(const char *s) {
    fprintf(stderr, "Error: %s\n", s);
}

int main() {
    yyparse();
    return 0;
}