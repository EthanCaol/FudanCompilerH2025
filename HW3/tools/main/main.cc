#include "ASTheader.hh"
#include "FDMJAST.hh"
#include "ast2xml.hh"
#include "xml2ast.hh"
#include <cstring>
#include <fstream>
#include <iostream>
#include <string>
#include <filesystem>
#include <unistd.h>

using namespace std;
using namespace fdmj;
using namespace tinyxml2;

#define with_location_info true

fdmj::Program* prog();

int main(int argc, const char* argv[])
{
    // 切换到test目录
    filesystem::path filePath(__FILE__);
    filesystem::path directory = filePath.parent_path();
    chdir(directory.c_str());
    chdir("../../test");

    string file;
    // file = "test2";
    file = argv[argc - 1];

    string file_fmj = file + ".fmj";
    string file_ast = file + ".2.xml";
    string file_ast_semant = file + ".2-semant.xml";

    ifstream fmjfile(file_fmj);
    fdmj::Program* root = fdmjParser(fmjfile, false);
    if (root == nullptr) {
        cout << "AST无效" << endl;
        return EXIT_FAILURE;
    }

    cout << "将AST转换为XML文件: "<< file_ast << endl;
    XMLDocument* x = ast2xml(root, nullptr, with_location_info, false); // no semant info yet
    x->SaveFile(file_ast.c_str());
    if (x->Error()) {
        cout << "AST无效" << endl;
        return EXIT_FAILURE;
    }

    delete root;
    cout << "从XML文件读取AST: " << file_ast << endl;
    x->LoadFile(file_ast.c_str());
    root = xml2ast(x->FirstChildElement());
    if (root == nullptr) {
        cout <<  "AST无效" << endl;
        return EXIT_FAILURE;
    }

    cout << "将AST转换为XML文件: " << file_ast_semant << endl;
    AST_Semant_Map* semant_map = semant_analyze(root);
    x = ast2xml(root, semant_map, with_location_info, true); // no semant info yet

    if (x->Error()) {
        cout << "AST无效" << endl;
        return EXIT_FAILURE;
    }

    x->SaveFile(file_ast_semant.c_str());
    return EXIT_SUCCESS;
}