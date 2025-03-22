# FDMJ-SLP 文法描述

复旦大学计算机专业2025年春季学期“编译(H)”

这是一个简化的文法，使用括号来严格确定操作的次序（如何表达操作优先级等，在后续学习Parsing（解析）时再解决）。“有值”或“无值”指的是词法分析在返回Token时是否附加一个值。*（注意每个Token都会带有一个位置信息“Pos”。）*

## 终结符（无值）

- `PUBLIC`
- `INT`
- `MAIN`
- `RETURN`

## 终结符（有值）

- `NONNEGATIVEINT` (非负整数)
- `IDENTIFIER` (标识符)
- `(` `)` `[` `]` `{` `}` `=` `,` `;` `.`
- `ADD` (加号)
- `MINUS` (减号)
- `TIMES` (乘号)
- `DIVIDE` (除号)

## 非终结符（需要类型信息）

值类型对应FDMJ-SLP中的classes的pointer
（除ID：类型为string 和 STMLIST：类型为vector<Stm*>*）
- `PROG` (程序)
- `MAINMETHOD` (主方法)
- `STM` (语句)
- `STMLIST` (语句列表)
- `EXP` (表达式)
- `ID` (标识符字符串常量)

## 文法规则

- AST: 抽象类
- Program: 程序
    - MainMethod
- MainMethod: 主方法
    - Stm列表
- Stm: 语句
    - Assign: 赋值语句 (IdExp, Exp)
    - Return: 返回语句 (Exp)
- Exp: 表达式
    - BinaryOp: 二元操作 (Exp, OpExp, Exp)
    - UnaryOp: 一元操作 (OpExp, Exp)
    - Esc: 逃逸表达式 (Stm列表, Exp)
    - IdExp: 标识符 (string)
    - IntExp: 整数 (int)
    - OpExp: 操作符 (string)
- 所有节点都有位置信息 (sline, scolumn, eline, ecolumn)


```sh
PROG: MAINMETHOD;

MAINMETHOD: PUBLIC INT MAIN '(' ')' '{' STMLIST '}';

STMLIST: /* empty */
       | STM STMLIST;

STM: ID '=' EXP ';'
    | RETURN EXP ';';

EXP: '(' EXP ADD EXP ')'
    | '(' EXP MINUS EXP ')'
    | '(' EXP TIMES EXP ')'
    | '(' EXP DIVIDE EXP ')'
    | NONNEGATIVEINT
    | '(' MINUS EXP ')'
    | '(' EXP ')'
    | '(' '{' STMLIST '}' EXP ')'
    | ID;

ID: IDENTIFIER;
```


