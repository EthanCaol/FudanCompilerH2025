#ifndef _FDMJAST_HH
#define _FDMJAST_HH

#include <string>
#include <vector>
#include "ASTheader.hh"
#include "FDMJAST.hh"

using namespace std;

namespace fdmj {

// 程序: 主方法 类声明列表
// PROG: MAINMETHOD CLASSDECLLIST
class Program : public AST {
public:
    MainMethod* mainMethod;
    vector<ClassDecl*>* classDeclList = new vector<ClassDecl*>();

    Program(Pos* pos, MainMethod* mainMethod)
        : AST(pos)
        , mainMethod(mainMethod)
        , classDeclList(nullptr) { };
    Program(Pos* pos, MainMethod* mainMethod, vector<ClassDecl*>* classDeclList)
        : AST(pos)
        , mainMethod(mainMethod)
        , classDeclList(classDeclList) { };

    ASTKind getASTKind() override { return ASTKind::Program; }
    Program* clone() override;
    void accept(AST_Visitor& v) override { v.visit(this); }
};

// 主方法: public int main() { 变量声明列表 语句列表 }
// MAINMETHOD: PUBLIC INT MAIN '(' ')' '{' VARDECLLIST STMLIST '}';
class MainMethod : public AST {
public:
    vector<VarDecl*>* varDeclList;
    vector<Stm*>* stmList;

    MainMethod(Pos* pos, vector<VarDecl*>* varDeclList, vector<Stm*>* stmList)
        : AST(pos)
        , varDeclList(varDeclList)
        , stmList(stmList) { };

    ASTKind getASTKind() override { return ASTKind::MainMethod; }
    MainMethod* clone() override;
    void accept(AST_Visitor& v) override { v.visit(this); }
};

// 类声明: 类名 [基类名] { 变量声明列表 方法声明列表 }
// CLASSDECL: PUBLIC CLASS ID '{' VARDECLLIST METHODDECLLIST '}'
//          | PUBLIC CLASS ID EXTENDS ID '{' VARDECLLIST METHODDECLLIST '}'
class ClassDecl : public AST {
public:
    IdExp* id = nullptr;
    IdExp* eid = nullptr; // 基类id
    vector<VarDecl*>* varDeclList = new vector<VarDecl*>();
    vector<MethodDecl*>* methodDeclList = new vector<MethodDecl*>();

    ClassDecl(Pos* pos, IdExp* id, vector<VarDecl*>* varDeclList, vector<MethodDecl*>* methodDeclList)
        : AST(pos)
        , id(id)
        , varDeclList(varDeclList)
        , methodDeclList(methodDeclList) { };
    ClassDecl(Pos* pos, IdExp* id, IdExp* eid, vector<VarDecl*>* varDeclList, vector<MethodDecl*>* methodDeclList)
        : AST(pos)
        , id(id)
        , eid(eid)
        , varDeclList(varDeclList)
        , methodDeclList(methodDeclList) { };

    ASTKind getASTKind() override { return ASTKind::ClassDecl; }
    ClassDecl* clone() override;
    void accept(AST_Visitor& v) override { v.visit(this); }
};

// 类型:  整型 | 整型数组 | 类
// TYPE: INT | INT '[' ']' | CLASS ID
class Type : public AST {
public:
    TypeKind typeKind;
    IdExp* cid = nullptr;    // 类
    IntExp* arity = nullptr; // 数组维数

    // 整型
    Type(Pos* pos)
        : AST(pos)
        , typeKind(TypeKind::INT) { };

    // 整型数组
    Type(Pos* pos, IntExp* arity)
        : AST(pos)
        , typeKind(TypeKind::ARRAY)
        , arity(arity) { };

    // 类
    Type(Pos* pos, IdExp* cid)
        : AST(pos)
        , typeKind(TypeKind::CLASS)
        , cid(cid) { };

    Type(Pos* pos, TypeKind typeKind, IdExp* cid, IntExp* arity)
        : AST(pos)
        , typeKind(typeKind)
        , cid(cid)
        , arity(arity) { };

