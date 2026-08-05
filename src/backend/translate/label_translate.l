%{
    #include "labelAddressTable.h"
    extern FILE* outputFile;
%}

%%  
(L[0-9]+):[ \t]*\r?\n {}

(L[0-9]+) {
    int address = getAddress(yytext);
    // The trailing newline is emitted by the [\n] rule below.
    fprintf(outputFile, "%d", address);
}

[\n] {fprintf(outputFile, "\n");}
. {
    fprintf(outputFile, "%s", yytext);
}
%%

int translatewrap() {
    return 1;
}