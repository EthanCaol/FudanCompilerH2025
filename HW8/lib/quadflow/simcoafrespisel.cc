#define DEBUG
#undef DEBUG

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include "temp.hh"
#include "ig.hh"
#include "coloring.hh"

bool isAnEdge(map<int, set<int>>& graph, int src, int dst)
{
    if (graph.find(src) == graph.end())
        return false; // src not in the graph
    if (graph[src].find(dst) == graph[src].end())
        return false; // dst not in the graph
    return true;      // edge exists
}

// 简化: 找到邻居数小于k的结点, 压入简化栈
// 移动指令的两个操作数结点被保护不被简化
// 返回true, 如果出现结点被简化
bool Coloring::simplify()
{
    // 遍历工作图, 寻找邻居数小于k的结点, 并且未被保护
    for (auto it = graph.begin(); it != graph.end(); it++) {
        int node = it->first;
        if (getNeighbors(node).size() < k && !isMove(node)) {
            simplifiedNodes.push(node);
            eraseNode(node);
            return true;
        }
    }
    return false;
}

// 合并: 如果两个结点属于同个Move对, 则合并
// 返回true. 如果出现结点被合并
bool Coloring::coalesce()
{
    for (auto it = movePairs.begin(); it != movePairs.end(); it++) {
        int dst = it->first; // 默认为机器寄存器
        int src = it->second;

        // 如果互相干扰, 则不合并
        if (isAnEdge(graph, src, dst))
            return false;

        // Briggs策略
        // 合并后新节点的高度数邻居(degree ≥ k)数量 < k
        auto dst_neighbors = getNeighbors(dst);
        auto src_neighbors = getNeighbors(src);
        set<int> union_neighbors;
        union_neighbors.insert(dst_neighbors.begin(), dst_neighbors.end());
        union_neighbors.insert(src_neighbors.begin(), src_neighbors.end());

        int high_num = 0;
        for (auto it = union_neighbors.begin(); it != union_neighbors.end(); it++)
            if (getNeighbors(*it).size() >= k)
                high_num++;
        if (high_num >= k)
            continue;

        // 使得dst为机器寄存器
        if (isMachineReg(src))
            swap(dst, src);

        // 移除该移动对 (双向)
        movePairs.erase(it);
        for (auto it = movePairs.begin(); it != movePairs.end(); it++) {
            if (it->first == src && it->second == dst) {
                movePairs.erase(it);
                break;
            }
        }

        // cout << "合并: " << src << " -> " << dst << endl;

        // 移除src结点
        eraseNode(src);

        // 将src的邻居添加到dst
        for (auto it = src_neighbors.begin(); it != src_neighbors.end(); it++)
            addEdge(dst, *it);

        // 将src的合并集 添加到dst
        coalescedMoves[dst].insert(src);
        if (coalescedMoves.find(src) != coalescedMoves.end()) {
            auto src_coalesced = coalescedMoves[src];
            coalescedMoves[dst].insert(src_coalesced.begin(), src_coalesced.end());
        }

        // 重命名移动对中的src
        std::set<std::pair<int, int>> newMovePairs;
        for (const auto& p : movePairs) {
            auto newPair = p;
            if (newPair.first == src)
                newPair.first = dst;
            if (newPair.second == src)
                newPair.second = dst;
            newMovePairs.insert(newPair);
        }
        movePairs = std::move(newMovePairs);
        return true;
    }

    return false;
}

// 冻结: 如果无法简化和合并, 则解除Move对的保护关系
// 返回true, 如果出现冻结
bool Coloring::freeze()
{
    if (!movePairs.empty()) {
        // 移除该移动对 (双向)
        int dst = movePairs.begin()->first;
        int src = movePairs.begin()->second;
        movePairs.erase(movePairs.begin());
        for (auto it = movePairs.begin(); it != movePairs.end(); it++) {
            if (it->first == src && it->second == dst) {
                movePairs.erase(it);
                break;
            }
        }
        return true;
    }
    return false;
}

// 溢出: 选择高干扰结点, 移动到简化栈
bool Coloring::spill()
{
    int max_k = 0;
    int max_node = -1;
    for (auto it = graph.begin(); it != graph.end(); it++) {
        int node = it->first;
        if (isMachineReg(node))
            continue;
        int k = getNeighbors(node).size();
        if (k > max_k) {
            max_k = k;
            max_node = node;
        }
    }

    if (max_node != -1) {
        simplifiedNodes.push(max_node);
        eraseNode(max_node);
        return true;
    }

    return false;
}

// 选择: 从简化栈中逐个弹出结点
// 如果颜色不够分配, 则将该结点标记到溢出结点集
bool Coloring::select()
{
    graph = ig->graph;
    movePairs = ig->movePairs;

    // 优先给机器寄存器上色
    for (int node = 0; node < 100; node++) {
        if (graph.find(node) != graph.end()) {
            colors[node] = node;
            if (coalescedMoves.find(node) != coalescedMoves.end()) {
                auto coalesced_nodes = coalescedMoves[node];
                for (auto it = coalesced_nodes.begin(); it != coalesced_nodes.end(); it++)
                    colors[*it] = node;
            }
        }
    }

    while (!simplifiedNodes.empty()) {
        int node = simplifiedNodes.top();
        simplifiedNodes.pop();

        // 跳过机器寄存器
        if (isMachineReg(node))
            continue;

        // cout << "选择: " << node << endl;

        set<int> neighbors = getNeighbors(node);
        for (auto it = coalescedMoves[node].begin(); it != coalescedMoves[node].end(); it++) {
            auto coalesced_neighbors = getNeighbors(*it);
            neighbors.insert(coalesced_neighbors.begin(), coalesced_neighbors.end());
        }

        set<int> usedColors;

        // 如果邻居结点已着色, 则将其颜色加入已使用颜色集合
        for (auto it = neighbors.begin(); it != neighbors.end(); it++) {
            if (isMachineReg(*it))
                colors[*it] = *it;
            if (colors.find(*it) != colors.end())
                usedColors.insert(colors[*it]);
        }

        // 遍历寻找未使用的颜色, 给当前结点着色
        for (int i = 0; i < k; i++)
            if (usedColors.find(i) == usedColors.end()) {
                colors[node] = i;
                if (coalescedMoves.find(node) != coalescedMoves.end()) {
                    auto coalesced_nodes = coalescedMoves[node];
                    for (auto it = coalesced_nodes.begin(); it != coalesced_nodes.end(); it++)
                        colors[*it] = i;
                }
                break;
            }

        // 如果无可用颜色, 则将该结点标记为溢出结点
        if (colors.find(node) == colors.end()) {
            spilled.insert(node);
            if (coalescedMoves.find(node) != coalescedMoves.end()) {
                auto coalesced_nodes = coalescedMoves[node];
                spilled.insert(coalesced_nodes.begin(), coalesced_nodes.end());
            }
        }
    }

    return checkColoring();
}
