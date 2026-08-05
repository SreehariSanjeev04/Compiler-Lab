%{
    #include <stdio.h>
    #include <string.h>
    #include "labelAddressTable.h"

    int line_number = 0;
%}

label  (L[0-9]+):[ \t]*\r?\n

%%

{label} {
    char labelName[32];
    sscanf(yytext, "%[^:]", labelName);

    // The program header occupies 8 words starting at address 2048 and each
    // instruction is 2 words. A label occupies a file line but no instruction
    // slot, so line_number counts only header + instruction lines (labels are
    // intentionally not counted here).
    int labelAddress = 2056 + (line_number - 8) * 2;
    storeAddress(labelName, labelAddress);
}

.*\n {
    line_number++;
}

.* {
    line_number++;
}

%%

int detectwrap() { return 1; }