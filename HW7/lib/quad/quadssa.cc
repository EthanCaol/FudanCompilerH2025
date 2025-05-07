#define DEBUG
// #undef DEBUG

#include <iostream>
#include <vector>
#include <map>
#include <set>
#include <stack>
#include <queue>
#include <algorithm>
#include <utility>
#include "treep.hh"
#include "quad.hh"
#include "flowinfo.hh"
#include "quadssa.hh"
#include "temp.hh"

using namespace std;
using namespace quad;

// 函数声明前移
static void deleteUnreachableBlocks(QuadFuncDecl* func, ControlFlowInfo* domInfo);
static void placePhi(QuadFuncDecl* func, ControlFlowInfo* domInfo);
static void renameVariables(QuadFuncDecl* func, ControlFlowInfo* domInfo);
static void cleanupUnusedPhi(QuadFuncDecl* func);

// 删除不可达基本块
static void deleteUnreachableBlocks(QuadFuncDecl* func, ControlFlowInfo* domInfo) { domInfo->eliminateUnreachableBlocks(); }

// 在支配边界插入Phi函数
static void placePhi(QuadFuncDecl* func, ControlFlowInfo* domInfo)
{

    //
}

// 重命名变量(确保每个变量仅赋值一次)
static void renameVariables(QuadFuncDecl* func, ControlFlowInfo* domInfo)
{
    //
}

// 清理无用的Phi函数
static void cleanupUnusedPhi(QuadFuncDecl* func)
{
    //
}

QuadProgram* quad2ssa(QuadProgram* program)
{
    QuadProgram* ssaProgram = new QuadProgram(static_cast<tree::Program*>(program->node), new vector<QuadFuncDecl*>());

    for (auto func : *program->quadFuncDeclList) {
        // 创建控制流信息对象
        ControlFlowInfo* domInfo = new ControlFlowInfo(func);

        // 计算支配关系、前驱后继等控制流信息
        domInfo->computeEverything();

        // 删除不可达的基本块
        deleteUnreachableBlocks(func, domInfo);

        // 在支配边界插入Phi函数
        placePhi(func, domInfo);

        // 重命名变量(确保每个变量仅赋值一次)
        renameVariables(func, domInfo);

        // 清理无用的Phi函数
        cleanupUnusedPhi(func);

        // 将处理后的函数添加到新程序中
        ssaProgram->quadFuncDeclList->push_back(func);
    }

    return ssaProgram;
}