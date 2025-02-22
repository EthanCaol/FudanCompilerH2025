#ifndef _EXECUTOR_H
#define _EXECUTOR_H

#include "ASTheader.hh"
#include "FDMJAST.hh"
#include <unordered_map>

using namespace std;
using namespace fdmj;

int execute(Program* root);

class Executer : public ASTVisitor {
public:
    AST* newNode = nullptr;
    Executer(AST* newNode)
        : newNode(newNode)
    {
    }

    unordered_map<string, int> varTable;

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