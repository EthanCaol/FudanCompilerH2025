#include "ASTheader.hh"
#include "FDMJAST.hh"

#include "MinusIntConverter.hh"
#include "constantPropagation.hh"
#include "executor.hh"

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

// XML是否记录位置信息
#define with_location_info false

Program* prog();

int main(int argc, const char* argv[])
{
    // 切换到test目录
    filesystem::path filePath(__FILE__);
    filesystem::path directory = filePath.parent_path();
    chdir(directory.c_str());
    chdir("../../test");

    string file;
    file = "test5";

    // const bool debug = argc > 1 && std::strcmp(argv[1], "--debug") == 0;

    // if ((!debug && argc != 2) || (debug && argc != 3)) {
    //   cerr << "Usage: " << argv[0] << " [--debug] filename" << endl;
    //   return EXIT_FAILURE;
    // }
    // file = argv[argc - 1];

    // boilerplate output filenames (used throutghout the compiler pipeline)
    string file_fmj = file + ".fmj";
    string file_out = file + ".out";
    string file_src = file + ".src";
    string file_src2 = file + ".1.debug.src";
    string file_ast1 = file + ".2-debug1.xml";
    string file_ast2_clone = file + ".2-debug2_clone.xml";
    string file_ast3_minus = file + ".2-debug3_minus.xml";
    string file_ast4_constant = file + ".2-debug4_constant.xml";
    string file_irp = file + ".3.irp";
    string file_stm = file + ".4.stm";
    string file_liv = file + ".5.liv";

    cout << "Parsing fmj source file: " << file_fmj << endl;
    std::ifstream fmjfile(file_fmj);
    Program* root = fdmjParser(fmjfile, false); // false means no debug info from parser
    if (root == nullptr) {
        std::cout << "AST is not valid!" << endl;
        return EXIT_FAILURE;
    }

    cout << "Convert AST  to XML..." << endl;
    XMLDocument* x = ast2xml(root, with_location_info);
    delete root;
    if (x->Error()) {
        std::cout << "AST is not valid!" << endl;
        return EXIT_FAILURE;
    }

    cout << "Saving AST (XML) to: " << file_ast1 << endl;
    x->SaveFile(file_ast1.c_str());

    // ---------------- clone ----------------

    cout << "Loading AST (XML) from: " << file_ast1 << endl;
    x = new XMLDocument();
    x->LoadFile(file_ast1.c_str());
    root = xml2ast(x->RootElement());
    if (root == nullptr) {
        std::cout << "AST is not valid!" << endl;
        return EXIT_FAILURE;
    }

    cout << "Clone AST ..." << endl;
    Program* root_clone = root->clone();
    delete root;

    XMLDocument* y = ast2xml(root_clone, with_location_info);
    delete root_clone;

    cout << "Saving AST (XML) to: " << file_ast2_clone << endl;
    y->SaveFile(file_ast2_clone.c_str());

    // ---------------- minusIntRewrite ----------------

    cout << "Loading AST (XML) from: " << file_ast2_clone << endl;
    XMLDocument* z = new XMLDocument();
    z->LoadFile(file_ast2_clone.c_str());
    root = xml2ast(z->RootElement());

    cout << "minusIntRewrite AST..." << endl;
    Program* root_minus = minusIntRewrite(root);
    delete root;

    cout << "Convert cloned AST to XML..." << endl;
    XMLDocument* w = ast2xml(root_minus, with_location_info);

    cout << "Saving cloned AST (XML) to: " << file_ast3_minus << endl;
    w->SaveFile(file_ast3_minus.c_str());

    // ---------------- constantPropagate ----------------

    cout << "constantPropagate AST..." << endl;
    Program* root_constant = constantPropagate(root_minus);
    delete root_minus;

    cout << "Convert rewrote AST to XML..." << endl;
    w = ast2xml(root_constant, with_location_info);

    cout << "Saving AST (XML) to: " << file_ast4_constant << endl;
    w->SaveFile(file_ast4_constant.c_str());

    // ---------------- execute ----------------

    cout << "execute AST..." << endl;
    int ret = execute(root_constant);
    delete root_constant;

    cout << "Return Value: " << ret << endl;
    cout << "-----Done---" << endl;
    return EXIT_SUCCESS;
}
