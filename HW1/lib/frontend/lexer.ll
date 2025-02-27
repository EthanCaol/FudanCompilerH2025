%{
#include <string>
#include <variant>
#include "ASTheader.hh"
#include "FDMJAST.hh"
#include "ast_location.hh"
#include "parser.tab.hh"

using namespace std;
using namespace fdmj;

// 定义yylex接口, 接受两个参数lval和lloc
#undef  YY_DECL
#define YY_DECL int ASTLexer::yylex(AST_YYSTYPE* const lval, location_t *const lloc)

// 初始化参数
#define YY_USER_INIT yylval = lval; yylloc = lloc;

// 执行拷贝位置信息
#define YY_USER_ACTION copyLocation();

%}

%option c++ noyywrap debug
%option yyclass="ASTLexer"
%option prefix="yy_ast_"

%s COMMENT1 COMMENT2

punctuation             [()\[\]{}=,;.!]
non_negative_integer    [1-9][0-9]*|0
identifier              [a-z_A-Z][a-z_A-Z0-9]*

%%

%{
    using Token = ASTParser::token;
%}

<INITIAL>"//" { BEGIN COMMENT1; }
<INITIAL>"/*" { BEGIN COMMENT2; }
<INITIAL>" "|\t { }
<INITIAL>\r { ++currentLine; currentColumn = 1; }
<INITIAL>\n { ++currentLine; currentColumn = 1; }

<INITIAL>"public"   { return Token::PUBLIC; }
<INITIAL>"int"      { return Token::INT; }
<INITIAL>"main"     { return Token::MAIN; }
<INITIAL>"return"   { return Token::RETURN; }

<INITIAL>"+" { return Token::ADD; }
<INITIAL>"-" { return Token::MINUS; }
<INITIAL>"*" { return Token::TIMES; }
<INITIAL>"/" { return Token::DIVIDE; }

<INITIAL>{punctuation} { return yytext[0];}

<INITIAL>{non_negative_integer} { copyValue(std::atoi(yytext)); return Token::NONNEGATIVEINT; }
<INITIAL>{identifier}           { copyValue(yytext); return Token::IDENTIFIER; }

<INITIAL>. { cerr << "Illegal input " << yytext[0] << endl; return 0; }

<COMMENT1>\n { ++currentLine; currentColumn = 1; BEGIN INITIAL; }
<COMMENT1>. { }

<COMMENT2>"*/" { BEGIN INITIAL; }
<COMMENT2>\n { ++currentLine; currentColumn = 1; }
<COMMENT2>. { }
%%

int yywrap() {
    return 1;
}
