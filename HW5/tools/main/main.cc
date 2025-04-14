#include <iostream>
#include <fstream>
#include <cstring>
#include <string>
#include <filesystem>
#include <unistd.h>

#include "ASTheader.hh"
#include "FDMJAST.hh"
#include "ast2xml.hh"
#include "xml2ast.hh"

#include "config.hh"
#include "temp.hh"
#include "treep.hh"
#include "namemaps.hh"
#include "semant.hh"

#include "ast2tree.hh"
#include "tree2xml.hh"

using namespace std;
using namespace fdmj;
using namespace tree;
using namespace tinyxml2;

#define with_location_info false

int main(int argc, const char* argv[])
{
    // 切换到test目录
    filesystem::path filePath(__FILE__);
    filesystem::path directory = filePath.parent_path();
    chdir(directory.c_str());
    chdir("../../test");

    string file;
    string dir = "HW3/";
    // file = dir + "bubblesort";
    file = dir + argv[argc - 1];

    string file_fmj = file + ".fmj";
    string file_ast = file + ".2.ast.my";
    string file_ast_semant = file + ".2-semant.ast.my";
    string file_irp = file + ".3.irp.my";

    cout << "\n从FMJ文件读取并解析AST: " << file_fmj << endl;
    ifstream fmjfile(file_fmj);
    fdmj::Program* root = fdmj::fdmjParser(fmjfile, false);
    if (root == nullptr) {
        cout << "AST无效" << endl;
        return EXIT_FAILURE;
    }

    cout << "将AST保存为XML文件: " << file_ast << endl;
    XMLDocument* x = ast2xml(root, nullptr, with_location_info, false); // no semant info yet
    x->SaveFile(file_ast.c_str());
    if (x->Error()) {
        cout << "AST无效" << endl;
        return EXIT_FAILURE;
    }

    cout << "将AST进行语义分析" << endl;
    AST_Semant_Map* semant_map = semant_analyze(root);
    x = ast2xml(root, semant_map, with_location_info, true); // no semant info yet
    if (x->Error()) {
        cout << "AST无效" << endl;
        return EXIT_FAILURE;
    }
    x->SaveFile(file_ast_semant.c_str());

    // cout << "从XML文件读取AST: " << file_ast << endl;
    // AST_Semant_Map* semant_map = new AST_Semant_Map();
    // fdmj::Program* root = xml2ast(file_ast, &semant_map);
    // if (root == nullptr) {
    //     cout << "AST无效" << endl;
    //     return EXIT_FAILURE;
    // }
    // semant_map->getNameMaps()->print();

    // cout << "将AST转换为IR" << endl;
    // tree::Program* ir = ast2tree(root, semant_map);

    // cout << "将IR保存为XML文件: " << file_irp << endl;
    // XMLDocument* x = tree2xml(ir);

    // x->SaveFile(file_irp.c_str());
    return EXIT_SUCCESS;
}
