%{
    #include "labelAddressTable.h"
    FILE* targetFile;
%}

%%  
(MAIN|L[0-9]+):\r?\n {
    /* ignore labels in the second pass */
}

(MAIN|L[0-9]+) {
    int address = getAddress(yytext);
    fprintf(targetFile, "%d\n", address);
}
. {/* ignore everything else */}

(.|\n) {
    fprintf(targetFile, "%s", yytext);
}
%%

int yywrap() {
    return 1;
}