#define DEBUG
// #undef DEBUG

#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <algorithm>
#include "config.hh"
#include "ASTheader.hh"
#include "FDMJAST.hh"
#include "treep.hh"
#include "temp.hh"
#include "ast2tree.hh"
#include "namemaps.hh"

using namespace std;
// using namespace fdmj;
// using namespace tree;

Class_table* generate_class_table(AST_Semant_Map* semant_map) { return nullptr; }

// 为当前函数生成方法变量表
Method_var_table* generate_method_var_table(string class_name, string method_name, Name_Maps* nm, Temp_map* tm)
{
    auto var_temp_map = new map<string, tree::Temp*>; // 变量名->变量结点
    auto var_type_map = new map<string, tree::Type>;   // 变量名->类型结点

    // 首先填充方法参数
    vector<string>* method_formal_list = nm->get_method_formal_list(class_name, method_name);
    for (auto f_str : *method_formal_list) {
        fdmj::Formal* f = nm->get_method_formal(class_name, method_name, f_str);
        tree::Temp* f_temp = tm->newtemp();
        string f_id = f->id->id;
        tree::Type f_type = f->type->typeKind == TypeKind::INT ? tree::Type::INT : tree::Type::PTR;
        (*var_temp_map)[f_id] = f_temp;
        (*var_type_map)[f_id] = f_type;
    }

    // 然后填充局部变量
    set<string>* method_var_list = nm->get_method_var_list(class_name, method_name);
    for (auto v_str : *method_var_list) {
        fdmj::VarDecl* vd = nm->get_method_var(class_name, method_name, v_str);
        tree::Temp* v_temp = tm->newtemp();
        string v_id = vd->id->id;
        tree::Type v_type = vd->type->typeKind == TypeKind::INT ? tree::Type::INT : tree::Type::PTR;
        (*var_temp_map)[v_id] = v_temp;
        (*var_type_map)[v_id] = v_type;
    }

    return new Method_var_table(var_temp_map, var_type_map);
}

// TODO
tree::Program* ast2tree(fdmj::Program* prog, AST_Semant_Map* semant_map)
{
    ASTToTreeVisitor visitor = ASTToTreeVisitor(semant_map);
    prog->accept(visitor);
    return visitor.getTree();
}

// 程序: 主方法 类声明列表
// PROG: MAINMETHOD CLASSDECLLIST
void ASTToTreeVisitor::visit(fdmj::Program* node)
{
    // 生成类表
    class_table = generate_class_table(semant_map);

    // 开始填充fdl
    vector<tree::FuncDecl*>* fdl = new vector<tree::FuncDecl*>();

    // 主方法
    node->main->accept(*this);
    assert(newNodes.size() == 1);
    fdl->push_back(static_cast<tree::FuncDecl*>(newNodes[0]));
    newNodes.clear();

    // 类声明列表
    for (auto cd : *(node->cdl)) {
        cd->accept(*this); // 类声明
        assert(newNodes.size() == 1);
        fdl->push_back(static_cast<tree::FuncDecl*>(newNodes[0]));
        newNodes.clear();
    }

    tree_root = new tree::Program(fdl);
}

// 主方法: public int main() { 变量声明列表 语句列表 }
// MAINMETHOD: PUBLIC INT MAIN '(' ')' '{' VARDECLLIST STMLIST '}';
// FunctionDeclaration Blocks Block Statements
void ASTToTreeVisitor::visit(fdmj::MainMethod* node)
{
    class_name = MAIN_class_name;
    method_name = MAIN_method_name;

    // 为当前函数生成方法变量表
    method_var_table = generate_method_var_table(class_name, method_name, semant_map->getNameMaps(), &temp_map);

    // 形参列表
    vector<tree::Temp*>* args = new vector<tree::Temp*>();
    vector<tree::Stm*>* sl = new vector<tree::Stm*>();

    // 变量声明列表
    for (fdmj::VarDecl* vd : *node->vdl) {
        vd->accept(*this);
        for (auto& newNode : newNodes)
            if (newNode)
                sl->push_back(static_cast<tree::Stm*>(newNode));
        newNodes.clear();
    }

    // 语句列表
    for (fdmj::Stm* stm : *node->sl) {
        stm->accept(*this);
        for (auto& newNode : newNodes)
            if (newNode)
                sl->push_back(static_cast<tree::Stm*>(newNode));
        newNodes.clear();
    }

    // 在开头插入返回标签
    Label* entry_label = temp_map.newlabel();
    sl->insert(sl->begin(), new tree::LabelStm(entry_label));

    // 构造Blocks
    tree::Block* bk = new tree::Block(entry_label, nullptr, sl);
    vector<tree::Block*>* bkl = new vector<tree::Block*>();
    bkl->push_back(bk);

    int lt = temp_map.next_temp - 1;
    int ll = temp_map.next_label - 1;
    newNodes.push_back(new tree::FuncDecl(class_name + "^" + method_name, args, bkl, tree::Type::INT, lt, ll));
}

