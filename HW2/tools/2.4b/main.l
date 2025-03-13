/* a((b|a*c)x)*|x*a */

%{
#include <stdio.h>
int match = 0;
int success = 1;
%}

%%

a((b|a*c)x)*|x*a {
    if (match == 0) 
        match = 1;
    else 
        success = 0;
}
. { success = 0; }

%%

int yywrap() {
    return 1;
}

int main() {
    yylex();
    if (success) 
        printf("accept\n");
    else
        printf("reject\n");
    return 0;
}