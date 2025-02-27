#include "constantPropagation.hh"
#include "ASTheader.hh"
#include "FDMJAST.hh"
#include <iostream>
#include <variant>
#include <vector>

using namespace std;
using namespace fdmj;

#define StmList vector<Stm*>
#define ExpList vector<Exp*>

// 对外接口函数
Program* constantPropagate(Program* root)
{
    if (root == nullptr)
        return nullptr;
    ConstantPropagater v(nullptr);
    root->accept(v);
    return dynamic_cast<Program*>(v.newNode);
}

// 1. 判断参数非空
// 2. 判断子结点非空
// 3. 调用accept(visit)
// 4. 记录下层newNode
// 3. 更新本层newNode

template <typename T>
static vector<T*>* visitList(ConstantPropagater& v, vector<T*>* tl)
{
    // 1. 判断参数非空
    if (tl == nullptr || tl->size() == 0)
        return nullptr;

    vector<T*>* vt = new vector<T*>();
    for (T* x : *tl) {
        // 2. 判断子结点非空
        if (x == nullptr)
            continue;

        // 3. 调用accept(visit)
        // 4. 记录下层newNode
        x->accept(v);
        if (v.newNode == nullptr)
            continue;
        vt->push_back(static_cast<T*>(v.newNode));
    }

    // 5. 更新本层newNode
    if (vt->size() == 0) {
        delete vt;
        vt = nullptr;
    }
    return vt;
}

// 程序
// PROG: MAINMETHOD;
void ConstantPropagater::visit(Program* node)
{
    // 1. 判断参数非空
    if (node == nullptr) {
        newNode = nullptr;
        return;
    }

    // 2. 判断子结点非空
    if (node->main == nullptr) {
        cerr << "Error: No main method found in the Program" << endl;
        newNode = nullptr;
        return;
    }

    // 3. 调用accept(visit)
    // 4. 记录下层newNode
    node->main->accept(*this);
    MainMethod* m = static_cast<MainMethod*>(newNode);

    // 5. 更新本层newNode
    newNode = new Program(node->getPos()->clone(), m);
}

// 主方法
// MAINMETHOD: PUBLIC INT MAIN '(' ')' '{' STMLIST '}';
void ConstantPropagater::visit(MainMethod* node)
{
    // 1. 判断参数非空
    if (node == nullptr) {
        newNode = nullptr;
        return;
    }

    // 2. 判断子结点非空
    if (node->sl == nullptr) {
        cerr << "Error: No statement list found in the MainMethod" << endl;
        newNode = nullptr;
        return;
    }

    // 3. 调用accept(visit)
    // 4. 记录下层newNode
    StmList* sl = visitList<Stm>(*this, node->sl);

    // 5. 更新本层newNode
    newNode = new MainMethod(node->getPos()->clone(), sl);
}

// 语句: 赋值
// STM: EXP '=' EXP ';'
void ConstantPropagater::visit(Assign* node)
{
    // 1. 判断参数非空
    if (node == nullptr) {
        newNode = nullptr;
        return;
    }

    // 2. 判断子结点非空
    if (node->left == nullptr) {
        cerr << "Error: No left expression found in the Assign statement" << endl;
        newNode = nullptr;
        return;
    }
    if (node->exp == nullptr) {
        cerr << "Error: No right expression found in the Assign statement" << endl;
        newNode = nullptr;
        return;
    }

    // 3. 调用accept(visit)
    // 4. 记录下层newNode
    node->left->accept(*this);
    IdExp* l = static_cast<IdExp*>(newNode);

    // 3. 调用accept(visit)
    // 4. 记录下层newNode
    node->exp->accept(*this);
    Exp* r = static_cast<Exp*>(newNode);

    // 5. 更新本层newNode
    newNode = new Assign(node->getPos()->clone(), l, r);
}

// 语句: 返回
// STM: RETURN EXP ';'
void ConstantPropagater::visit(Return* node)
{
    // 1. 判断参数非空
    if (node == nullptr) {
        newNode = nullptr;
        return;
    }

    // 2. 判断子结点非空
    if (node->exp == nullptr) {
        cerr << "Error: No expression found in the Return statement" << endl;
        newNode = nullptr;
        return;
    }

    // 3. 调用accept(visit)
    // 4. 记录下层newNode
    node->exp->accept(*this);
    Exp* e = static_cast<Exp*>(newNode);

    // 3. 更新本层newNode
    newNode = new Return(node->getPos()->clone(), e);
}

