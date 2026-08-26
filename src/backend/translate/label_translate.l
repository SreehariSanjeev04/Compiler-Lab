%{
    #include "labelAddressTable.h"
    #include <string.h>
    extern FILE* outputFile;
%}

%%
([LF][0-9]+|MAIN):[ \t]*\r?\n {}

[ \t]([LF][0-9]+|MAIN)[ \t]*\r?\n {
    char labelName[32];
    sscanf(yytext, " %31s", labelName);
    int address = getAddress(labelName);
    // The label reference is always the last token on its line (JMP/JZ/CALL),
    // so the trailing newline is re-emitted here.
    fprintf(outputFile, " %d\n", address);
}

[\n] {fprintf(outputFile, "\n");}
. {
    fprintf(outputFile, "%s", yytext);
}
%%

int translatewrap() {
    return 1;
}
