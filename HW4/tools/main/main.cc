#include <iostream>
#include <fstream>
#include <cstring>
#include <string>
#include <filesystem>
#include <unistd.h>

#include "config.hh"
#include "ASTheader.hh"
#include "FDMJAST.hh"
#include "xml2ast.hh"
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

int main(int argc, const char* argv[])
{
    // 切换到test目录
    filesystem::path filePath(__FILE__);
    filesystem::path directory = filePath.parent_path();
    chdir(directory.c_str());
    chdir("../../test");

    string file;
    file = "hw4test02";
    // file = argv[argc - 1];

    string file_ast = file + ".2-semant.ast";
    string file_irp = file + ".3.irp.my";

    cout << "从XML文件读取AST: " << file_ast << endl;
    AST_Semant_Map* semant_map = new AST_Semant_Map();
    fdmj::Program* root = xml2ast(file_ast, &semant_map);
    if (root == nullptr) {
        cout << "AST无效" << endl;
        return EXIT_FAILURE;
    }
    semant_map->getNameMaps()->print();
    

    cout << "将AST转换为IR" << endl;
    tree::Program* ir = ast2tree(root, semant_map);

    cout << "将IR保存为XML文件: " << file_irp << endl;
    XMLDocument* x = tree2xml(ir);

    x->SaveFile(file_irp.c_str());
    return EXIT_SUCCESS;
}
