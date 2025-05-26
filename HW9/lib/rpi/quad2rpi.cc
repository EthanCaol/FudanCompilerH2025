#define DEBUG
#undef DEBUG

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include "quad.hh"
#include "flowinfo.hh"
#include "color.hh"
#include "quad2rpi.hh"

// 当前函数名
static string current_funcname = "";

// 将函数名转换为汇编器可接受的格式
string normalizeName(string name)
{
    // 特殊情况处理: 主函数名称转换
    if (name == "_^main^_^main") return "main";

    // 将特殊字符替换为美元符号
    for (char& c : name)
        if (!isalnum(c)) c = '$';
    return name;
}

bool rpi_isMachineReg(int n) { return (n >= 0 && n <= 15); }

string term2str(QuadTerm* term, Color* color)
{
    string result;
    if (term->kind == QuadTermKind::TEMP) {
        Temp* t = term->get_temp()->temp;
        result = "r" + to_string(color->color_of(t->num));
    }

    else if (term->kind == QuadTermKind::CONST) {
        result = "#" + to_string(term->get_const());
    }

    else if (term->kind == QuadTermKind::MAME) {
        result = "=" + normalizeName(term->get_name());
    }

    return result;
}

int current_label = 0; // DEBUG: 调试用
string label2str(Label* label) { return current_funcname + "$" + label->str(); }
string convert(QuadLabel* label, Color* c, int indent)
{
    current_label = label->label->num;
    return current_funcname + "$" + label->label->str() + ": \n";
}