// 类声明: 类名 [基类名] { 变量声明列表 方法声明列表 }
// CLASSDECL: PUBLIC CLASS ID '{' VARDECLLIST METHODDECLLIST '}'
//          | PUBLIC CLASS ID EXTENDS ID '{' VARDECLLIST METHODDECLLIST '}'
void ASTToTreeVisitor::visit(fdmj::ClassDecl* node) { }

// 类型:  整型 | 整型数组 | 类
// TYPE: INT | INT '[' ']' | CLASS ID
void ASTToTreeVisitor::visit(fdmj::Type* node) { }

// 变量声明
// VARDECL: CLASS ID ID ';'
//        | INT ID ';'
//        | INT '[' ']' ID ';'
//        | INT '[' NUM ']' ID ';'
//        | INT ID '=' CONST ';'
//        | INT '[' ']' ID '=' '{' CONSTLIST '}' ';'
//        | INT '[' NUM ']' ID '=' '{' CONSTLIST '}' ';'
void ASTToTreeVisitor::visit(fdmj::VarDecl* node)
{
    auto type = node->type;
    auto id = node->id;
    auto init = node->init;

    auto temp = method_var_table->get_var_temp(id->id);

    // 整型
    if (type->typeKind == TypeKind::INT) {
        if (holds_alternative<fdmj::IntExp*>(init)) {
            int val = get<fdmj::IntExp*>(init)->val;
            tree::TempExp* tempExp = new tree::TempExp(tree::Type::INT, temp);
            tree::Const* sizeConst = new tree::Const(val);
            newNodes.push_back(new tree::Move(tempExp, sizeConst));
        }
    }

    // 整型数组
    else if (type->typeKind == TypeKind::ARRAY) {
        int size; // 计算数组大小
        int int_length = Compiler_Config::get("int_length");
        if (type->arity != nullptr)
            size = type->arity->val;
        else if (holds_alternative<vector<IntExp*>*>(init))
            size = get<vector<IntExp*>*>(init)->size();

        // malloc((size+1) * int_length)
        auto args = new vector<tree::Exp*>();
        args->push_back(new tree::Const((size + 1) * int_length));
        auto ext_call = new tree::ExtCall(tree::Type::PTR, "malloc", args);

        // temp=malloc((size+1) * int_length)
        tree::TempExp* tempExp = new tree::TempExp(tree::Type::PTR, temp);
        newNodes.push_back(new tree::Move(tempExp, ext_call));

        // temp.size=size
        auto size_mem = new tree::Mem(tree::Type::INT, tempExp);
        newNodes.push_back(new tree::Move(size_mem, new tree::Const(size)));

        if (holds_alternative<vector<IntExp*>*>(init)) {
            auto init_list = *get<vector<IntExp*>*>(init);
            for (int i = 0; i < init_list.size(); i++) {
                auto valExp = init_list[i];
                auto offset = new tree::Const((i + 1) * int_length);
                auto index_mem = new tree::Mem(tree::Type::INT, new tree::Binop(tree::Type::PTR, "+", tempExp, offset));
                newNodes.push_back(new tree::Move(index_mem, new tree::Const(valExp->val)));
            }
        }
    }

    // 类
    else if (node->type->typeKind == TypeKind::CLASS) {
    }
}

