#ifndef _CONSTPROPAGATION_H
#define _CONSTPROPAGATION_H

#include "ASTheader.hh"
#include "FDMJAST.hh"

using namespace std;
using namespace fdmj;

Program* constantPropagate(Program* root);

class ConstantPropagater : public ASTVisitor {
public:
    AST* newNode = nullptr;
    ConstantPropagater(AST* newNode)
        : newNode(newNode)
    {
    }

    void visit(Program* node) override;
    void visit(MainMethod* node) override;
    void visit(Assign* node) override;
    void visit(Return* node) override;
    void visit(BinaryOp* node) override;
    void visit(UnaryOp* node) override;
    void visit(Esc* node) override;
    void visit(IdExp* node) override;
    void visit(OpExp* node) override;
    void visit(IntExp* node) override;
};

#endif