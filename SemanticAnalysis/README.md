# 语义分析与中间代码生成实验报告

## 1. 功能描述
本实验实现了一个 **语法制导翻译 (Syntax-Directed Translation)** 系统，在进行语法分析的同时进行：
- **语义检查**: 变量类型检查、未声明变量使用检查、除零错误检查。
- **中间代码生成**: 生成四元式 (Quadruple) 形式的中间代码 (`ir_code.txt`)。
- **符号表管理**: 维护变量的类型 (int/real) 和值 (`symbol_table_snapshot.csv`)。

## 2. 输入输出
- **输入**: 源代码。
- **输出**:
    - `SemanticAnalysis/output/ir_code.txt`: 生成的中间代码序列。
    - `SemanticAnalysis/output/symbol_table_snapshot.csv`: 程序结束时的符号表快照。
    - 控制台输出: 语义错误提示（如除零）。

## 3. 数据结构
- **符号表**: `SymbolTable` 类，内部使用 `unordered_map<string, Symbol>`。
    - `Symbol` 结构体包含 `name`, `type` (INT/REAL), `value`。
- **中间代码生成器**: `IRGenerator` 类，维护 `vector<Quadruple>`。
    - `Quadruple`: `{op, arg1, arg2, result}`。

## 4. 实现算法
采用 **递归下降分析** 配合 **语义动作**：
- **声明处理**: 遇到 `int a = 1;`，在符号表中注册变量并生成 `DECL` 指令。
- **赋值语句**: 解析表达式，进行类型推导（Type Inference）或转换，更新符号表，生成 `=` 指令。
- **控制流**:
    - `if-else`: 生成条件跳转指令 (`IF_GT`, `GOTO` 等)。
    - 采用回填 (Backpatching) 思想的简化版或直接生成跳转标签。
- **优化读取**: 实现了 `BufferedReader` 和 `read_safe`，高效处理输入流并过滤非法字符。

## 5. 错误处理
- **除零检查**: 在除法运算前检查除数是否接近 0。
    ```cpp
    if (abs(val2) < 1e-6) {
        printf("error message:line %d,division by zero\n", line);
        flag_err = 0;
    }
    ```
- **类型检查**: 虽为弱类型推导，但内部区分 INT 和 REAL，并在运算时进行适当处理。

## 6. 亮点概述
- **即时解释执行**: 分析器不仅生成中间代码，还实时计算变量值（解释器模式），因此能在分析结束时输出准确的符号表快照（包含最终运行结果）。
- **类型推导与混合运算**: 支持整型和浮点型的混合运算及自动类型转换。

**代码证据**:
```cpp
// 解释执行与类型处理
if (sym.type == TYPE_INT)
    symTable.setInt(result, (int)res);
else
    symTable.setReal(result, res);
```
