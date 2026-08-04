%{
  #include <stdio.h>
%}

%option noyywrap

alpha  [a-zA-Z]
number  [0-9]
identifier  {alpha}({alpha}|{number})*

%%
{identifier} {
    printf("Identifier - %s\n", yytext);
}
{number}({alpha}|{number})* {
    printf("Invalid identifier - %s\n", yytext);
}
[ \t] {/* IGNORE */}
\n {/* IGNORE */}
. {
    printf("Invalid token - %s\n", yytext);
}
%%

int yyerror(const char* s) {
    fprintf(stderr, "%s\n", s);
    return 0;
}

int main() {
    yylex();
    return 0;
}