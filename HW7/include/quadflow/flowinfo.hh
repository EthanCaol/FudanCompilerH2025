#ifndef QUADFLOW_FLOWINFO_HH
#define QUADFLOW_FLOWINFO_HH

#include <map>
#include <set>
#include <string>
#include <vector>
#include "quad.hh"

// 控制流
class ControlFlowInfo {
public:
    quad::QuadFuncDecl* func;                // 基本块化的四元式程序
    map<int, quad::QuadBlock*> labelToBlock; // 标签到基本块的映射
    int entryBlock;                          // 函数的入口基本块

    set<int> allBlocks;         // 函数中所有基本块的集合
    set<int> unreachableBlocks; // 不可达基本块的集合

    // int 是标签编号，代表 QuadFuncDecl 中的一个基本块
    map<int, set<int>> predecessors;       // 基本块到其前驱基本块的映射
    map<int, set<int>> successors;         // 基本块到其后继基本块的映射
    map<int, set<int>> dominators;         // 基本块到其支配者的映射
    map<int, int> immediateDominator;      // 基本块到其直接支配者的映射
    map<int, set<int>> dominanceFrontiers; // 基本块到其支配边界的映射
    map<int, set<int>> domTree;            // 支配树（节点到其子基本块的映射）

    ControlFlowInfo(quad::QuadFuncDecl* func)
        : func(func)
    {
        entryBlock = -1;
        if (func->quadblocklist != nullptr && !func->quadblocklist->empty()) {
            entryBlock = func->quadblocklist->at(0)->entry_label->num;
        }
        if (entryBlock == -1) {
            cerr << "错误：函数中未找到入口基本块" << endl;
        }
        for (auto block : *func->quadblocklist) {
            if (block->entry_label) {
                labelToBlock[block->entry_label->num] = block;
            }
        }
    }

    void computeAllBlocks();           // 计算所有基本块
    void computeUnreachableBlocks();   // 计算不可达基本块
    void eliminateUnreachableBlocks(); // 从函数中移除不可达基本块，
                                       // 并清空上述所有集合和映射，
                                       // 因为可能需要重新计算！

    void printPredecessors();        // 打印前驱信息
    void printSuccessors();          // 打印后继信息
    void printDominators();          // 打印支配者信息
    void printImmediateDominators(); // 打印直接支配者信息
    void printDominanceFrontier();   // 打印支配边界信息
    void printDomTree();             // 打印支配树信息

    void computePredecessors();       // 计算前驱
    void computeSuccessors();         // 计算后继
    void computeDominators();         // 计算支配者
    void computeImmediateDominator(); // 计算直接支配者
    void computeDominanceFrontiers(); // 计算支配边界
    void computeDomTree();            // 计算支配树

    void computeEverything(); // 计算所有信息
};

class DataFlowInfo {
public:
    quad::QuadFuncDecl* func;                                    // blocked quad program
    set<int> allVars;                                            // Set of all variables
    map<int, set<pair<quad::QuadBlock*, quad::QuadStm*>>>* defs; // where the variable is used
    map<int, set<pair<quad::QuadBlock*, quad::QuadStm*>>>* uses; // where the variable is used
    map<quad::QuadStm*, set<int>>* liveout;                      // live_in vars of a statement
    map<quad::QuadStm*, set<int>>* livein;                       // live_in vars of a statment

    DataFlowInfo(quad::QuadFuncDecl* func)
        : func(func)
    {
        allVars.clear();
        defs = new map<int, set<pair<quad::QuadBlock*, quad::QuadStm*>>>();
        uses = new map<int, set<pair<quad::QuadBlock*, quad::QuadStm*>>>();
        liveout = new map<quad::QuadStm*, set<int>>();
        livein = new map<quad::QuadStm*, set<int>>();
    }

    void findAllVars();
    void computeLiveness();
    string printLiveness();
};

set<DataFlowInfo*>* dataFLowProg(quad::QuadProgram* prog);

#endif