// 方法声明: 返回类型 方法名(形参列表) { 变量声明列表 语句列表 }
// METHODDECL: PUBLIC TYPE ID '(' FORMALLIST ')' '{' VARDECLLIST STMLIST '}'
void ASTToTreeVisitor::visit(fdmj::MethodDecl* node) { }

// 形参: 类型 变量名
// FORMAL: TYPE ID
void ASTToTreeVisitor::visit(fdmj::Formal* node) { }

// 语句
// STM: '{' STMLIST '}'
//      | IF '(' EXP ')' STM ELSE STM
//      | IF '(' EXP ')' STM
//      | WHILE '(' EXP ')' STM
//      | WHILE '(' EXP ')' ';'
//      | EXP '=' EXP ';'
//      | EXP '.' ID '(' EXPLIST ')' ';'
//      | CONTINUE ';'
//      | BREAK ';'
//      | RETURN EXP ';'
//      | PUTINT '(' EXP ')' ';'
//      | PUTCH '(' EXP ')' ';'
//      | PUTARRAY '(' EXP ',' EXP ')' ';'
//      | STARTTIME '(' ')' ';'
//      | STOPTIME '(' ')' ';'

// 语句->语句块
// NESTED: '{' STMLIST '}'
void ASTToTreeVisitor::visit(fdmj::Nested* node)
{
    vector<tree::Stm*>* sl = new vector<tree::Stm*>();
    for (fdmj::Stm* stm : *node->sl) {
        stm->accept(*this);
        for (auto newNode : newNodes)
            if (newNode)
                sl->push_back(static_cast<tree::Stm*>(newNode));
        newNodes.clear();
    }
    newNodes.push_back(new tree::Seq(sl));
}

// 语句->if语句: if (条件表达式) 语句1 [else 语句2]
// STM: IF '(' EXP ')' STM ELSE STM
//    | IF '(' EXP ')' STM
void ASTToTreeVisitor::visit(fdmj::If* node)
{
    node->exp->accept(*this);
    auto exp_cx = newExp->unCx(&temp_map);

    // exp_cx:
    //   True_patch_list: *L1
    //   False_patch_list: *L2
    // If:
    //   exp_cx.stm
    //   L_true [L1]
    //   stm1
    //   Jump L_end
    //   L_false [L2]
    //   stm2
    //   L_end

    auto L1 = exp_cx->true_list;
    auto L2 = exp_cx->false_list;
    auto L_true = temp_map.newlabel();
    auto L_false = temp_map.newlabel();
    auto L_end = temp_map.newlabel();

    // 用L_true填补L1, 用L_false填补L2
    L1->patch(L_true);
    L2->patch(L_false);

    vector<tree::Stm*>* sl = new vector<tree::Stm*>();
    sl->push_back(exp_cx->stm);
    sl->push_back(new tree::LabelStm(L_true));

    node->stm1->accept(*this);
    auto stm1 = static_cast<tree::Stm*>(newNodes[0]);
    newNodes.clear();
    sl->push_back(stm1);
    sl->push_back(new tree::Jump(L_end));

    sl->push_back(new tree::LabelStm(L_false));
    if (node->stm2) {
        node->stm2->accept(*this);
        auto stm2 = static_cast<tree::Stm*>(newNodes[0]);
        newNodes.clear();
        sl->push_back(stm2);
    }

    sl->push_back(new tree::LabelStm(L_end));
    newNodes.push_back(new tree::Seq(sl));
}

// 语句->while语句: 辅助函数
static vector<tree::Stm*>* while_helper(tree::Temp_map& temp_map, Tr_Exp* exp, vector<tree::Stm*>& body)
{
    // exp_cx:
    //   True_patch_list: *L1
    //   False_patch_list: *L2
    // While:
    //   L_while
    //   exp_cx.stm
    //   L_true [L1]
    //   stm
    //   Jump L_while
    //   L_end [L2]

    auto exp_cx = exp->unCx(&temp_map);
    auto L1 = exp_cx->true_list;
    auto L2 = exp_cx->false_list;

    auto L_while = temp_map.newlabel();
    auto L_true = temp_map.newlabel();
    auto L_end = temp_map.newlabel();

    L1->patch(L_true);
    L2->patch(L_end);

    vector<tree::Stm*>* sl = new vector<tree::Stm*>();
    sl->push_back(new tree::LabelStm(L_while));
    sl->push_back(exp_cx->stm);

    sl->push_back(new tree::LabelStm(L_true));
    for (auto& stm : body)
        sl->push_back(stm);
    sl->push_back(new tree::Jump(L_while));
    sl->push_back(new tree::LabelStm(L_end));
    return sl;
}

