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

#include "xml2tree.hh"
#include "tree2xml.hh"

#include "config.hh"
#include "temp.hh"
#include "treep.hh"

#include "canon.hh"
#include "quad.hh"

#include "namemaps.hh"
#include "semant.hh"

#include "ast2tree.hh"
#include "tree2quad.hh"

using namespace std;
using namespace fdmj;
using namespace tree;
using namespace tinyxml2;

#define with_location_info false
XMLDocument* x;

int main(int argc, const char* argv[])
{
    // 切换到test目录
    filesystem::path filePath(__FILE__);
    filesystem::path directory = filePath.parent_path();
    chdir(directory.c_str());
    chdir("../../test");

    string file;
    string dir = "HW5/";
    file = dir + "test0";
    // file = dir + argv[argc - 1];

    string file_fmj = file + ".fmj";
    string file_ast = file + ".2.ast.my";
    string file_irp = file + ".3.irp.my";
    string file_irp_canon = file + ".3.irp-canon.my";
    string file_quad = file + ".4.quad.my";

    cout << "读取: " << file_fmj << endl;
    ifstream fmjfile(file_fmj);
    fdmj::Program* root = fdmj::fdmjParser(fmjfile, false);
    if (root == nullptr) {
        cout << "AST无效" << endl;
        return EXIT_FAILURE;
    }

    cout << "写入: " << file_ast << endl;
    AST_Semant_Map* semant_map = semant_analyze(root);
    // semant_map->getNameMaps()->print();
    x = ast2xml(root, semant_map, with_location_info, true);
    if (x->Error()) {
        cout << "AST无效" << endl;
        return EXIT_FAILURE;
    }
    x->SaveFile(file_ast.c_str());

    cout << "写入: " << file_irp << endl;
    tree::Program* ir = ast2tree(root, semant_map);
    x = tree2xml(ir);
    x->SaveFile(file_irp.c_str());

    // ----------------------------------------------------------------

    cout << "写入: " << file_irp_canon << endl;
    tree::Program* ir_canon = canon(ir);
    XMLDocument* doc = tree2xml(ir_canon);
    doc->SaveFile(file_irp_canon.c_str());

    cout << "写入: " << file_quad << endl;
    QuadProgram* qd = tree2quad(ir_canon);
    if (qd == nullptr) {
        cerr << "Error converting IR to Quad" << endl;
        return EXIT_FAILURE;
    }

    string temp_str;
    temp_str.reserve(50000);
    qd->print(temp_str, 0, true);
    ofstream qo(file_quad);
    if (!qo) {
        cerr << "Error opening file: " << file_quad << endl;
        return EXIT_FAILURE;
    }
    qo << temp_str;
    qo.flush();
    qo.close();

    ofstream out("/dev/tty");
    out << "-----Done---" << endl;

    return EXIT_SUCCESS;
}