    ASTKind getASTKind() override { return ASTKind::Type; }
    Type* clone() override;
    void accept(AST_Visitor& v) override { v.visit(this); }
};

// 变量声明
// VARDECL: CLASS ID ID ';'
//        | INT ID ';'
//        | INT '[' ']' ID ';'
//        | INT '[' NUM ']' ID ';'
//        | INT ID '=' CONST ';'
//        | INT '[' ']' ID '=' '{' CONSTLIST '}' ';'
//        | INT '[' NUM ']' ID '=' '{' CONSTLIST '}' ';'
class VarDecl : public AST {
public:
    Type* type = nullptr;                               // 变量类型
    IdExp* id = nullptr;                                // 变量名
    variant<monostate, IntExp*, vector<IntExp*>*> init; // 初始化数据

    // nullptr 意味着没有init
    // vector.size=0 意味着空数组初始化

    // 类型声明 (无初始化)
    VarDecl(Pos* pos, Type* type, IdExp* id)
        : AST(pos)
        , type(type)
        , id(id)
        , init(std::monostate {}) { };

    // 整型初始化
    VarDecl(Pos* pos, Type* type, IdExp* id, IntExp* init_int)
        : AST(pos)
        , type(type)
        , id(id)
        , init(init_int) { };

    // 数组初始化
    VarDecl(Pos* pos, Type* type, IdExp* id, vector<IntExp*>* init_array)
        : AST(pos)
        , type(type)
        , id(id)
        , init(init_array) { };

    VarDecl(Pos* pos, Type* type, IdExp* id, variant<monostate, IntExp*, vector<IntExp*>*> init)
        : AST(pos)
        , type(type)
        , id(id)
        , init(init) { };

    ASTKind getASTKind() override { return ASTKind::VarDecl; }
    VarDecl* clone() override;
    void accept(AST_Visitor& v) override { v.visit(this); }
};

// 方法声明: 返回类型 方法名(形参列表) { 变量声明列表 语句列表 }
// METHODDECL: PUBLIC TYPE ID '(' FORMALLIST ')' '{' VARDECLLIST STMLIST '}'
//           | PUBLIC TYPE ID '(' ')' '{' VARDECLLIST STMLIST '}'
//           | PUBLIC TYPE ID '(' FORMALLIST ')' '{' STMLIST '}'
//           | PUBLIC TYPE ID '(' ')' '{' STMLIST '}'
class MethodDecl : public AST {
public:
    Type* type = nullptr;                                   // 返回类型
    IdExp* id = nullptr;                                    // 方法名
    vector<Formal*>* formalList = new vector<Formal*>();    // 形参列表
    vector<VarDecl*>* varDeclList = new vector<VarDecl*>(); // 变量声明列表
    vector<Stm*>* stmList = new vector<Stm*>();             // 语句列表

    MethodDecl(Pos* pos, Type* type, IdExp* id, vector<Formal*>* formalList, vector<VarDecl*>* varDeclList, vector<Stm*>* stmList)
        : AST(pos)
        , type(type)
        , id(id)
        , formalList(formalList)
        , varDeclList(varDeclList)
        , stmList(stmList) { };
    MethodDecl(Pos* pos, Type* type, IdExp* id, vector<VarDecl*>* varDeclList, vector<Stm*>* stmList)
        : AST(pos)
        , type(type)
        , id(id)
        , varDeclList(varDeclList)
        , stmList(stmList) { }; // 无形参列表
    MethodDecl(Pos* pos, Type* type, IdExp* id, vector<Formal*>* formalList, vector<Stm*>* stmList)
        : AST(pos)
        , type(type)
        , id(id)
        , formalList(formalList)
        , stmList(stmList) { }; // 无变量声明列表
    MethodDecl(Pos* pos, Type* type, IdExp* id, vector<Stm*>* stmList)
        : AST(pos)
        , type(type)
        , id(id)
        , stmList(stmList) { }; // 无形参列表 无变量声明列表

    ASTKind getASTKind() override { return ASTKind::MethodDecl; }
    MethodDecl* clone() override;
    void accept(AST_Visitor& v) override { v.visit(this); }
};

// 方法形参: (类型 形参名, 类型 形参名, ...)
// FORMALLIST: ε | TYPE ID FORMALREST
// FORMALREST: ε | ',' TYPE ID FORMALREST
class Formal : public AST {
public:
    Type* type = nullptr; // 形参类型
    IdExp* id = nullptr;  // 形参名

