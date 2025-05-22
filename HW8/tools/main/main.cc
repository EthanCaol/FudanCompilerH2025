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
#include "prepareregalloc.hh"
#include "regalloc.hh"

using namespace std;
using namespace tree;
using namespace quad;
using namespace tinyxml2;

int main(int argc, const char* argv[])
{
    // 切换到test目录
    // filesystem::path filePath(__FILE__);
    // filesystem::path directory = filePath.parent_path();
    // chdir(directory.c_str());
    // chdir("../../test");

    string file;
    file = argv[argc - 1];
    // string dir = "input_example/";
    // file = dir + "hw8test04";

    // 寄存器数量
    int number_of_colors = 9; // default 9: r0-r8
    cout << "颜色数: " << number_of_colors << endl;

    string file_quad_ssa_xml = file + ".4-ssa-xml.quad";
    string file_quad_ssa = file + ".4-ssa-back.txt";
    string file_quad_prepared = file + ".4-ssa-prepared.txt";
    string file_quad_color_xml = file + ".4-ssa-xml.clr";

    cout << "读取Quad(XML): " << file_quad_ssa_xml << endl;
    quad::QuadProgram* x3 = xml2quad(file_quad_ssa_xml.c_str());
    if (x3 == nullptr) {
        cerr << "Error reading Quad from xml: " << file_quad_ssa_xml << endl;
        return EXIT_FAILURE;
    }

    cout << "保存Quad(txt): " << file_quad_ssa << endl;
    quad2file(x3, file_quad_ssa, true);

    cout << "预处理Quad(txt): " << file_quad_prepared << endl;
    QuadProgram* x4 = prepareRegAlloc(x3);
    quad2file(x4, file_quad_prepared.c_str(), true);

    cout << "着色: " << file_quad_color_xml << endl;
    XMLDocument* x5 = coloring(x4, number_of_colors, false);
    x5->SaveFile(file_quad_color_xml.c_str());
    cout << "-----Done---" << endl;
    return EXIT_SUCCESS;
}