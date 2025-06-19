#include <iostream>
#include <fstream>
#include <cstring>
#include <string>
#include <filesystem>
#include <unistd.h>

#include "config.hh"

#include "FDMJAST.hh"
#include "ast2xml.hh"
#include "xml2ast.hh"

#include "temp.hh"
#include "treep.hh"
#include "xml2tree.hh"
#include "tree2xml.hh"

#include "quad.hh"
#include "xml2quad.hh"
#include "quad2xml.hh"

// .fmj -> .2.ast
#include "ASTheader.hh"

// .2.ast -> .2-semant.ast
#include "namemaps.hh"
#include "semant.hh"

// 2-semant.ast -> .3.irp
#include "ast2tree.hh"

// .3.irp -> .3-canon.irp
#include "canon.hh"

// .3-canon.irp -> .4.quad
#include "tree2quad.hh"

// .4.quad -> .4-block.quad -> .4-ssa.quad
#include "blocking.hh"
#include "quadssa.hh"

// .4-ssa.quad -> .4-ssa-opt.quad
#include "opt.hh"

// .4-ssa-opt.quad -> .4-ssa-prepared (.clr)
#include "prepareregalloc.hh"
#include "regalloc.hh"

// .4-ssa-prepared (.clr) -> .s
#include "color.hh"
#include "quad2rpi.hh"

using namespace std;
using namespace fdmj;
using namespace tree;
using namespace tinyxml2;
namespace fs = std::filesystem;

#define with_location_info false
XMLDocument* x;

void exec(string file)
{
    // 寄存器数量
    int number_of_colors = 9; // default 9: r0-r8
    cout << "颜色数: " << number_of_colors << endl;

    string file_fmj = file + ".fmj";
    string file_ast = file + ".2.ast.my";

    string file_irp = file + ".3.irp.my";
    string file_irp_canon = file + ".3.irp-canon.my";

    string file_quad = file + ".4.quad.my";
    string file_quad_xml = file + ".4-xml.quad.my";
    string file_quad_block = file + ".4-block.quad.my";
    string file_quad_ssa = file + ".4-ssa.quad.my";

    string file_quad_ssa_opt = file + ".5-ssa-opt.quad.my";

    string file_quad_prepared = file + ".6-ssa-prepared.my";
    string file_quad_color_xml = file + ".6-ssa-prepared.clr.my";

    string file_rpi = file + ".s";

    cout << "读取: " << file_fmj << endl;
    ifstream fmjfile(file_fmj);
    fdmj::Program* root = fdmj::fdmjParser(fmjfile, false);

    cout << "写入: " << file_ast << endl;
    AST_Semant_Map* semant_map = semant_analyze(root);
    x = ast2xml(root, semant_map, with_location_info, true);
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
    quad2file(qd, file_quad.c_str(), true);

    // ----------------------------------------------------------------

    // 需要重新写入并读取quad (摆脱Temp的指针一致性 用于重命名)
    cout << "写入: " << file_quad_xml << endl;
    quad2xml(qd, file_quad_xml.c_str());

    cout << "读取: " << file_quad_xml << endl;
    quad::QuadProgram* x3 = xml2quad(file_quad_xml.c_str());

    // ----------------------------------------------------------------

    cout << "写入: " << file_quad_block << endl;
    QuadProgram* x4 = blocking(x3);
    quad2file(x4, file_quad_block.c_str(), true);

    cout << "写入: " << file_quad_ssa << endl;
    QuadProgram* x5 = quad2ssa(x4);
    quad2file(x5, file_quad_ssa.c_str(), true);

    // ----------------------------------------------------------------

    cout << "写入: " << file_quad_ssa_opt << endl;
    QuadProgram* x6 = optProg(x5);
    quad2file(x6, file_quad_ssa_opt.c_str(), true);

    // ----------------------------------------------------------------

    cout << "写入: " << file_quad_prepared << endl;
    QuadProgram* x7 = prepareRegAlloc(x5);
    quad2file(x7, file_quad_prepared.c_str(), true);

    cout << "着色: " << file_quad_color_xml << endl;
    XMLDocument* x8 = coloring(x7, number_of_colors, false);
    x8->SaveFile(file_quad_color_xml.c_str());

    // ----------------------------------------------------------------

    cout << "读取: " << file_quad_color_xml << endl;
    ColorMap* colormap = xml2colormap(file_quad_color_xml);

    cout << "写入: " << file_rpi << endl;
    quad2rpi(x7, colormap, file_rpi);

    cout << "-----Done---" << endl << endl;
}

int main(int argc, const char* argv[])
{
    // 切换到test目录
    // filesystem::path filePath(__FILE__);
    // filesystem::path directory = filePath.parent_path();
    // chdir(directory.c_str());
    // chdir("../../test");

    vector<string> files;

    // 处理命令行单文件
    string filename = argv[1];
    assert(filename.size() >= 4 && filename.substr(filename.size() - 4) == ".fmj");
    size_t dotPos = filename.find_last_of('.');
    if (dotPos != std::string::npos) {
        files.push_back(filename.substr(0, dotPos));
    }

    // 遍历当前目录下的所有文件
    // for (const auto& entry : fs::directory_iterator(".")) {
    //     if (entry.is_regular_file()) {
    //         const std::string filename = entry.path().filename().string();

    //         // 检查是否以 .fmj 结尾
    //         if (filename.size() >= 4 && filename.substr(filename.size() - 4) == ".fmj") {
    //             size_t dotPos = filename.find_last_of('.');
    //             if (dotPos != std::string::npos) {
    //                 files.push_back(filename.substr(0, dotPos));
    //             }
    //         }
    //     }
    // }

    for (auto file : files) {
        cout << file << endl;
        exec(file);
    }
}
