%require "3.2"
%language "c++"

%code requires {
    #include <string>
    #include "FooLexer.hh"
}

/* 指定类型 std::string yylval */
%define api.value.type {std::string}

/* 指定参数 parser(FooLexer &lexer) */
%parse-param {FooLexer &lexer}

%header

%code {
    // 指定 lexer.yylex
    #define yylex lexer.yylex
}

%token HELLO
%token WORLD

%%

hello_world: HELLO WORLD '!' { std::cout << "Goodbye " << $WORLD << '!' << std::endl; }

%%

void yy::parser::error(const std::string &message)
{
    std::cerr << "Error: " << message << std::endl;
}