// TODO: 遍历函数中的四元式, 逐条语句进行转换
// (有时可以将两条语句合并为一条指令, 尝试这样做！)
// 1) 在函数开头正确设置栈帧结构, 并处理返回逻辑
// 2) 使用颜色映射表(color map)将临时变量替换为实际寄存器
// 3) 妥善处理溢出的临时变量(spilled temps)
// 4) 以及其他实现细节
string convert(QuadFuncDecl* func, DataFlowInfo* dfi, Color* color, int indent)
{
    current_funcname = normalizeName(func->funcname);

    string result;
    result.reserve(10000);
    result += "@ Here's function: " + func->funcname + "\n\n";
    result += ".balign 4\n";
    result += ".global " + current_funcname + "\n";
    result += ".section .text\n\n";
    result += current_funcname + ":\n";
    result += string(indent, ' ') + "push {r4-r10, fp, lr}\n";
    result += string(indent, ' ') + "add fp, sp, #32\n";

    // 处理每个基本块
    for (auto bk : *func->quadblocklist) {
        auto ql = bk->quadlist;

        // 遍历块中的每条语句
        for (auto stmIt = ql->begin(); stmIt != ql->end(); stmIt++) {
            QuadStm* stm = *stmIt;
            QuadStm* stmNext = (stmIt + 1 != ql->end()) ? *(stmIt + 1) : nullptr;
            QuadLabel* labelNext = (stmNext && stmNext->kind == QuadKind::LABEL) ? static_cast<QuadLabel*>(stmNext) : nullptr;
            QuadLoad* loadNext = (stmNext && stmNext->kind == QuadKind::LOAD) ? static_cast<QuadLoad*>(stmNext) : nullptr;
            QuadStore* storeNext = (stmNext && stmNext->kind == QuadKind::STORE) ? static_cast<QuadStore*>(stmNext) : nullptr;

            // Label
            if (stm->kind == QuadKind::LABEL) result += convert(static_cast<QuadLabel*>(stm), color, indent);

            // Move
            else if (stm->kind == QuadKind::MOVE) {
                auto moveStm = static_cast<QuadMove*>(stm);
                auto dstReg = color->color_of(moveStm->dst->temp->num);

                // 跳过同寄存器的移动
                if (moveStm->src->kind == QuadTermKind::TEMP) {
                    auto srcReg = color->color_of(moveStm->src->get_temp()->temp->num);
                    if (dstReg == srcReg) continue;
                }

                result += string(indent, ' ') + "mov ";
                result += "r" + to_string(dstReg) + ", ";
                result += term2str(moveStm->src, color) + "\n";
            }

            // MoveBinop
            else if (stm->kind == QuadKind::MOVE_BINOP) {
                auto moveBinopStm = static_cast<QuadMoveBinop*>(stm);
                auto dstReg = color->color_of(moveBinopStm->dst->temp->num);
                auto left = moveBinopStm->left;
                auto right = moveBinopStm->right;
                auto binop = moveBinopStm->binop;

                if (binop == "+") {
                    // 如果右操作数是常数, 则可以尝试合并
                    if (right->kind == QuadTermKind::CONST) {
                        // 加法结果 只会 用于下一条加载指令, 则可以合并
                        if (loadNext && loadNext->src->kind == QuadTermKind::TEMP
                            && loadNext->src->get_temp()->temp->num == moveBinopStm->dst->temp->num) {
                            // 确保仅仅用于下一条指令
                            set<int>& nextLiveout = dfi->liveout->at(stmNext);
                            if (nextLiveout.find(moveBinopStm->dst->temp->num) == nextLiveout.end()) {
                                result += string(indent, ' ') + "ldr ";
                                result += "r" + to_string(color->color_of(loadNext->dst->temp->num)) + ", ";
                                result += "[" + term2str(left, color) + ", " + term2str(right, color) + "]\n";
                                stmIt++;
                                continue;
                            }
                        }

                        // 加法结果 只会 用于下一条存储指令, 则可以合并
                        if (storeNext && storeNext->dst->kind == QuadTermKind::TEMP
                            && storeNext->dst->get_temp()->temp->num == moveBinopStm->dst->temp->num) {
                            // 确保仅仅用于下一条指令
                            set<int>& nextLiveout = dfi->liveout->at(stmNext);
                            if (nextLiveout.find(moveBinopStm->dst->temp->num) == nextLiveout.end()) {
                                result += string(indent, ' ') + "str ";
                                result += term2str(storeNext->src, color) + ", [";
                                result += term2str(left, color) + ", " + term2str(right, color) + "]\n";
                                stmIt++;
                                continue;
                            }
                        }
                    }

                    result += string(indent, ' ') + "add ";
                } else if (binop == "-")
                    result += string(indent, ' ') + "sub ";
                else if (binop == "*")
                    result += string(indent, ' ') + "mul ";

                result += "r" + to_string(dstReg) + ", ";
                result += term2str(moveBinopStm->left, color) + ", ";
                result += term2str(moveBinopStm->right, color) + "\n";
            }

            // Load
            else if (stm->kind == QuadKind::LOAD) {
                auto loadStm = static_cast<QuadLoad*>(stm);
                result += string(indent, ' ') + "ldr ";
                result += "r" + to_string(color->color_of(loadStm->dst->temp->num)) + ", ";
                if (loadStm->src->kind == QuadTermKind::TEMP)
                    result += "[" + term2str(loadStm->src, color) + "]\n";
                else
                    result += term2str(loadStm->src, color) + "\n";
            }

            // Store
            else if (stm->kind == QuadKind::STORE) {
                auto storeStm = static_cast<QuadStore*>(stm);
                result += string(indent, ' ') + "str ";
                result += term2str(storeStm->src, color) + ", [";
                result += term2str(storeStm->dst, color) + "]\n";
            }

            // Call
            else if (stm->kind == QuadKind::CALL) {
                auto callStm = static_cast<QuadCall*>(stm);
                result += string(indent, ' ') + "blx " + term2str(callStm->obj_term, color) + "\n";
            }

            // MoveCall
            else if (stm->kind == QuadKind::MOVE_CALL) {
                auto moveCallStm = static_cast<QuadMoveCall*>(stm);
                result += string(indent, ' ') + "blx " + term2str(moveCallStm->call->obj_term, color) + "\n";
            }

            // ExtCall
            else if (stm->kind == QuadKind::EXTCALL) {
                auto extCallStm = static_cast<QuadExtCall*>(stm);
                result += string(indent, ' ') + "bl " + extCallStm->extfun + "\n";
            }

            // MoveExtCall
            else if (stm->kind == QuadKind::MOVE_EXTCALL) {
                auto moveExtCallStm = static_cast<QuadMoveExtCall*>(stm);
                result += string(indent, ' ') + "bl " + moveExtCallStm->extcall->extfun + "\n";
            }

            // Jump
            else if (stm->kind == QuadKind::JUMP) {
                auto jumpStm = static_cast<QuadJump*>(stm);
                if (labelNext && labelNext->label->num == jumpStm->label->num) continue; // 相同跳转
                result += string(indent, ' ') + "b " + label2str(jumpStm->label) + "\n";
            }

            // CJump
            else if (stm->kind == QuadKind::CJUMP) {
                auto cJumpStm = static_cast<QuadCJump*>(stm);
                auto relop = cJumpStm->relop;

                result += string(indent, ' ') + "cmp ";
                result += term2str(cJumpStm->left, color) + ", ";
                result += term2str(cJumpStm->right, color) + "\n";
                result += string(indent, ' ');

                if (relop == "==")
                    result += "beq ";
                else if (relop == "!=")
                    result += "bne ";
                else if (relop == "<")
                    result += "blt ";
                else if (relop == "<=")
                    result += "ble ";
                else if (relop == ">")
                    result += "bgt ";
                else if (relop == ">=")
                    result += "bge ";

                result += label2str(cJumpStm->t) + "\n";
                if (labelNext && labelNext->label->num == cJumpStm->f->num) continue; // 相同跳转
                result += string(indent, ' ') + "b " + label2str(cJumpStm->f) + "\n";
            }

            // Retuen
            else if (stm->kind == QuadKind::RETURN) {
                result += string(indent, ' ') + "sub sp, fp, #32\n";
                result += string(indent, ' ') + "pop {r4-r10, fp, pc}\n";
            }
        }
    }

    return result;
}

string quad2rpi(QuadProgram* quadProgram, ColorMap* cm)
{
    string result;
    result.reserve(10000);
    result = ".section .note.GNU-stack\n\n@ Here is the RPI code\n\n";

    // 遍历处理所有Quad函数
    for (QuadFuncDecl* func : *quadProgram->quadFuncDeclList) {
        // 计算数据流信息
        DataFlowInfo* dfi = new DataFlowInfo(func);
        dfi->computeLiveness();
        // dfi->printLiveness();

        trace(func);
        Color* c = cm->color_map[func->funcname];

        result += convert(func, dfi, c, 9) + "\n";
    }

    result += ".global malloc\n";
    result += ".global getint\n";
    result += ".global putint\n";
    result += ".global putch\n";
    result += ".global putarray\n";
    result += ".global getch\n";
    result += ".global getarray\n";
    result += ".global starttime\n";
    result += ".global stoptime\n";
    return result;
}

// 打印RPI代码到文件
void quad2rpi(QuadProgram* quadProgram, ColorMap* cm, string filename)
{
    ofstream outfile(filename);
    if (outfile.is_open()) {
        outfile << quad2rpi(quadProgram, cm);
        outfile.flush();
        outfile.close();
    } else {
        cerr << "Error: Unable to open file " << filename << endl;
    }
}