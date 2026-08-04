%{
    #include<stdio.h>
    #include<stdlib.h>
    char *file1;   
%}

even  [0-9]*[02468]
odd  [0-9]*[13579]

%%

{even} {
    printf("Even - %d\n", atoi(yytext));
}
    
{odd} {
    printf("Odd - %d\n", atoi(yytext));
}

\n {/* IGNORE */}

. {/* IGNORE */}
%%

int yywrap() {
    return 1;
}

int yyerror(char *s) {
    fprintf(stderr, "%s\n", s);
    return 0;
}

int main() {
    yylex();
    return 0;
}