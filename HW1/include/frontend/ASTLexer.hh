#ifndef _ASTLexer_hh
#define _ASTLexer_hh

#include <fstream>
#include <iostream>
#include <string>

#if !defined(yyFlexLexerOnce)
#define yyFlexLexer yy_ast_FlexLexer
#include <FlexLexer.h>
#undef yyFlexLexer
#endif

#include "FDMJAST.hh"
#include "ast_location.hh"

using namespace std;
using namespace fdmj;

namespace fdmj {
// %option yyclass="ASTLexer"
// token: yylval的类型
class AST_YYSTYPE {
public:
    int i;
    string s;

    IntExp* intExp;
    IdExp* idExp;
    OpExp* opExp;
    Program* program;
    MainMethod* mainMethod;
    Stm* stm;
    vector<Stm*>* stmList;
    Exp* exp;

    Program* root;
};

class ASTLexer : public yy_ast_FlexLexer {
    // 初始化行号和列号
    // 每次遇到换行符时, 行号增加, 列号重置
    std::size_t currentLine = 1;
    std::size_t currentColumn = 1;

    // lexer返回的token信息
    AST_YYSTYPE* yylval = nullptr;
    location_t* yylloc = nullptr;

    // 将token的值复制到yylval
    // <INITIAL>{non_negative_integer} { copyValue(std::atoi(yytext)); return Token::NONNEGATIVEINT; }
    // <INITIAL>{identifier}           { copyValue(yytext); return Token::IDENTIFIER; }
    void copyValue(const string s) { yylval->s = s; }
    void copyValue(const int n) { yylval->i = n; }

    // 将token的位置复制到yylloc (每次发生匹配时执行)
    // #define YY_USER_ACTION copyLocation();
    void copyLocation()
    {
        *yylloc = location_t(currentLine, currentColumn, currentLine, yyleng + currentColumn - 1);
        currentColumn += yyleng;
    }

public:
    ASTLexer(std::istream& in, const bool debug)
        : yy_ast_FlexLexer(&in)
    {
        yy_ast_FlexLexer::set_debug(debug);
    }
    int yylex(AST_YYSTYPE* const lval, location_t* const lloc);
};
} // namespace fdmj
#endif