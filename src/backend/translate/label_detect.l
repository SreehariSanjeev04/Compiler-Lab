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
    
    int labelAddress = 2056 + (line_number - 8) * 2; 
    storeAddress(labelName, labelAddress);
}

.*\n {
    line_number++;
}

%%

int detectwrap() { return 1; }