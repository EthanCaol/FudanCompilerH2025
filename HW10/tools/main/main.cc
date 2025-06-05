#include <iostream>
#include <fstream>
#include <cstring>
#include <string>
#include <filesystem>
#include <unistd.h>

#include "treep.hh"
#include "quad.hh"
#include "quad2xml.hh"
#include "xml2quad.hh"
#include "opt.hh"

using namespace std;
using namespace tree;
using namespace quad;
using namespace tinyxml2;

int main(int argc, const char* argv[])
{
    // 切换到test目录
    filesystem::path filePath(__FILE__);
    filesystem::path directory = filePath.parent_path();
    chdir(directory.c_str());
    chdir("../../test/input_example");

    string file;
    file = argv[argc - 1];
    file = "hw8/hw8test12";
    // file = "bubblesort";
    // file = "fibonacci";

    string file_quad_ssa_xml = file + ".4-ssa-xml.quad";
    string file_quad_ssa = file + ".4-ssa.txt";
    string file_quad_ssa_opt = file + ".4-zopt.txt";

    cout << "读取quad: " << file_quad_ssa_xml << endl;
    quad::QuadProgram* x3 = xml2quad(file_quad_ssa_xml.c_str());
    if (x3 == nullptr) {
        cerr << "Error reading Quad from xml: " << file_quad_ssa_xml << endl;
        return EXIT_FAILURE;
    }

    quad2file(x3, file_quad_ssa, true);
    QuadProgram* x4 = optProg(x3);

    quad2file(x4, file_quad_ssa_opt.c_str(), true);

    cout << "-----Done---" << endl;
    return EXIT_SUCCESS;
}