    Formal(Pos* pos, Type* type, IdExp* id)
        : AST(pos)
        , type(type)
        , id(id)
    {
        // 确保数组类型有维数
        if (type->typeKind == TypeKind::ARRAY) {
            if (type->arity == nullptr) {
                cerr << "at position: " << pos->print() << endl;
                cerr << "Error: Array type has no arity in the formal. Not allowed!" << endl;
                exit(1);
            }
        }
    }

    ASTKind getASTKind() override { return ASTKind::Formal; }
    Formal* clone() override;
    void accept(AST_Visitor& v) override { v.visit(this); }
};

// 语句
// STMLIST: ε | STM STMLIST
// STM: '{' STMLIST '}'
//      | IF '(' EXP ')' STM ELSE STM
//      | IF '(' EXP ')' STM
//      | WHILE '(' EXP ')' STM
//      | WHILE '(' EXP ')' ';'
//      | EXP '=' EXP ';'
//      | EXP '.' ID '(' EXPLIST ')' ';'
//      | EXP '.' ID '(' ')' ';'
//      | CONTINUE ';'
//      | BREAK ';'
//      | RETURN EXP ';'
//      | PUTINT '(' EXP ')' ';'
//      | PUTCH '(' EXP ')' ';'
//      | PUTARRAY '(' EXP ',' EXP ')' ';'
//      | STARTTIME '(' ')' ';'
//      | STOPTIME '(' ')' ';'
class Stm : public AST {
public:
    Stm(Pos* pos)
        : AST(pos) { };
    virtual ASTKind getASTKind() = 0;
};

// 语句->语句块
// NESTED: '{' STMLIST '}'
class Nested : public Stm {
public:
    vector<Stm*>* stmList;
    Nested() = default;
    Nested(Pos* pos, vector<Stm*>* stmList)
        : Stm(pos)
        , stmList(stmList) { };
    ASTKind getASTKind() override { return ASTKind::Nested; }
    Nested* clone() override;
    void accept(AST_Visitor& v) override { v.visit(this); }
};

// 语句->if语句: if (条件表达式) 语句1 [else 语句2]
// STM: IF '(' EXP ')' STM ELSE STM
//    | IF '(' EXP ')' STM
class If : public Stm {
public:
    Exp* exp = nullptr;
    Stm* stm1 = nullptr;
    Stm* stm2 = nullptr;

    If(Pos* pos, Exp* exp, Stm* stm1, Stm* stm2)
        : Stm(pos)
        , exp(exp)
        , stm1(stm1)
        , stm2(stm2) { };
    If(Pos* pos, Exp* exp, Stm* stm1)
        : Stm(pos)
        , exp(exp)
        , stm1(stm1)
        , stm2(nullptr) { };

    ASTKind getASTKind() override { return ASTKind::If; }
    If* clone() override;
    void accept(AST_Visitor& v) override { v.visit(this); }
};

// 语句->while语句: while (条件表达式) 语句
// STM: WHILE '(' EXP ')' STM
//    | WHILE '(' EXP ')' ';'
class While : public Stm {
public:
    Exp* exp = nullptr;
    Stm* stm = nullptr;

    While(Pos* pos, Exp* exp, Stm* stm)
        : Stm(pos)
        , exp(exp)
        , stm(stm) { };
    While(Pos* pos, Exp* exp)
        : Stm(pos)
        , exp(exp)
        , stm(nullptr) { };

    ASTKind getASTKind() override { return ASTKind::While; }
    While* clone() override;
    void accept(AST_Visitor& v) override { v.visit(this); }
};

// 语句->赋值语句: 左值表达式 = 右值表达式;
// STM: EXP '=' EXP ';'
class Assign : public Stm {
public:
    Exp* left = nullptr;
    Exp* exp = nullptr;

    Assign(Pos* pos, Exp* left, Exp* exp)
        : Stm(pos)
        , left(left)
        , exp(exp) { };