// 语句->while语句: while (条件表达式) 语句
// STM: WHILE '(' EXP ')' STM
//    | WHILE '(' EXP ')' ';'
void ASTToTreeVisitor::visit(fdmj::While* node)
{
    node->exp->accept(*this);
    vector<tree::Stm*> body;
    if (node->stm) {
        node->stm->accept(*this);
        for (auto& newNode : newNodes)
            if (newNode)
                body.push_back(static_cast<tree::Stm*>(newNode));
        newNodes.clear();
    }

    auto sl = while_helper(temp_map, newExp, body);
    newNodes.push_back(new tree::Seq(sl));
}

// 语句->赋值语句: 左值表达式 = 右值表达式;
// STM: EXP '=' EXP ';'
void ASTToTreeVisitor::visit(fdmj::Assign* node)
{
    node->left->accept(*this);
    auto left = newExp->unEx(&temp_map)->exp;

    node->exp->accept(*this);
    auto exp = newExp->unEx(&temp_map)->exp;

    newNodes.push_back(new tree::Move(left, exp));
}

// 语句->类方法调用: 类对象.方法名(形参列表);
// STM: EXP '.' ID '(' EXPLIST ')' ';'
void ASTToTreeVisitor::visit(fdmj::CallStm* node) { }

// 语句->continue语句: continue;
// STM: CONTINUE ';'
void ASTToTreeVisitor::visit(fdmj::Continue* node) { newNodes.push_back(new tree::Jump(cur_L_while)); }

// 语句->break语句: break;
// STM: BREAK ';'
void ASTToTreeVisitor::visit(fdmj::Break* node) { newNodes.push_back(new tree::Jump(cur_L_end)); }

// 语句->return语句: return 表达式;
// STM: RETURN EXP ';'
void ASTToTreeVisitor::visit(fdmj::Return* node)
{
    node->exp->accept(*this);
    auto exp = newExp->unEx(&temp_map)->exp;
    newNodes.push_back(new tree::Return(exp));
}

// 语句->打印整数语句: putint(表达式);
// STM: PUTINT '(' EXP ')' ';'
void ASTToTreeVisitor::visit(fdmj::PutInt* node)
{
    node->exp->accept(*this);
    auto exp = newExp->unEx(&temp_map)->exp;

    auto args = new vector<tree::Exp*>();
    args->push_back(exp);

    auto ext_call = new tree::ExtCall(tree::Type::INT, "putint", args);
    newNodes.push_back(new tree::ExpStm(ext_call));
}

// 语句->打印字符语句: putch(表达式);
// STM: PUTCH '(' EXP ')' ';'
void ASTToTreeVisitor::visit(fdmj::PutCh* node)
{
    node->exp->accept(*this);
    auto exp = newExp->unEx(&temp_map)->exp;

    auto args = new vector<tree::Exp*>();
    args->push_back(exp);

    auto ext_call = new tree::ExtCall(tree::Type::INT, "putch", args);
    newNodes.push_back(new tree::ExpStm(ext_call));
}

// 语句->打印数组语句: putarray(长度表达式, 数组表达式);
// STM: PUTARRAY '(' EXP ',' EXP ')' ';'
void ASTToTreeVisitor::visit(fdmj::PutArray* node) { }

// 语句->开始计时语句: starttime();
// STM: STARTTIME '(' ')' ';'
void ASTToTreeVisitor::visit(fdmj::Starttime* node)
{
    auto args = new vector<tree::Exp*>();
    auto ext_call = new tree::ExtCall(tree::Type::INT, "starttime", args);
    newNodes.push_back(new tree::ExpStm(ext_call));
}

