#ifndef _FDMJAST_HH
#define _FDMJAST_HH

// This file defines all the AST node classes

#include "ASTheader.hh"
#include "FDMJAST.hh"
#include <string>
#include <vector>

using namespace std;

namespace fdmj {

// 程序
// PROG: MAINMETHOD;
class Program : public AST {
public:
    MainMethod* main;
    Program(Pos* pos, MainMethod* main)
        : AST(pos)
        , main(main)
    {
    }
    ASTKind getASTKind() override { return ASTKind::Program; }
    Program* clone() override;
    void accept(ASTVisitor& v) override { v.visit(this); }
};

// 主方法
// MAINMETHOD: PUBLIC INT MAIN '(' ')' '{' STMLIST '}';
class MainMethod : public AST {
public:
    vector<Stm*>* sl;
    MainMethod(Pos* pos, vector<Stm*>* sl)
        : AST(pos)
        , sl(sl)
    {
    }
    ASTKind getASTKind() override { return ASTKind::MainMethod; }
    MainMethod* clone() override;
    void accept(ASTVisitor& v) override { v.visit(this); }
};

// ------------------------------------------------

// 语句
// STM: ID '=' EXP ';'
//    | RETURN EXP ';';
class Stm : public AST {
public:
    Stm(Pos* pos)
        : AST(pos)
    {
    }
    virtual ASTKind getASTKind() = 0;
};

// 语句: 赋值
// STM: ID '=' EXP ';'
class Assign : public Stm {
public:
    IdExp* left = nullptr;
    Exp* exp = nullptr;
    Assign(Pos* pos, IdExp* left, Exp* exp)
        : Stm(pos)
        , left(left)
        , exp(exp)
    {
    }
    ASTKind getASTKind() override { return ASTKind::Assign; }
    Assign* clone() override;
    void accept(ASTVisitor& v) override { v.visit(this); }
};

// 语句: 返回
// STM: RETURN EXP ';'
class Return : public Stm {
public:
    Exp* exp = nullptr;
    Return(Pos* pos, Exp* exp)
        : Stm(pos)
        , exp(exp)
    {
    }

    int retVal = 0;
    Return(Pos* pos, int val)
        : Stm(pos)
        , retVal(val)
    {
    }

    ASTKind getASTKind() override { return ASTKind::Return; }
    Return* clone() override;
    void accept(ASTVisitor& v) override { v.visit(this); }
};

// ------------------------------------------------

// 表达式
// EXP: '(' EXP ADD EXP ')'
//    | '(' EXP MINUS EXP ')'
//    | '(' EXP TIMES EXP ')'
//    | '(' EXP DIVIDE EXP ')'
//    | '(' MINUS EXP ')'
//    | '(' '{' STMLIST '}' EXP ')'
//    | '(' EXP ')'
//    | ID;
//    | NONNEGATIVEINT
class Exp : public AST {
public:
    Exp(Pos* pos)
        : AST(pos)
    {
    }
    virtual ASTKind getASTKind() = 0;
};

// 表达式: 二元运算
// EXP: '(' EXP ADD EXP ')'
//    | '(' EXP MINUS EXP ')'
//    | '(' EXP TIMES EXP ')'
//    | '(' EXP DIVIDE EXP ')'
class BinaryOp : public Exp {
public:
    Exp* left = nullptr;
    OpExp* op = nullptr;
    Exp* right = nullptr;
    BinaryOp(Pos* pos, Exp* left, OpExp* op, Exp* right)
        : Exp(pos)
        , left(left)
        , op(op)
        , right(right)
    {
    }
    ASTKind getASTKind() override { return ASTKind::BinaryOp; }
    BinaryOp* clone() override;
    void accept(ASTVisitor& v) override { v.visit(this); }
};

// 表达式: 一元运算
// EXP: '(' MINUS EXP ')'
class UnaryOp : public Exp {
public:
    OpExp* op = nullptr;
    Exp* exp = nullptr;
    UnaryOp(Pos* pos, OpExp* op, Exp* exp)
        : Exp(pos)
        , op(op)
        , exp(exp)
    {
    }
    ASTKind getASTKind() override { return ASTKind::UnaryOp; }
    UnaryOp* clone() override;
    void accept(ASTVisitor& v) override { v.visit(this); }
};

// 表达式: 逃逸表达式
// EXP: '(' '{' STMLIST '}' EXP ')'
class Esc : public Exp {
public:
    vector<Stm*>* sl = new vector<Stm*>();
    Exp* exp = nullptr;

    Esc(Pos* pos, vector<Stm*>* sl, Exp* exp)
        : Exp(pos)
        , sl(sl)
        , exp(exp)
    {
    }
    ASTKind getASTKind() override { return ASTKind::Esc; }
    Esc* clone() override;
    void accept(ASTVisitor& v) override { v.visit(this); }
};

// 表达式: 标识符
// EXP: ID
class IdExp : public Exp {
public:
    string id;
    IdExp(Pos* pos, string id)
        : Exp(pos)
        , id(id)
    {
    }
    ASTKind getASTKind() override { return ASTKind::IdExp; }
    IdExp* clone() override;
    void accept(ASTVisitor& v) override { v.visit(this); }
};

// 表达式: 整数
// EXP: NONNEGATIVEINT
class IntExp : public Exp {
public:
    int val;
    IntExp(Pos* pos, int val)
        : Exp(pos)
        , val(val)
    {
    }
    ASTKind getASTKind() override { return ASTKind::IntExp; }
    IntExp* clone() override;
    void accept(ASTVisitor& v) override { v.visit(this); }
};

// 表达式: 操作符
// EXP: '(' EXP ADD EXP ')'
class OpExp : public Exp {
public:
    string op;
    OpExp(Pos* pos, string op)
        : Exp(pos)
        , op(op)
    {
    }
    ASTKind getASTKind() override { return ASTKind::OpExp; }
    OpExp* clone() override;
    void accept(ASTVisitor& v) override { v.visit(this); }
};

} // namespace fdmj

#endif