#include "executor.hh"
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
int execute(Program* root)
{
    if (root == nullptr)
        return 0;
    Executer v(nullptr);
    root->accept(v);

    AST* node = v.newNode;
    if (node->getASTKind() != ASTKind::Program) {
        cerr << "> execute: No Program" << endl;
        return 0;
    }

    node = static_cast<Program*>(node)->main;
    if (node->getASTKind() != ASTKind::MainMethod) {
        cerr << "> execute: No MainMethod" << endl;
        return 0;
    }

    StmList* sl = static_cast<MainMethod*>(node)->sl;
    if (sl == nullptr) {
        cerr << "> execute: No StmList" << endl;
        return 0;
    }

    node = sl->back();
    if (node->getASTKind() != ASTKind::Return) {
        cerr << "> execute: No Return" << endl;
        return 0;
    }

    return static_cast<Return*>(node)->val;
}

// 判断值 (IdExp, IntExp)
static bool judgeValue(AST* e, unordered_map<string, int>& varTable)
{
    if (e->getASTKind() == ASTKind::IdExp) {
        IdExp* id = static_cast<IdExp*>(e);
        return varTable.find(id->id) != varTable.end();
    } else if (e->getASTKind() == ASTKind::IntExp)
        return true;
    else
        return false;
}

// 取出值 (IdExp, IntExp)
static int getValue(AST* e, unordered_map<string, int>& varTable)
{
    if (e->getASTKind() == ASTKind::IdExp) {
        IdExp* id = static_cast<IdExp*>(e);
        return varTable[id->id];
    } else if (e->getASTKind() == ASTKind::IntExp) {
        IntExp* ie = static_cast<IntExp*>(e);
        return ie->val;
    }
}

// 1. 判断参数非空
// 2. 判断子结点非空
// 3. 调用accept(visit)
// 4. 执行操作

