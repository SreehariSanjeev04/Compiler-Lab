%{
  #include <stdlib.h>
  #include "add.tab.h"
%}


%%

[0-9]+ {yylval = atoi(yytext); return NUMBER;}
[ \t]  {/* IGNORE */}
\n {return '\n';}
"+" {return PLUS;}
. {/* IGNORE */}

%%

int yywrap() {
    return 1;
}
