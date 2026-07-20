%{
    #include "labelAddressTable.h"
    FILE* outputFile;
%}

%%  
(L[0-9]+):[ \t]*\r?\n {
    /* ignore labels in the second pass */
}

(L[0-9]+) {
    int address = getAddress(yytext);
    fprintf(outputFile, "%d\n", address);
}

(.|\n) {
    fprintf(outputFile, "%s", yytext);
}
%%

int translatewrap() {
    return 1;
}