template <typename T>
static vector<T*>* visitList(Executer& v, vector<T*>* tl)
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
void Executer::visit(Program* node)
{
    // 1. 判断参数非空
    if (node == nullptr) {
        newNode = nullptr;
        return;
    }

    // 2. 判断子结点非空
    if (node->main == nullptr) {
        cerr << "> Program: No MainMethod" << endl;
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
void Executer::visit(MainMethod* node)
{
    // 1. 判断参数非空
    if (node == nullptr) {
        newNode = nullptr;
        return;
    }

    // 2. 判断子结点非空
    if (node->sl == nullptr) {
        cerr << "> MainMethod: No StmList" << endl;
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
void Executer::visit(Assign* node)
{
    // 1. 判断参数非空
    if (node == nullptr) {
        newNode = nullptr;
        return;
    }

    // 2. 判断子结点非空
    if (node->left == nullptr) {
        cerr << "> Assign: No left Exp" << endl;
        newNode = nullptr;
        return;
    }
    if (node->exp == nullptr) {
        cerr << "> Assign: No right Exp" << endl;
        newNode = nullptr;
        return;
    }

    // 3. 调用accept(visit)
    // 4. 记录下层newNode
    node->left->accept(*this);
    Exp* l = static_cast<Exp*>(newNode);

    // 3. 调用accept(visit)
    // 4. 记录下层newNode
    node->exp->accept(*this);
    Exp* r = static_cast<Exp*>(newNode);

    // 5. 执行操作: 赋值
    int val = 0;
    if (judgeValue(r, varTable))
        val = getValue(r, varTable);
    else {
        cerr << "> Assign: Invalid right value Exp" << endl;
        newNode = nullptr;
        return;
    }

    // 更新变量表
    if (l->getASTKind() == ASTKind::IdExp) {
        IdExp* id = static_cast<IdExp*>(l);
        varTable[id->id] = val;
    } else {
        cerr << "> Assign: Invalid left id Exp" << endl;
        newNode = nullptr;
        return;
    }

    // 5. 返回值表达式
    newNode = new IntExp(node->getPos()->clone(), val);
}

// 语句: 返回
// STM: RETURN EXP ';'
void Executer::visit(Return* node)
{
    // 1. 判断参数非空
    if (node == nullptr) {
        newNode = nullptr;
        return;
    }

    // 2. 判断子结点非空
    if (node->exp == nullptr) {
        cerr << "> Return: No Exp" << endl;
        newNode = nullptr;
        return;
    }

    // 3. 调用accept(visit)
    node->exp->accept(*this);

    // 4. 执行操作: 取出值
    int val;
    if (judgeValue(newNode, varTable))
        val = getValue(newNode, varTable);
    else {
        cerr << "> Return: Invalid value Exp" << endl;
        newNode = nullptr;
        return;
    }

    newNode = new Return(node->getPos()->clone(), val);
}

// 表达式: 二元运算
// 返回: 计算结果
// EXP: '(' EXP ADD EXP ')'
//     | '(' EXP MINUS EXP ')'
//     | '(' EXP TIMES EXP ')'
//     | '(' EXP DIVIDE EXP ')'
void Executer::visit(BinaryOp* node)
{
    // 1. 判断参数非空
    if (node == nullptr) {
        newNode = nullptr;
        return;
    }

    // 2. 判断子结点非空
    if (node->left == nullptr) {
        cerr << "> BinaryOp: No left Exp" << endl;
        newNode = nullptr;
        return;
    }
    if (node->op == nullptr) {
        cerr << "> BinaryOp: No OpExp" << endl;
        newNode = nullptr;
        return;
    }
    if (node->right == nullptr) {
        cerr << "> BinaryOp: No right Exp" << endl;
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

    // 5. 执行操作: 二元运算
    int lval = 0, rval = 0; // 取出值
    if (judgeValue(l, varTable))
        lval = getValue(l, varTable);
    else {
        cerr << "> Binary: Invalid left value Exp" << endl;
        newNode = nullptr;
        return;
    }
    if (judgeValue(r, varTable))
        rval = getValue(r, varTable);
    else {
        cerr << "> Binary: Invalid right value Exp" << endl;
        newNode = nullptr;
        return;
    }

    // 按操作符计算
    int val = 0;
    if (node->op->op == "+")
        val = lval + rval;
    else if (node->op->op == "-")
        val = lval - rval;
    else if (node->op->op == "*")
        val = lval * rval;
    else if (node->op->op == "/")
        val = lval / rval;
    else {
        cerr << "> Binary: Invalid OpExp" << endl;
        newNode = nullptr;
        return;
    }

    // 5. 更新本层newNode
    newNode = new IntExp(node->getPos()->clone(), val);
}

// 表达式: 一元运算
// 返回: 计算结果
// EXP: '(' MINUS EXP ')'
void Executer::visit(UnaryOp* node)
{
    // 1. 判断参数非空
    if (node == nullptr) {
        newNode = nullptr;
        return;
    }

    // 2. 判断子结点非空
    if (node->exp == nullptr) {
        cerr << "> UnaryOp: No Exp" << endl;
        newNode = nullptr;
        return;
    }
    if (node->op == nullptr) {
        cerr << "> UnaryOp: No OpExp" << endl;
        newNode = nullptr;
        return;
    }

    // 3. 调用accept(visit)
    node->exp->accept(*this);

    // 4. 执行操作: 一元运算
    int val = 0; // 取出值
    if (judgeValue(node->exp, varTable))
        val = getValue(node->exp, varTable);
    else {
        cerr << "> UnaryOp: Invalid value Exp" << endl;
        newNode = nullptr;
        return;
    }

    // 按操作符计算
    if (node->op->op == "-")
        newNode = new IntExp(node->getPos()->clone(), -val);
    else {
        cerr << "> UnaryOp: Invalid OpExp" << endl;
        newNode = nullptr;
        return;
    }
}

// 表达式: 逃逸表达式
// 返回: 最后一个表达式
// EXP: '(' '{' STMLIST '}' EXP ')'
void Executer::visit(Esc* node)
{
    // 1. 判断参数非空
    if (node == nullptr) {
        newNode = nullptr;
        return;
    }

    // 2. 判断子结点非空
    if (node->sl == nullptr) {
        cerr << "> Esc: No StmList" << endl;
        newNode = nullptr;
        return;
    }

    // 3. 调用accept(visit)
    visitList<Stm>(*this, node->sl);

    // 3. 调用accept(visit)
    node->exp->accept(*this);

    // 4. 执行操作: 直接返回下层newNode
    ASTKind k = newNode->getASTKind();
}

void Executer::visit(IdExp* node) { newNode = (node == nullptr) ? nullptr : static_cast<IdExp*>(node->clone()); }
void Executer::visit(OpExp* node) { newNode = (node == nullptr) ? nullptr : static_cast<OpExp*>(node->clone()); }
void Executer::visit(IntExp* node) { newNode = (node == nullptr) ? nullptr : static_cast<IntExp*>(node->clone()); }