    ASTKind getASTKind() override { return ASTKind::Assign; }
    Assign* clone() override;
    void accept(AST_Visitor& v) override { v.visit(this); }
};

// 语句->类方法调用语句: 类对象.方法名(形参列表);
// STM: EXP '.' ID '(' EXPLIST ')' ';'
//    | EXP '.' ID '(' ')' ';'
class CallStm : public Stm {
public:
    Exp* obj = nullptr;      // 类对象
    IdExp* method = nullptr; // 类方法名
    vector<Exp*>* param = new vector<Exp*>();

    CallStm(Pos* pos, Exp* obj, IdExp* method, vector<Exp*>* param)
        : Stm(pos)
        , obj(obj)
        , method(method)
        , param(param) { };
    CallStm(Pos* pos, Exp* obj, IdExp* method)
        : Stm(pos)
        , obj(obj)
        , method(method)
        , param(nullptr) { };

    ASTKind getASTKind() override { return ASTKind::CallStm; }
    CallStm* clone() override;
    void accept(AST_Visitor& v) override { v.visit(this); }
};

// 语句->continue语句: continue;
// STM: CONTINUE ';'
class Continue : public Stm {
public:
    Continue(Pos* pos)
        : Stm(pos) { };
    ASTKind getASTKind() override { return ASTKind::Continue; }
    Continue* clone() override;
    void accept(AST_Visitor& v) override { v.visit(this); }
};

// 语句->break语句: break;
// STM: BREAK ';'
class Break : public Stm {
public:
    Break(Pos* pos)
        : Stm(pos) { };
    ASTKind getASTKind() override { return ASTKind::Break; }
    Break* clone() override;
    void accept(AST_Visitor& v) override { v.visit(this); }
};

// 语句->return语句: return 表达式;
// STM: RETURN EXP ';'
class Return : public Stm {
public:
    Exp* exp = nullptr;
    Return(Pos* pos, Exp* exp)
        : Stm(pos)
        , exp(exp) { };
    ASTKind getASTKind() override { return ASTKind::Return; }
    Return* clone() override;
    void accept(AST_Visitor& v) override { v.visit(this); }
};

// 语句->打印整数语句: putint(表达式);
// STM: PUTINT '(' EXP ')' ';'
class PutInt : public Stm {
public:
    Exp* exp = nullptr;
    PutInt(Pos* pos, Exp* exp)
        : Stm(pos)
        , exp(exp) { };
    ASTKind getASTKind() override { return ASTKind::PutInt; }
    PutInt* clone() override;
    void accept(AST_Visitor& v) override { v.visit(this); }
};

// 语句->打印字符语句: putch(表达式);
// STM: PUTCH '(' EXP ')' ';'
class PutCh : public Stm {
public:
    Exp* exp = nullptr;

    PutCh(Pos* pos, Exp* exp)
        : Stm(pos)
        , exp(exp) { };
    ASTKind getASTKind() override { return ASTKind::PutCh; }
    PutCh* clone() override;
    void accept(AST_Visitor& v) override { v.visit(this); }
};

// 语句->打印数组语句: putarray(长度表达式, 数组表达式);
// STM: PUTARRAY '(' EXP ',' EXP ')' ';'
class PutArray : public Stm {
public:
    Exp* n = nullptr;   // 打印长度
    Exp* arr = nullptr; // 数组

