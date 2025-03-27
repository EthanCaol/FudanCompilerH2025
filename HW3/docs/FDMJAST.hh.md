### 程序结构类

- Program: 程序
  - MainMethod
  - ClassDecl

- MainMethod: 主方法
  - VarDecl
  - Stm

- ClassDecl: 类声明
  - IdExp: 类标识符
  - IdExp: 继承类标识符
  - VarDecl
  - MethodDecl

### 类型和声明类

- Type: 类型信息
  - TypeKind: 类型分类 (INT, CLASS, ARRAY)
  - IdExp: 类标识符 (CLASS 类型)
  - IntExp: 数组维度 (ARRAY 类型)

- VarDecl: 变量声明
  - Type: 变量类型
  - IdExp: 变量标识符
  - 初始化值 (IntExp 或 IntExp 数组)

- MethodDecl: 方法声明
  - Type: 返回类型
  - IdExp: 方法标识符
  - Formal
  - VarDecl
  - Stm

- Formal: 方法参数
  - Type: 参数类型
  - IdExp: 参数标识符

### 语句类

- Stm (基类): 所有语句类型的基类

**控制流语句:**
- If: If-else 语句
- While: While 循环语句
- Nested: 语句块
- Continue: Continue 语句
- Break: Break 语句
- Return: Return 语句

**赋值和调用语句:**
- Assign: 赋值语句
- CallStm: 方法调用语句

**输入输出语句:**
- PutInt: 打印整数语句
- PutCh: 打印字符语句
- PutArray: 打印数组语句
- GetInt: 读取整数语句
- GetCh: 读取字符语句
- GetArray: 读取数组语句

**计时语句:**
- Starttime: 开始计时语句
- Stoptime: 停止计时语句

### 表达式类

- Exp (基类): 所有表达式类型的基类

**操作符:**
- BinaryOp: 二元操作表达式
- UnaryOp: 一元操作表达式
- OpExp: 操作符表示

**访问表达式:**
- ArrayExp: 数组访问表达式
- ClassVar: 类变量访问表达式
- CallExp: 方法调用表达式
- Length: 数组长度表达式

**字面量表达式:**
- BoolExp: 布尔字面量
- IntExp: 整数字面量
- IdExp: 标识符表达式

**特殊表达式:**
- This: 'this' 引用
- Esc: 带语句块的表达式

### 通用特性
所有类:
- 继承自 AST 基类
- 包含位置信息 (Pos)
- 实现 clone() 方法用于深拷贝
- 通过 accept() 方法支持访问者模式
- 具有唯一的 ASTKind 标识符
