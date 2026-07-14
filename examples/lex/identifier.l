// Auxilary Declarations

%{
  #include <stdio.h>
%}

%option noyywrap

// Declarations of Regular Expressions
alpha  [a-zA-Z]
number  [0-9]
identifier  {alpha}{alpha}*{number}

// Rules
%%
{identifier} {
    printf("Identifier - %s\n", yytext);
}
\n {/* IGNORE */}
. {/* IGNORE */}
%%

// Auxiliary Functions
int yyerror(const char* s) {
    fprintf(stderr, "%s\n", s);
    return 0;
}

int main() {
    yylex();
    return 0;
}