    PutArray(Pos* pos, Exp* n, Exp* arr)
        : Stm(pos)
        , n(n)
        , arr(arr) { };
    ASTKind getASTKind() override { return ASTKind::PutArray; }
    PutArray* clone() override;
    void accept(AST_Visitor& v) override { v.visit(this); }
};

// 语句->开始计时语句: starttime();
// STM: STARTTIME '(' ')' ';'
class Starttime : public Stm {
public:
    Starttime(Pos* pos)
        : Stm(pos) { };
    ASTKind getASTKind() override { return ASTKind::Starttime; }
    Starttime* clone() override;
    void accept(AST_Visitor& v) override { v.visit(this); }
};

// 语句->停止计时语句: stoptime();
// STM: STOPTIME '(' ')' ';'
class Stoptime : public Stm {
public:
    Stoptime(Pos* pos)
        : Stm(pos) { };
    ASTKind getASTKind() override { return ASTKind::Stoptime; }
    Stoptime* clone() override;
    void accept(AST_Visitor& v) override { v.visit(this); }
};

// 表达式
// EXP -> '(' EXP ')'
//      | '(' '{' STMLIST '}' EXP ')'
//      | ID
//      | NUM
//      | TRUE | FALSE
//      | EXP '[' EXP ']'
//      | OP
//      | EXP [+-*/ COMP && ||] EXP
//      | [-!] EXP
//      | THIS
//      | EXP '.' ID '(' EXPLIST ')'
//      | EXP '.' ID '(' ')'
//      | EXP '.' ID
//      | GETINT '(' ')'
//      | GETCH '(' ')'
//      | GETARRAY '(' EXP ')'
//      | LENGTH '(' EXP ')'
class Exp : public AST {
public:
    Exp(Pos* pos)
        : AST(pos) { };
    virtual ASTKind getASTKind() = 0;
};

// 表达式->逃逸表达式: ({ 语句列表 } 表达式)
// EXP: '(' '{' STMLIST '}' EXP ')'
class Esc : public Exp {
public:
    vector<Stm*>* stmList = new vector<Stm*>();
    Exp* exp = nullptr;

    Esc(Pos* pos, vector<Stm*>* stmList, Exp* exp)
        : Exp(pos)
        , stmList(stmList)
        , exp(exp) { };
    ASTKind getASTKind() override { return ASTKind::Esc; }
    Esc* clone() override;
    void accept(AST_Visitor& v) override { v.visit(this); }
};

// 表达式->标识符: id
// EXP: ID
class IdExp : public Exp {
public:
    string id;
    IdExp(Pos* pos, string id)
        : Exp(pos)
        , id(id) { };
    ASTKind getASTKind() override { return ASTKind::IdExp; }
    IdExp* clone() override;
    void accept(AST_Visitor& v) override { v.visit(this); }
};

// 表达式->整数: num
// EXP: NUM
class IntExp : public Exp {
public:
    int val;
    IntExp(Pos* pos, int val)
        : Exp(pos)
        , val(val) { };
    ASTKind getASTKind() override { return ASTKind::IntExp; }
    IntExp* clone() override;
    void accept(AST_Visitor& v) override { v.visit(this); }
};

// 表达式->布尔常量: true | false
// EXP: TRUE | FALSE
class BoolExp : public Exp {
public:
    bool val;
    BoolExp(Pos* pos, bool val)
        : Exp(pos)
        , val(val) { };
    ASTKind getASTKind() override { return ASTKind::BoolExp; }
    BoolExp* clone() override;
    void accept(AST_Visitor& v) override { v.visit(this); }
};

// 表达式->数组访问: 数组表达式[下标表达式]
// EXP: EXP '[' EXP ']'
class ArrayExp : public Exp {
public:
    Exp* arr = nullptr;
    Exp* index = nullptr;
    ArrayExp(Pos* pos, Exp* arr, Exp* index)
        : Exp(pos)
        , arr(arr)
        , index(index) { };
    ASTKind getASTKind() override { return ASTKind::ArrayExp; }
    ArrayExp* clone() override;
    void accept(AST_Visitor& v) override { v.visit(this); }
};

// 表达式->操作符: op
// EXP: [+-*/ COMP && ||] [-!]
class OpExp : public Exp {
public:
    string op;
    OpExp(Pos* pos, string op)
        : Exp(pos)
        , op(op) { };
    ASTKind getASTKind() override { return ASTKind::OpExp; }
    OpExp* clone() override;
    void accept(AST_Visitor& v) override { v.visit(this); }
};

// 表达式->二元操作: 左表达式 OP 右表达式
// EXP: EXP OP EXP
class BinaryOp : public Exp {
public:
    Exp* left = nullptr;
    OpExp* op = nullptr;
    Exp* right = nullptr;
    BinaryOp(Pos* pos, Exp* left, OpExp* op, Exp* right)
        : Exp(pos)
        , left(left)
        , op(op)
        , right(right) { };
    ASTKind getASTKind() override { return ASTKind::BinaryOp; }
    BinaryOp* clone() override;
    void accept(AST_Visitor& v) override { v.visit(this); }
};

// 表达式->一元操作: OP 表达式
// EXP: '!' EXP | '-' EXP
class UnaryOp : public Exp {
public:
    OpExp* op = nullptr;
    Exp* exp = nullptr;
    UnaryOp(Pos* pos, OpExp* op, Exp* exp)
        : Exp(pos)
        , op(op)
        , exp(exp) { };
    ASTKind getASTKind() override { return ASTKind::UnaryOp; }
    UnaryOp* clone() override;
    void accept(AST_Visitor& v) override { v.visit(this); }
};

// 表达式->this指针: this
// EXP: THIS
class This : public Exp {
public:
    This(Pos* pos)
        : Exp(pos) { };
    ASTKind getASTKind() override { return ASTKind::This; }
    This* clone() override;
    void accept(AST_Visitor& v) override { v.visit(this); }
};

// 表达式->类方法调用: 类对象.方法名(形参列表)
// EXP: EXP '.' ID '(' EXPLIST ')'
//    | EXP '.' ID '(' ')'
class CallExp : public Exp {
public:
    Exp* obj = nullptr;      // 类对象
    IdExp* method = nullptr; // 类方法名
    vector<Exp*>* param = new vector<Exp*>();

