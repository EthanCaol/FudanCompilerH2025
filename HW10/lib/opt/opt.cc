#define DEBUG
// #undef DEBUG

#include <string>
#include <stack>
#include <variant>
#include <vector>
#include <map>
#include "quad.hh"
#include "opt.hh"

// TODO: 计算可执行块和变量
void Opt::calculateBT()
{
    // 常量传播
    vector<QuadStm*> W;
    for (auto block : *func->quadblocklist)
        W.insert(W.end(), block->quadlist->begin(), block->quadlist->end());

    // 遍历处理每个语句
    auto W_origin = W;
    while (!W.empty()) {
        QuadStm* stm = W.front();
        W.erase(W.begin());

        // 语句->标签
        if (stm->kind == QuadKind::LABEL) {
            auto label = static_cast<QuadLabel*>(stm);
            auto block = label2block[label->label->num];
            // 可执行块的直接后继 也是可执行块
            if (block_executable[label->label->num] && block->exit_labels->size() == 1) {
                auto next_label = block->exit_labels->at(0);
                if (setExecutable(next_label->num, true))
                    W = W_origin;
            }
        }

        // 语句->条件跳转
        // 如果 V[x]=c1 && V[y]=c2, 那么 E[L1], E[L2] = c1 relop c2
        // 如果 V[x]=T || V[y]=T, 那么 E[L1]=true, E[L2]=true
        else if (stm->kind == QuadKind::CJUMP) {
            auto cjump = static_cast<QuadCJump*>(stm);
            auto left = cjump->left, right = cjump->right;
            bool left_one = false, right_one = false;
            int left_value = 0, right_value = 0;

            if (left->kind == QuadTermKind::CONST) {
                left_one = true;
                left_value = get<int>(left->term);
            } else if (left->kind == QuadTermKind::TEMP) {
                auto left_temp = get<TempExp*>(left->term);
                auto left_rtvalue = getRtValue(left_temp->temp->num);
                if (left_rtvalue.getType() == ValueType::ONE_VALUE) {
                    left_one = true;
                    left_value = left_rtvalue.getIntValue();
                } else if (left_rtvalue.getType() == ValueType::MANY_VALUES) {
                    if (setExecutable(cjump->t->num, true) || setExecutable(cjump->f->num, true))
                        W = W_origin;
                }
            }

            if (right->kind == QuadTermKind::CONST) {
                right_one = true;
                right_value = get<int>(right->term);
            } else if (right->kind == QuadTermKind::TEMP) {
                auto right_temp = get<TempExp*>(right->term);
                auto right_rtvalue = getRtValue(right_temp->temp->num);
                if (right_rtvalue.getType() == ValueType::ONE_VALUE) {
                    right_one = true;
                    right_value = right_rtvalue.getIntValue();
                } else if (right_rtvalue.getType() == ValueType::MANY_VALUES) {
                    if (setExecutable(cjump->t->num, true) || setExecutable(cjump->f->num, true))
                        W = W_origin;
                }
            }

            // 如果左边和右边都是单值
            if (left_one && right_one) {
                bool result = false;
                if (cjump->relop == "==")
                    result = (left_value == right_value);
                else if (cjump->relop == "!=")
                    result = (left_value != right_value);
                else if (cjump->relop == "<")
                    result = (left_value < right_value);
                else if (cjump->relop == "<=")
                    result = (left_value <= right_value);
                else if (cjump->relop == ">")
                    result = (left_value > right_value);
                else if (cjump->relop == ">=")
                    result = (left_value >= right_value);

                if (result == true) {
                    if (setExecutable(cjump->t->num, true))
                        W = W_origin;
                } else if (result == false) {
                    if (setExecutable(cjump->f->num, true))
                        W = W_origin;
                }
            }
        }

        // 语句->赋值
        else if (stm->kind == QuadKind::MOVE) {
            auto move = static_cast<QuadMove*>(stm);
            if (move->src->kind == QuadTermKind::CONST) {
                if (setRtValue(move->dst->temp->num, RtValue(get<int>(move->src->term))))
                    W = W_origin;
            }

            else if (move->src->kind == QuadTermKind::TEMP) {
                auto src_rtvalue = getRtValue(get<TempExp*>(move->src->term)->temp->num);
                if (setRtValue(move->dst->temp->num, src_rtvalue))
                    W = W_origin;
            }
        }

        // 语句->二元操作赋值
        // 对于v <- x op y
        // 如果 V[x]=c1 && V[y]=c2, 那么 V[v] = c1 op c2
        // 如果 V[x]=T || V[y]=T, 那么 V[v] = T
        else if (stm->kind == QuadKind::MOVE_BINOP) {
            auto move_binop = static_cast<QuadMoveBinop*>(stm);
            auto left = move_binop->left, right = move_binop->right;
            bool left_one = false, right_one = false;
            int left_value = 0, right_value = 0;

            if (left->kind == QuadTermKind::CONST) {
                left_one = true;
                left_value = get<int>(left->term);
            } else if (left->kind == QuadTermKind::TEMP) {
                auto left_temp = get<TempExp*>(left->term);
                auto left_rtvalue = getRtValue(left_temp->temp->num);
                if (left_rtvalue.getType() == ValueType::ONE_VALUE) {
                    left_one = true;
                    left_value = left_rtvalue.getIntValue();
                } else if (left_rtvalue.getType() == ValueType::MANY_VALUES) {
                    if (setRtValue(move_binop->dst->temp->num, RtValue(ValueType::MANY_VALUES)))
                        W = W_origin;
                }
            }

            if (right->kind == QuadTermKind::CONST) {
                right_one = true;
                right_value = get<int>(right->term);
            } else if (right->kind == QuadTermKind::TEMP) {
                auto right_temp = get<TempExp*>(right->term);
                auto right_rtvalue = getRtValue(right_temp->temp->num);
                if (right_rtvalue.getType() == ValueType::ONE_VALUE) {
                    right_one = true;
                    right_value = right_rtvalue.getIntValue();
                } else if (right_rtvalue.getType() == ValueType::MANY_VALUES) {
                    if (setRtValue(move_binop->dst->temp->num, RtValue(ValueType::MANY_VALUES)))
                        W = W_origin;
                }
            }

            if (left_one && right_one) {
                int result = 0;
                if (move_binop->binop == "+")
                    result = left_value + right_value;
                else if (move_binop->binop == "-")
                    result = left_value - right_value;
                else if (move_binop->binop == "*")
                    result = left_value * right_value;

                if (setRtValue(move_binop->dst->temp->num, RtValue(result)))
                    W = W_origin;
            }
        }

        // 语句->Phi函数
        // 如果 V[xi]=T && E[i], 那么 V[v]=T
        // 如果 V[xi]!=V[xj] && E[i] && E[j], 那么 V[v]=T
        // 如果 V[xi]=c && E[i], 其他 !E[j] || V[xj]=Λ || V[xj]=c, 那么 V[v]=c
        else if (stm->kind == QuadKind::PHI) {
            int prev_value = 0;
            bool has_value = false;
            auto phi = static_cast<QuadPhi*>(stm);
            for (auto& arg : *phi->args) {
                auto temp = arg.first;
                auto label = arg.second;
                auto rtvalue = getRtValue(temp->num);

                // 如果 V[xi]=T && E[i], 那么 V[v]=T
                if (rtvalue.getType() == ValueType::MANY_VALUES && block_executable[label->num]) {
                    if (setRtValue(phi->temp->temp->num, RtValue(ValueType::MANY_VALUES)))
                        W = W_origin;
                }

                // 如果 V[xi]!=V[xj] && E[i] && E[j], 那么 V[v]=T
                else if (rtvalue.getType() == ValueType::ONE_VALUE && block_executable[label->num]) {
                    int cur_value = rtvalue.getIntValue();
                    if (has_value && prev_value != cur_value) {
                        if (setRtValue(phi->temp->temp->num, RtValue(ValueType::MANY_VALUES)))
                            W = W_origin;
                        break;
                    }
                    has_value = true;
                    prev_value = cur_value;
                }
            }

            if (has_value) {
                // 如果 V[xi]=c && E[i], 其他 !E[j] || V[xj]=Λ || V[xj]=c, 那么 V[v]=c
                if (setRtValue(phi->temp->temp->num, RtValue(prev_value)))
                    W = W_origin;
            }
        }

        // 语句->读内存
        else if (stm->kind == QuadKind::LOAD) {
            auto load = static_cast<QuadLoad*>(stm);
            if (setRtValue(load->dst->temp->num, RtValue(ValueType::MANY_VALUES)))
                W = W_origin;
        }

        // 语句->类方法函数调用 (需要赋值)
        else if (stm->kind == QuadKind::MOVE_CALL) {
            auto move_call = static_cast<QuadMoveCall*>(stm);
            if (setRtValue(move_call->dst->temp->num, RtValue(ValueType::MANY_VALUES)))
                W = W_origin;
        }

        // 语句->外部函数调用 (需要赋值)
        else if (stm->kind == QuadKind::MOVE_EXTCALL) {
            auto move_extcall = static_cast<QuadMoveExtCall*>(stm);
            if (setRtValue(move_extcall->dst->temp->num, RtValue(ValueType::MANY_VALUES)))
                W = W_origin;
        }
    }
}

// TODO: 更新代码
void Opt::modifyFunc() { }

QuadFuncDecl* Opt::optFunc()
{
    // 初始化所有块为不可达
    for (auto& block : *func->quadblocklist) {
        block_executable[block->entry_label->num] = false;
        label2block[block->entry_label->num] = block;
    }

    // 初始化入口块为可达
    block_executable[func->quadblocklist->at(0)->entry_label->num] = true;

    // 初始化所有函数参数为多值
    for (auto& temp : *func->params) {
        temp_value[temp->num] = RtValue(ValueType::MANY_VALUES);
    }

    calculateBT();

    printRtValue();
    printBlockExecutable();

    modifyFunc();

    return func;
}

QuadProgram* optProg(QuadProgram* prog)
{
    QuadProgram* newProg = new QuadProgram(nullptr, new vector<QuadFuncDecl*>());
    for (int i = 0; i < prog->quadFuncDeclList->size(); i++) {
        Opt optthis(prog->quadFuncDeclList->at(i));
        newProg->quadFuncDeclList->push_back(optthis.optFunc());
    }
    return newProg;
}