// 语句->停止计时语句: stoptime();
// STM: STOPTIME '(' ')' ';'
void ASTToTreeVisitor::visit(fdmj::Stoptime* node)
{
    auto args = new vector<tree::Exp*>();
    auto ext_call = new tree::ExtCall(tree::Type::INT, "stoptime", args);
    newNodes.push_back(new tree::ExpStm(ext_call));
}

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
//      | EXP '.' ID
//      | GETINT '(' ')'
//      | GETCH '(' ')'
//      | GETARRAY '(' EXP ')'
//      | LENGTH '(' EXP ')'

// 表达式->逃逸表达式: ({ 语句列表 } 表达式)
// EXP: '(' '{' STMLIST '}' EXP ')'
void ASTToTreeVisitor::visit(fdmj::Esc* node)
{
    vector<tree::Stm*>* sl = new vector<tree::Stm*>();
    for (fdmj::Stm* stm : *node->sl) {
        stm->accept(*this);
        for (auto newNode : newNodes)
            if (newNode)
                sl->push_back(static_cast<tree::Stm*>(newNode));
        newNodes.clear();
    }

    node->exp->accept(*this);
    auto exp = newExp->unEx(&temp_map)->exp;
    newExp = new Tr_ex(new tree::Eseq(tree::Type::INT, new tree::Seq(sl), exp));
}

// 表达式->标识符: id
// EXP: ID
void ASTToTreeVisitor::visit(fdmj::IdExp* node)
{
    auto temp = method_var_table->get_var_temp(node->id);
    newExp = new Tr_ex(new tree::TempExp(tree::Type::INT, temp));
}

// 表达式->整数: num
// EXP: NUM
void ASTToTreeVisitor::visit(fdmj::IntExp* node)
{
    auto val = node->val;
    newExp = new Tr_ex(new tree::Const(val));
}

// 表达式->布尔常量: true | false
// EXP: TRUE | FALSE
void ASTToTreeVisitor::visit(fdmj::BoolExp* node)
{
    auto val = node->val;
    newExp = new Tr_ex(new tree::Const(val));
}

// 表达式->数组访问: 数组表达式[下标表达式]
// EXP: EXP '[' EXP ']'
void ASTToTreeVisitor::visit(fdmj::ArrayExp* node) { }

// 表达式->操作符: op
// EXP: [+-*/ COMP && ||] [-!]
void ASTToTreeVisitor::visit(fdmj::OpExp* node) { }

Tr_cx* comp_helper(Temp_map& temp_map, Tr_Exp* left_tr, Tr_Exp* right_tr, string op)
{
    auto left = left_tr->unEx(&temp_map)->exp;
    auto right = right_tr->unEx(&temp_map)->exp;

    // 构造CJump
    Label* t = temp_map.newlabel();
    Label* f = temp_map.newlabel();
    auto cjump = new tree::Cjump(op, left, right, t, f);

    // 添加修补列表
    auto true_list = new Patch_list();
    auto false_list = new Patch_list();
    true_list->add_patch(t);
    false_list->add_patch(f);
    return new Tr_cx(true_list, false_list, cjump);
}

