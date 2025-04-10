#ifndef _TEMP
#define _TEMP

#include <map>
#include <string>
#include <cstdio>

using namespace std;

namespace tree {

// 临时变量
class Temp {
public:
    int num;
    Temp(int num)
        : num(num) { };
    int name() { return num; }
    string str() { return "t" + to_string(num); }
    bool operator==(const Temp& t) const { return num == t.num; } // 直接按照编号进行比较
};

// 标签
class Label {
public:
    int num;
    Label(int num)
        : num(num) { };
    int name() { return num; }
    string str() { return "L" + to_string(num); }
    bool operator==(const Label& l) const { return num == l.num; } // 直接按照编号进行比较
};

// 记录映射
class Temp_map {
public:
    map<int, bool> t_map; // 临时变量映射
    map<int, bool> l_map; // 标签映射

    int next_temp;  // 下一个临时变量
    int next_label; // 下一个标签

    Temp_map()
    {
        next_temp = 100;
        next_label = 100;
        t_map.clear();
        l_map.clear();
    }

    // 创建新的临时变量
    Temp* newtemp()
    {
        while (t_map[next_temp])
            next_temp++;
        t_map[next_temp] = true;
        return new Temp(next_temp++);
    }

    // 创建新的标签
    Label* newlabel()
    {
        while (l_map[next_label])
            next_label++;
        l_map[next_label] = true;
        return new Label(next_label++);
    }

    // 创建特定编号的临时变量
    Temp* named_temp(int num)
    {
        if (!t_map[num])
            t_map[num] = true;
        return new Temp(num);
    }

    // 创建特定编号的标签
    Label* named_label(int num)
    {
        if (!l_map[num])
            l_map[num] = true;
        return new Label(num);
    }

    bool in_tempmap(int num) { return t_map[num]; }
    bool in_labelmap(int num) { return l_map[num]; }
};

} // namespace tree

#endif
