
# HW1-实验报告

## 重构`MinusIntConverter`

将所有`visit`转换为下述5步骤:
1. 判断参数非空
2. 判断子结点非空
3. 调用accept(visit)
4. 记录下层newNode
3. 更新本层newNode

## 实现`ConstantPropagater`

依照`MinusIntConverter`中`visit`的设计模式实现
1. 移除`visit(UnaryOp*)`中的取负一元运算
2. 添加`visit(BinaryOp*)`中的二元运算

## 实现`Executer`

将所有`visit`转换为下述4步骤:
1. 判断参数非空
2. 判断子结点非空
3. 调用accept(visit)
4. 执行所需操作

### 对外接口`int execute(Program* root)`
1. 确保`accept`执行完成后, AST具有下述结构
   - Program
     - MainMethod
       - StmList
         - Return(int)
2. 从`Return`中取出`int`并返回其值

### 赋值语句: `visit(Assign*)`
1. 解析右侧表达式值
2. 将值赋给左侧变量(更新哈希表)

### 返回语句: `visit(Return*)`
1. 解析返回值
2. 将值记录到`Return`的内部成员`retVal`

### 运算表达式: `visit(BinaryOp*)`与`visit(UnaryOp*)`
1. 取出表达式值
2. 执行运算, 并构建返回值`IntExp`

### 逃逸表达式: `visit(Esc*)`
1. 调用执行`accept`后返回最后一个表达式