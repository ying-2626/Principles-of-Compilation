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
1. **解释器模式与中间代码生成并行 (Interpretation & Compilation)**
   分析器不仅仅是生成中间代码（编译），同时还充当了一个即时解释器。在解析赋值和算术运算时，它会实时计算变量的实际值并更新符号表。这使得程序在分析结束时能够直接输出最终的变量状态快照，极大地便利了调试和验证。
   **代码证据**:
   ```cpp
   // 生成代码的同时计算值
   ir.gen(OP_MUL, arg1, arg2, resVar); // 生成 MUL t1, a, b
   double res = val1 * val2; // 实时计算结果
   ```

2. **类型自动提升与混合运算 (Type Promotion)**
   实现了完善的类型系统，支持 `int` 和 `real` (float) 类型的混合运算。在运算过程中，如果操作数包含实数，系统会自动将整型操作数提升为实数进行计算，并生成相应的类型转换逻辑，确保了语义的正确性。
   **代码证据**:
   ```cpp
   // 混合类型处理
   if (sym1.type == TYPE_REAL || sym2.type == TYPE_REAL) {
       // ... 执行实数运算 ...
       targetType = TYPE_REAL; 
   } else {
       // ... 执行整数运算 ...
   }
   ```

3. **符号表快照导出 (Snapshot Export)**
   设计了符号表的序列化功能，能够将当前所有变量的状态（名称、类型、当前值）以 CSV 格式导出。这不仅用于最终结果展示，也可以用于程序运行过程中的状态监控。
   **代码证据**:
   ```cpp
   // 导出 CSV 快照
   fprintf(fp, "Name,Type,Value\n");
   for (auto &pair : table) {
       fprintf(fp, "%s,%s,%.2f\n", pair.first.c_str(), 
               (pair.second.type==TYPE_INT?"int":"real"), pair.second.value);
   }
   ```
