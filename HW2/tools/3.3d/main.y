%{
    #include <stdlib.h>
    #include <stdio.h>
    extern int yylex();
    void yyerror(const char* msg) {
        printf("reject\n");
        exit(0);
    }
%}

%start S

%%

S : T S
  |
  ;

T : '[' U ']'
  | '(' S ')'
  ;

U : T U
  | '(' U
  |
  ;

%%

int main() {
    yyparse();
    printf("accept\n");
    return 0;
}