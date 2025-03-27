#define DEBUG
#undef DEBUG

#include <iostream>
#include <map>
#include <vector>
#include <algorithm>
#include "namemaps.hh"
#include "semant.hh"

using namespace std;
using namespace fdmj;

// 对外接口函数
AST_Semant_Map* semant_analyze(Program* node)
{
    if (node == nullptr) {
        cerr << "semant_analyze: 程序为空" << endl;
        return nullptr;
    }

    // 构造名称映射
    Name_Maps* name_maps = makeNameMaps(node);
    AST_Semant_Visitor semant_visitor(name_maps);

    // 执行语义分析
    semant_visitor.visit(node);
    return semant_visitor.getSemantMap();
}

// 程序: 主方法 类声明列表
// PROG: MAINMETHOD CLASSDECLLIST
void AST_Semant_Visitor::visit(Program* node)
{
    if (node == nullptr) {
        cerr << "AST_Semant_Visitor: 程序为空" << endl;
        return;
    }

    node->main->accept(*this);

    if (node->cdl != nullptr)
        for (auto cl : *(node->cdl)) {
            cl->accept(*this);
        }
}


// 主方法: public int main() { 变量声明列表 语句列表 }
// MAINMETHOD: PUBLIC INT MAIN '(' ')' '{' VARDECLLIST STMLIST '}';
void AST_Semant_Visitor::visit(MainMethod* node)
{
    if (node == nullptr) {
        cerr << "AST_Semant_Visitor: 主方法为空" << endl;
        return;
    }

    // 调用accept(visit)
    if (node->vdl != nullptr)
        for (auto vd : *(node->vdl)) {
            vd->accept(*this);
        }
    if (node->sl != nullptr)
        for (auto s : *(node->sl)) {
            s->accept(*this);
        }
}