// 表达式->二元操作: 左表达式 OP 右表达式
// EXP: EXP OP EXP
void ASTToTreeVisitor::visit(fdmj::BinaryOp* node)
{
    string op = node->op->op;

    node->left->accept(*this);
    auto left_tr = newExp;

    node->right->accept(*this);
    auto right_tr = newExp;

    vector<string> algo_op = { "+", "-", "*", "/" };
    vector<string> comp_op = { "==", "!=", "<", ">", "<=", ">=" };

    // 算数运算
    if (find(algo_op.begin(), algo_op.end(), op) != algo_op.end()) {
        auto left = left_tr->unEx(&temp_map)->exp;
        auto right = right_tr->unEx(&temp_map)->exp;
        newExp = new Tr_ex(new tree::Binop(tree::Type::INT, op, left, right));
    }

    // 比较运算
    else if (find(comp_op.begin(), comp_op.end(), op) != comp_op.end()) {
        newExp = comp_helper(temp_map, left_tr, right_tr, op);
    }

    // Tr_cx1:
    //   True_patch_list: *L1
    //   False_patch_list: *L2
    // Tr_cx2:
    //   True_patch_list: *L3
    //   False_patch_list: *L4
    // Tr_cx3:
    //   True_patch_list: *L1, *L3
    //   False_patch_list: *L4
    //   Tr_cx1.stm [L5 patches L2]
    //   L5:
    //   Tr_cx2.stm

    // 逻辑或运算
    else if (op == "||") {
        auto left_cx = left_tr->unCx(&temp_map);
        auto right_cx = right_tr->unCx(&temp_map);
        auto L1 = left_cx->true_list;
        auto L2 = left_cx->false_list;
        auto L3 = right_cx->true_list;
        auto L4 = right_cx->false_list;

        // 用L5填补L2
        auto L5 = temp_map.newlabel();
        L2->patch(L5);

        // 合并L1和L3作为新的正确分支
        L1->add(L3);

        vector<tree::Stm*>* sl = new vector<tree::Stm*>();
        sl->push_back(left_cx->stm);
        sl->push_back(new tree::LabelStm(L5));
        sl->push_back(right_cx->stm);
        newExp = new Tr_cx(L1, L4, new tree::Seq(sl));
    }

    // Tr_cx1:
    //   True_patch_list: *L1
    //   False_patch_list: *L2
    // Tr_cx2:
    //   True_patch_list: *L3
    //   False_patch_list: *L4
    // Tr_cx3:
    //   True_patch_list: *L3
    //   False_patch_list: *L2, L4
    //   Tr_cx1.stm [L5 patches L1]
    //   L5:
    //   Tr_cx2.stm

    // 逻辑与运算
    else if (op == "&&") {
        auto left_cx = left_tr->unCx(&temp_map);
        auto right_cx = right_tr->unCx(&temp_map);
        auto L1 = left_cx->true_list;
        auto L2 = left_cx->false_list;
        auto L3 = right_cx->true_list;
        auto L4 = right_cx->false_list;

        // 用L5填补L1
        auto L5 = temp_map.newlabel();
        L1->patch(L5);

        // 合并L2和L4作为新的错误分支
        L2->add(L4);

        vector<tree::Stm*>* sl = new vector<tree::Stm*>();
        sl->push_back(left_cx->stm);
        sl->push_back(new tree::LabelStm(L5));
        sl->push_back(right_cx->stm);
        newExp = new Tr_cx(L3, L2, new tree::Seq(sl));
    }
}

