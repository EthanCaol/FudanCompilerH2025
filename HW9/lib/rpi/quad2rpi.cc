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

string term2str(QuadTerm* term, Color* color, bool isLeft = true)
{
    string result;
    if (term->kind == QuadTermKind::TEMP) {
        Temp* t = term->get_temp()->temp;
        if (color->is_spill(t->num)) // 如果溢出
            result = "r" + to_string(isLeft ? 9 : 10);
        else
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

string label2str(Label* label) { return current_funcname + "$" + label->str(); }
string convert(QuadLabel* label, Color* c, int indent) { return current_funcname + "$" + label->label->str() + ": \n"; }

// 获取目标寄存器编号
int getDstReg(int dstTempNum, Color* color)
{
    if (color->is_spill(dstTempNum)) return 10; // r10
    return color->color_of(dstTempNum);
}

// 存储目标寄存器到栈中 (如果溢出)
string storeDstReg(int dstTempNum, Color* color, int indent)
{
    if (color->is_spill(dstTempNum))
        return string(indent, ' ') + "str r10, [fp, #" + to_string(-color->spill_offset.at(dstTempNum)) + "]\n";
    return "";
}

// 获取源寄存器编号
int getSrcReg(QuadTerm* srcTerm, Color* color, bool isLeft = true)
{
    if (srcTerm->kind == QuadTermKind::TEMP) {
        auto srcTempNum = srcTerm->get_temp()->temp->num;
        if (color->is_spill(srcTempNum)) return isLeft ? 9 : 10; // r9 or r10
        return color->color_of(srcTempNum);
    }
    return -1;
}

// 加载源寄存器从栈中 (如果溢出)
string loadSrcReg(QuadTerm* srcTerm, Color* color, int indent, bool isLeft = true)
{
    if (srcTerm->kind == QuadTermKind::TEMP) {
        auto srcTempNum = srcTerm->get_temp()->temp->num;
        if (color->is_spill(srcTempNum))
            return string(indent, ' ') + "ldr r" + to_string(isLeft ? 9 : 10) + ", [fp, #"
                + to_string(-color->spill_offset.at(srcTempNum)) + "]\n";
    }
    return "";
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

    if (color->spills.size() > 0)
        result += string(indent, ' ') + "sub sp, sp, #" + to_string(color->spills.size() * 4) + "\n";

    // 处理每个基本块
    for (auto bk : *func->quadblocklist) {
        auto ql = bk->quadlist;

        // 遍历块中的每条语句
        for (auto stmIt = ql->begin(); stmIt != ql->end(); stmIt++) {
            QuadStm* stm = *stmIt;
            QuadStm* stmNext = (stmIt + 1 != ql->end()) ? *(stmIt + 1) : nullptr;
            auto nextLiveout = stmNext ? &dfi->liveout->at(stmNext) : nullptr;

            QuadLabel* labelNext
                = (stmNext && stmNext->kind == QuadKind::LABEL) ? static_cast<QuadLabel*>(stmNext) : nullptr;
            QuadLoad* loadNext
                = (stmNext && stmNext->kind == QuadKind::LOAD) ? static_cast<QuadLoad*>(stmNext) : nullptr;
            QuadStore* storeNext
                = (stmNext && stmNext->kind == QuadKind::STORE) ? static_cast<QuadStore*>(stmNext) : nullptr;

            // Label
            if (stm->kind == QuadKind::LABEL) result += convert(static_cast<QuadLabel*>(stm), color, indent);

            // Move: dst src
            // 出栈: src
            // 入栈: dst
            else if (stm->kind == QuadKind::MOVE) {
                auto moveStm = static_cast<QuadMove*>(stm);
                auto dstTempNum = moveStm->dst->temp->num;
                auto dstReg = getDstReg(dstTempNum, color);
                auto srcReg = getSrcReg(moveStm->src, color);

                // 跳过移动相同寄存器
                if (dstReg == srcReg) continue;

                // 溢出变量出栈
                result += loadSrcReg(moveStm->src, color, indent, true);

                result += string(indent, ' ') + "mov ";
                result += "r" + to_string(dstReg) + ", ";
                result += term2str(moveStm->src, color) + "\n";

                // 溢出变量入栈
                result += storeDstReg(dstTempNum, color, indent);
            }

            // MoveBinop: dst left binop right
            // 出栈: left, right
            // 入栈: dst
            else if (stm->kind == QuadKind::MOVE_BINOP) {
                auto moveBinopStm = static_cast<QuadMoveBinop*>(stm);
                auto dstTempNum = moveBinopStm->dst->temp->num;
                auto dstReg = getDstReg(dstTempNum, color);
                auto left = moveBinopStm->left;
                auto right = moveBinopStm->right;
                auto binop = moveBinopStm->binop;

                // 如果加法的右操作数是常数, 则可以尝试合并
                if (binop == "+" && right->kind == QuadTermKind::CONST) {
                    // LoadBinop: 如果加法结果 仅仅只会 用于下一条加载指令
                    if (loadNext && loadNext->src->kind == QuadTermKind::TEMP
                        && loadNext->src->get_temp()->temp->num == dstTempNum && nextLiveout
                        && nextLiveout->find(dstTempNum) == nextLiveout->end()) {
                        // LoadBinop: dst [left, right]
                        // 入栈: left (right是常数)
                        // 出栈: dst

                        // 将Load的结果 作为新的dst
                        dstTempNum = loadNext->dst->temp->num;
                        dstReg = getDstReg(dstTempNum, color);

                        // 溢出变量出栈
                        result += loadSrcReg(left, color, indent);

                        result += string(indent, ' ') + "ldr ";
                        result += "r" + to_string(dstReg) + ", ";
                        result += "[" + term2str(left, color) + ", " + term2str(right, color) + "]\n";

                        // 溢出变量入栈
                        result += storeDstReg(dstTempNum, color, indent);
                        stmIt++;
                        continue;
                    }

                    // StoreBinop: 如果加法结果 仅仅只会 用于下一条存储指令
                    if (storeNext && storeNext->dst->kind == QuadTermKind::TEMP
                        && storeNext->dst->get_temp()->temp->num == dstTempNum && nextLiveout
                        && nextLiveout->find(dstTempNum) == nextLiveout->end()) {
                        // StoreBinop: src [left, right]
                        // 出栈: src, left (right是常数)

                        // 溢出变量出栈
                        result += loadSrcReg(storeNext->src, color, indent);
                        result += loadSrcReg(left, color, indent, false);

                        result += string(indent, ' ') + "str ";
                        result += term2str(storeNext->src, color, true) + ", [";
                        result += term2str(left, color, false) + ", " + term2str(right, color, false) + "]\n";
                        stmIt++;
                        continue;
                    }
                }

                // 溢出变量出栈
                result += loadSrcReg(left, color, indent, true);
                result += loadSrcReg(right, color, indent, false);

                if (binop == "+")
                    result += string(indent, ' ') + "add ";
                else if (binop == "-")
                    result += string(indent, ' ') + "sub ";
                else if (binop == "*")
                    result += string(indent, ' ') + "mul ";

                result += "r" + to_string(dstReg) + ", ";
                result += term2str(moveBinopStm->left, color) + ", ";
                result += term2str(moveBinopStm->right, color) + "\n";

                // 溢出变量入栈
                result += storeDstReg(dstTempNum, color, indent);
            }

            // Load: dst [src]
            // 出栈: src
            // 入栈: dst
            else if (stm->kind == QuadKind::LOAD) {
                auto loadStm = static_cast<QuadLoad*>(stm);
                auto dstTempNum = loadStm->dst->temp->num;
                auto dstReg = getDstReg(dstTempNum, color);

                // 溢出变量出栈
                result += loadSrcReg(loadStm->src, color, indent, true);

                result += string(indent, ' ') + "ldr r" + to_string(dstReg) + ", ";
                if (loadStm->src->kind == QuadTermKind::TEMP)
                    result += "[" + term2str(loadStm->src, color) + "]\n";
                else
                    result += term2str(loadStm->src, color) + "\n";

                // 溢出变量入栈
                result += storeDstReg(dstTempNum, color, indent);
            }

            // Store: src [dst]
            // 出栈: src, dst
            else if (stm->kind == QuadKind::STORE) {
                auto storeStm = static_cast<QuadStore*>(stm);

                // 溢出变量出栈
                result += loadSrcReg(storeStm->src, color, indent, true);
                result += loadSrcReg(storeStm->dst, color, indent, false);

                result += string(indent, ' ') + "str ";
                result += term2str(storeStm->src, color) + ", [";
                result += term2str(storeStm->dst, color) + "]\n";
            }

            // Call: obj_term
            // 出栈: obj_term
            else if (stm->kind == QuadKind::CALL) {
                auto callStm = static_cast<QuadCall*>(stm);
                // 溢出变量出栈
                result += loadSrcReg(callStm->obj_term, color, indent);
                result += string(indent, ' ') + "blx " + term2str(callStm->obj_term, color) + "\n";
            }

            // MoveCall: obj_term
            // 出栈: obj_term
            else if (stm->kind == QuadKind::MOVE_CALL) {
                auto moveCallStm = static_cast<QuadMoveCall*>(stm);
                // 溢出变量出栈
                result += loadSrcReg(moveCallStm->call->obj_term, color, indent);
                result += string(indent, ' ') + "blx " + term2str(moveCallStm->call->obj_term, color) + "\n";
            }

            // ExtCall: extfun
            else if (stm->kind == QuadKind::EXTCALL) {
                auto extCallStm = static_cast<QuadExtCall*>(stm);
                result += string(indent, ' ') + "bl " + extCallStm->extfun + "\n";
            }

            // MoveExtCall: extfun
            else if (stm->kind == QuadKind::MOVE_EXTCALL) {
                auto moveExtCallStm = static_cast<QuadMoveExtCall*>(stm);
                result += string(indent, ' ') + "bl " + moveExtCallStm->extcall->extfun + "\n";
            }

            // Jump: label
            else if (stm->kind == QuadKind::JUMP) {
                auto jumpStm = static_cast<QuadJump*>(stm);
                if (labelNext && labelNext->label->num == jumpStm->label->num) continue; // 相同跳转
                result += string(indent, ' ') + "b " + label2str(jumpStm->label) + "\n";
            }

            // CJump: left relop right t f
            // 出栈: left, right
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
        dfi->printLiveness();

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