// 表达式: 二元运算
// EXP: '(' EXP ADD EXP ')'
//     | '(' EXP MINUS EXP ')'
//     | '(' EXP TIMES EXP ')'
//     | '(' EXP DIVIDE EXP ')'
void ConstantPropagater::visit(BinaryOp* node)
{
    // 1. 判断参数非空
    if (node == nullptr) {
        newNode = nullptr;
        return;
    }

    // 2. 判断子结点非空
    if (node->left == nullptr) {
        cerr << "Error: No left expression found in the BinaryOp statement" << endl;
        newNode = nullptr;
        return;
    }
    if (node->op == nullptr) {
        cerr << "Error: No operator found in the BinaryOp statement" << endl;
        newNode = nullptr;
        return;
    }
    if (node->right == nullptr) {
        cerr << "Error: No right expression found in the BinaryOp statement" << endl;
        newNode = nullptr;
        return;
    }

    // 3. 调用accept(visit)
    // 4. 记录下层newNode
    node->left->accept(*this);
    Exp* l = static_cast<Exp*>(newNode);

    // 3. 调用accept(visit)
    // 4. 记录下层newNode
    node->right->accept(*this);
    Exp* r = static_cast<Exp*>(newNode);

    // 5. 更新本层newNode (执行常量运算)
    if (l->getASTKind() == ASTKind::IntExp && r->getASTKind() == ASTKind::IntExp) {
        int val = 0;
        int lval = static_cast<IntExp*>(l)->val;
        int rval = static_cast<IntExp*>(r)->val;

        if (node->op->op == "+")
            val = lval + rval;
        else if (node->op->op == "-")
            val = lval - rval;
        else if (node->op->op == "*")
            val = lval * rval;
        else if (node->op->op == "/") {
            if (rval == 0) {
                cerr << "BinaryOp: Division by zero" << endl;
                newNode = nullptr;
                return;
            }
            val = lval / rval;
        } else {
            cerr << "Error: Invalid operator found in the BinaryOp statement" << endl;
            newNode = nullptr;
            return;
        }

        newNode = new IntExp(node->getPos()->clone(), val);
        return;
    }

    // 5. 更新本层newNode
    newNode = new BinaryOp(node->getPos()->clone(), l, node->op->clone(), r);
}

// 表达式: 一元运算
// EXP: '(' MINUS EXP ')'
void ConstantPropagater::visit(UnaryOp* node)
{
    // 1. 判断参数非空
    if (node == nullptr) {
        newNode = nullptr;
        return;
    }

    // 2. 判断子结点非空
    if (node->exp == nullptr) {
        cerr << "Error: No expression found in the UnaryOp statement" << endl;
        newNode = nullptr;
        return;
    }
    if (node->op == nullptr) {
        cerr << "Error: No operator found in the UnaryOp statement" << endl;
        newNode = nullptr;
        return;
    }

    // 3. 调用accept(visit)
    // 4. 记录下层newNode
    node->exp->accept(*this);
    Exp* e = static_cast<Exp*>(newNode);

    // 5. 更新本层newNode (执行运算)
    if (node->op->op == "-" && e->getASTKind() == ASTKind::IntExp) {
        int val = -(static_cast<IntExp*>(e)->val);
        newNode = new IntExp(node->getPos()->clone(), val);
        return;
    }

    // 5. 更新本层newNode
    newNode = new UnaryOp(node->getPos()->clone(), node->op->clone(), e);
}

void ConstantPropagater::visit(Esc* node)
{
    // 1. 判断参数非空
    if (node == nullptr) {
        newNode = nullptr;
        return;
    }

    // 2. 判断子结点非空
    if (node->sl == nullptr && node->exp == nullptr) {
        cerr << "Error: No statement list or expression found in the Esc statement" << endl;
        newNode = nullptr;
        return;
    }

    // 3. 调用accept(visit)
    // 4. 记录下层newNode
    StmList* sl = visitList<Stm>(*this, node->sl);

    // 3. 调用accept(visit)
    // 4. 记录下层newNode
    node->exp->accept(*this);
    Exp* e = static_cast<Exp*>(newNode);

    // 5. 更新本层newNode
    newNode = new Esc(node->getPos()->clone(), sl, e);
}

void ConstantPropagater::visit(IdExp* node)
{
    newNode = (node == nullptr) ? nullptr : static_cast<IdExp*>(node->clone());
}

void ConstantPropagater::visit(OpExp* node)
{
    newNode = (node == nullptr) ? nullptr : static_cast<OpExp*>(node->clone());
}

void ConstantPropagater::visit(IntExp* node)
{
    newNode = (node == nullptr) ? nullptr : static_cast<IntExp*>(node->clone());
}