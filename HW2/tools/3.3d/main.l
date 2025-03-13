%{
    #include "y.tab.h"
%}

%%
[\(|\)|\[|\]]       { return yytext[0]; }
.                   { printf("reject\n"); exit(0); }
%%

int yywrap(void) {
    return 1;
}