// 表达式->一元操作: OP 表达式
// EXP: '!' EXP | '-' EXP
void ASTToTreeVisitor::visit(fdmj::UnaryOp* node)
{
    string op = node->op->op;

    node->exp->accept(*this);
    auto exp_tr = newExp;

    if (op == "-") {
        tree::Exp* exp = exp_tr->unEx(&temp_map)->exp;

        // 整型
        if (exp->type == tree::Type::INT)
            newExp = new Tr_ex(new tree::Binop(tree::Type::INT, "-", new tree::Const(0), exp));

        // 整型数组
        else if (exp->type == tree::Type::PTR) {
            vector<tree::Stm*>* sl = new vector<tree::Stm*>();

            // 获取数组大小size
            auto a_size_temp = new tree::TempExp(tree::Type::INT, temp_map.newtemp());
            auto a_size_mem = new tree::Mem(tree::Type::INT, exp);
            sl->push_back(new tree::Move(a_size_temp, a_size_mem));

            // 初始化相同大小的暂存数组
            auto int_length = Compiler_Config::get("int_length");
            auto t_temp = new tree::TempExp(tree::Type::PTR, temp_map.newtemp());
            auto t_size_binop = new tree::Binop(tree::Type::INT, "*",
                new tree::Binop(tree::Type::INT, "+", a_size_temp, new tree::Const(1)), new tree::Const(int_length));
            auto t_malloc = new tree::ExtCall(tree::Type::PTR, "malloc", new vector<tree::Exp*> { t_size_binop });
            sl->push_back(new tree::Move(t_temp, t_malloc));

            // 设置暂存数组大小为size
            auto t_size_mem = new tree::Mem(tree::Type::INT, t_temp);
            sl->push_back(new tree::Move(t_size_mem, a_size_temp));

            // 初始化下标索引i=4
            auto i_temp = new tree::TempExp(tree::Type::INT, temp_map.newtemp());
            auto i_const = new tree::Const(4);
            sl->push_back(new tree::Move(i_temp, i_const));

            // 初始化边界end=(size+1)*4
            auto end_temp = new tree::TempExp(tree::Type::INT, temp_map.newtemp());
            auto end_binop = new tree::Binop(tree::Type::INT, "*",
                new tree::Binop(tree::Type::INT, "+", a_size_temp, new tree::Const(1)), new tree::Const(int_length));
            sl->push_back(new tree::Move(end_temp, end_binop));

            // while(i&lt;end)
            //   *(t+i)=0-*(a+i)
            //   i=i+4
            // auto sl = while_helper(tree::Temp_map& temp_map, Tr_Exp* exp, vector<tree::Stm*> body)
            // Tr_cx* comp_helper(Temp_map& temp_map, Tr_Exp* left_tr, Tr_Exp* right_tr, string op)

            auto i_lt_end = comp_helper(temp_map, new Tr_ex(i_temp), new Tr_ex(end_temp), "<");

            vector<tree::Stm*> body;

            // *(t+i)=0-*(a+i)
            auto t_i_mem = new tree::Mem(tree::Type::INT, new tree::Binop(tree::Type::PTR, "+", t_temp, i_temp));
            auto a_i_mem = new tree::Mem(tree::Type::INT, new tree::Binop(tree::Type::PTR, "+", exp, i_temp));
            auto minus_binop = new tree::Binop(tree::Type::INT, "-", new tree::Const(0), a_i_mem);
            body.push_back(new tree::Move(t_i_mem, minus_binop));

            // i=i+4
            auto i_plus = new tree::Binop(tree::Type::INT, "+", i_temp, new tree::Const(4));
            body.push_back(new tree::Move(i_temp, i_plus));

            // 生成while循环 合并到sl
            auto sl_while = while_helper(temp_map, i_lt_end, body);
            for (auto& stm : *sl_while)
                sl->push_back(stm);
            newExp = new Tr_ex(new tree::Eseq(tree::Type::PTR, new tree::Seq(sl), t_temp));
        }

    } else if (op == "!") {
        auto exp = exp_tr->unCx(&temp_map);
        auto L1 = exp->true_list;
        auto L2 = exp->false_list;

        // 交换L1和L2
        newExp = new Tr_cx(L2, L1, exp->stm);
    }
}

// 表达式->this指针: this
// EXP: THIS
void ASTToTreeVisitor::visit(fdmj::This* node) { }

// 表达式->类变量访问: 类对象.变量名
// EXP: EXP '.' ID
void ASTToTreeVisitor::visit(fdmj::ClassVar* node) { }

// 表达式->类方法调用: 类对象.方法名(形参列表)
// EXP: EXP '.' ID '(' EXPLIST ')'
void ASTToTreeVisitor::visit(fdmj::CallExp* node) { }

// 表达式->读取整数: getint()
// EXP: GETINT '(' ')'
void ASTToTreeVisitor::visit(fdmj::GetInt* node)
{
    auto args = new vector<tree::Exp*>();
    auto ext_call = new tree::ExtCall(tree::Type::INT, "getint", args);
    newExp = new Tr_ex(ext_call);
}

// 表达式->读取字符: getch()
// EXP: GETCH '(' ')'
void ASTToTreeVisitor::visit(fdmj::GetCh* node)
{
    auto args = new vector<tree::Exp*>();
    auto ext_call = new tree::ExtCall(tree::Type::INT, "getch", args);
    newExp = new Tr_ex(ext_call);
}

// 表达式->读取数组: getarray(数组变量)
// EXP: GETARRAY '(' EXP ')'
void ASTToTreeVisitor::visit(fdmj::GetArray* node) { }

// 表达式->获取数组长度: length(数组表达式)
// EXP: LENGTH '(' EXP ')'
void ASTToTreeVisitor::visit(fdmj::Length* node) { }