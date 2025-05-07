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
static void deleteUnreachableBlocks(ControlFlowInfo* domInfo);
static void placePhi(ControlFlowInfo* domInfo);
static void renameVariables(ControlFlowInfo* domInfo);
static void cleanupUnusedPhi(QuadFuncDecl* func);

// 删除不可达基本块
static void deleteUnreachableBlocks(ControlFlowInfo* domInfo) { domInfo->eliminateUnreachableBlocks(); }

// 在支配边界插入Phi函数
static void placePhi(ControlFlowInfo* domInfo)
{
    // 构造数据流信息
    DataFlowInfo dataFlowInfo(domInfo->func);
    dataFlowInfo.findAllVars();

    // 记录块的Phi函数定义过变量
    map<int, set<int>> A_phi;
    for (auto block : domInfo->allBlocks)
        A_phi[block] = set<int>();

    // 遍历所有变量
    for (auto a : dataFlowInfo.allVars) {
        auto W = dataFlowInfo.defs->at(a);
        while (!W.empty()) {
            quad::QuadBlock* block = W.begin()->first;
            quad::QuadStm* stm = W.begin()->second;
            W.erase(W.begin());

            for (int Y : domInfo->dominanceFrontiers[block->entry_label->num]) {
                // 如果该变量的phi函数未定义
                // 并且该变量在Y入口标签的liveout里, 则执行
                const auto& Y_liveout = dataFlowInfo.liveout->at(*block->quadlist->begin());
                if (A_phi[Y].find(a) == A_phi[Y].end() && Y_liveout.find(a) != Y_liveout.end()) {
                    auto tempExp = new TempExp(dataFlowInfo.varTypeMap[a], new Temp(a));
                    auto phi = new QuadPhi(nullptr, tempExp, new vector<pair<Temp*, Label*>>(), new set<Temp*>(), new set<Temp*>());
                    domInfo->labelToBlock[Y]->quadlist->insert(domInfo->labelToBlock[Y]->quadlist->begin() + 1, phi);
                }
            }
        }
    }
}

static map<int, int> Count;
static map<int, vector<int>> Stack;

static void Rename(ControlFlowInfo* domInfo, int n)
{
    QuadBlock* block = domInfo->labelToBlock[n];

    for (QuadStm* S : *(block->quadlist)) {
        if (S->kind != QuadKind::PHI) { }
    }
}

// 重命名变量(确保每个变量仅赋值一次)
static void renameVariables(ControlFlowInfo* domInfo)
{
    // 构造数据流信息
    DataFlowInfo dataFlowInfo(domInfo->func);
    dataFlowInfo.findAllVars();

    // 初始化所有变量的计数和栈
    Count.clear();
    Stack.clear();
    for (int a : dataFlowInfo.allVars) {
        Count[a] = a * 100;
        Stack[a] = vector<int> { a * 100 };
    }

    // 深度优先遍历基本块
    Rename(domInfo, domInfo->entryBlock);
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
        deleteUnreachableBlocks(domInfo);

        // 在支配边界插入Phi函数
        placePhi(domInfo);

        // 重命名变量(确保每个变量仅赋值一次)
        renameVariables(domInfo);

        // 清理无用的Phi函数
        cleanupUnusedPhi(func);

        // 将处理后的函数添加到新程序中
        ssaProgram->quadFuncDeclList->push_back(func);
    }

    return ssaProgram;
}