    CallExp(Pos* pos, Exp* obj, IdExp* method, vector<Exp*>* param)
        : Exp(pos)
        , obj(obj)
        , method(method)
        , param(param) { };
    CallExp(Pos* pos, Exp* obj, IdExp* method)
        : Exp(pos)
        , obj(obj)
        , method(method)
        , param(nullptr) { };
    ASTKind getASTKind() override { return ASTKind::CallExp; }
    CallExp* clone() override;
    void accept(AST_Visitor& v) override { v.visit(this); }
};

// 表达式->类变量访问: 类对象.变量名
// EXP: EXP '.' ID
class ClassVar : public Exp {
public:
    Exp* obj = nullptr;  // 类对象
    IdExp* id = nullptr; // 类内部变量名

    ClassVar(Pos* pos, Exp* obj, IdExp* id)
        : Exp(pos)
        , obj(obj)
        , id(id) { };
    ASTKind getASTKind() override { return ASTKind::ClassVar; }
    ClassVar* clone() override;
    void accept(AST_Visitor& v) override { v.visit(this); }
};

// 表达式->读取整数: getint()
// EXP: GETINT '(' ')'
class GetInt : public Exp {
public:
    GetInt(Pos* pos)
        : Exp(pos) { };
    ASTKind getASTKind() override { return ASTKind::GetInt; }
    GetInt* clone() override;
    void accept(AST_Visitor& v) override { v.visit(this); }
};

// 表达式->读取字符: getch()
// EXP: GETCH '(' ')'
class GetCh : public Exp {
public:
    GetCh(Pos* pos)
        : Exp(pos) { };
    ASTKind getASTKind() override { return ASTKind::GetCh; }
    GetCh* clone() override;
    void accept(AST_Visitor& v) override { v.visit(this); }
};

// 表达式->读取数组: getarray(数组变量)
// EXP: GETARRAY '(' EXP ')'
class GetArray : public Exp {
public:
    Exp* exp = nullptr; // 数组变量
    GetArray(Pos* pos, Exp* exp)
        : Exp(pos)
        , exp(exp) { };
    ASTKind getASTKind() override { return ASTKind::GetArray; }
    GetArray* clone() override;
    void accept(AST_Visitor& v) override { v.visit(this); }
};

// 表达式->获取数组长度: length(数组表达式)
// EXP: LENGTH '(' EXP ')'
class Length : public Exp {
public:
    Exp* exp = nullptr;
    Length(Pos* pos, Exp* exp)
        : Exp(pos)
        , exp(exp) { };
    ASTKind getASTKind() override { return ASTKind::Length; }
    Length* clone() override;
    void accept(AST_Visitor& v) override { v.visit(this); }
};

} // namespace fdmj

#endif