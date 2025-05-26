#ifndef __COLORING_HH
#define __COLORING_HH

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <stack>
#include "ig.hh"
#include "tinyxml2.hh"

using namespace std;
using namespace quad;
using namespace tinyxml2;

class Coloring {
public:
    InterferenceGraph* ig;             // 干扰图
    int k;                             // 颜色数 (寄存器数)
    map<int, set<int>> graph;          // 工作图 (邻接表)
    set<pair<int, int>> movePairs;     // 激活的移动对 (未被合并 未被冻结)
    stack<int> simplifiedNodes;        // 简化栈
    map<int, set<int>> coalescedMoves; // 合并结点
    set<int> spilled;                  // 溢出结点
    map<int, int> colors;              // 结点着色映射

    Coloring(InterferenceGraph* ig, int k)
        : ig(ig)
        , k(k)
    {
        graph = ig->graph;
        movePairs = ig->movePairs;
        simplifiedNodes = stack<int>();
        coalescedMoves = map<int, set<int>>();
        spilled = set<int>();
    };

    // 打印函数
    string printColoring();                                                // Print the current graph with other info (move pairs, etc.)
    void printColoring(iostream& io);                                      // Print the current graph with other info (move pairs, etc.)
    tinyxml2::XMLElement* coloring2xml(XMLDocument* doc, string funcname); // Getting into XML form

    // 辅助函数
    void addEdge(int u, int v);                          // 添加边
    void eraseNode(int node);                            // 从图中删除结点
    set<int> getNeighbors(int node);                     // 获取结点的邻居
    bool isMove(int n);                                  // 判断结点是否在移动对中
    static bool isMachineReg(int n) { return n < 100; }; // 判断结点是否是机器寄存器

    // 着色函数
    bool simplify();
    bool coalesce();
    bool freeze();
    bool spill();
    bool select();
};

Coloring* coloring(InterferenceGraph* ig, int k);

#endif