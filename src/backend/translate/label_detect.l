%{
    #include <stdio.h>
    #include "labelAddressTable.h"
    int current_address = 2048;
    int line_number = 8;
%}

// currently the labels are either MAIN or L<int>
label (MAIN|L[0-9]+):\r?\n
%%  

{label} {
    char labelName[32];
    sscanf(yytext, "%[^:]", labelName);
    printf("Label detected: %s\n", labelName);
    storeAddress(labelName, current_address);
}

[\n] {
    if(line_number <= 8) current_address++;
    else current_address += 2;
    line_number++;
}
%%

int yywrap() {
    return 1;
}