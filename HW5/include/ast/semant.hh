#ifndef __AST_SEMANT_HH__
#define __AST_SEMANT_HH__

#include <map>
#include <set>
#include "ASTheader.hh"
#include "FDMJAST.hh"
#include "namemaps.hh"

using namespace std;
using namespace fdmj;

class AST_Semant {
public:
    // Value: 值 typeKind lvalue
    // MethodName: 方法名称
    // ClassName: 类名
    enum Kind { Value, MethodName, ClassName };

private:
    Kind s_kind;                              // 结点类型
    TypeKind typeKind;                        // 值类型 {CLASS=0, INT=1, ARRAY=2}
    variant<monostate, string, int> type_par; // 类名<string> | 数组元数<int>
    bool lvalue;                              // 是否为左值
public:
    AST_Semant(AST_Semant::Kind s_kind, TypeKind typeKind, variant<monostate, string, int> type_par, bool lvalue)
        : s_kind(s_kind)
        , typeKind(typeKind)
        , type_par(type_par)
        , lvalue(lvalue) { };
    Kind get_kind() { return s_kind; }
    TypeKind get_type() { return typeKind; }
    variant<monostate, string, int> get_type_par() { return type_par; }
    bool is_lvalue() { return lvalue; }
    static string s_kind_string(Kind s_kind)
    {
        switch (s_kind) {
            case AST_Semant::Kind::Value:
                return "Value";
            case AST_Semant::Kind::MethodName:
                return "MethodName";
            case AST_Semant::Kind::ClassName:
                return "ClassName";
            default:
                return "Unknown";
        }
    }
};

// 结点语义信息
class AST_Semant_Map {
private:
    Name_Maps* name_maps;
    map<AST*, AST_Semant*> semant_map;

public:
    AST_Semant_Map() { semant_map = map<AST*, AST_Semant*>(); }
    ~AST_Semant_Map() { semant_map.clear(); }

    void setNameMaps(Name_Maps* nm) { name_maps = nm; }
    Name_Maps* getNameMaps() { return name_maps; }

    AST_Semant* getSemant(AST* node)
    {
        if (node == nullptr)
            return nullptr;
        if (semant_map.find(node) == semant_map.end())
            return nullptr;
        return semant_map[node];
    }
    
    void setSemant(AST* node, AST_Semant* semant)
    {
        if (node == nullptr) {
            cerr << "Error: setting semantic information for a null node" << endl;
            return;
        }
        semant_map[node] = semant;
    }
};

#endif