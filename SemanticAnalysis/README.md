# 语义分析与中间代码生成实验报告

## 1. 功能描述
本实验实现了一个 **语法制导翻译 (Syntax-Directed Translation)** 系统，在进行语法分析的同时进行：
- **语义检查**: 变量类型检查、未声明变量使用检查、除零错误检查。
- **中间代码生成**: 生成四元式 (Quadruple) 形式的中间代码 (`ir_code.txt`)。
- **符号表管理**: 维护变量的类型 (int/real) 和值 (`symbol_table_snapshot.csv`)。

## 2. 输入输出
- **输入**: 源代码，实验中典型输入文件位于 `dataset/semantic` 目录，例如 `test1.txt`、`test2.txt`、`error_test2.txt` 等。
- **输出**:
    - `SemanticAnalysis/output/<输入文件名前缀>_ir_code.txt`: 生成的中间代码序列（例如 `test1_ir_code.txt`）。
    - `SemanticAnalysis/output/<输入文件名前缀>_symbol_table_snapshot.csv`: 程序结束时的符号表快照（例如 `test1_symbol_table_snapshot.csv`）。
    - 控制台输出: 语义错误提示（如除零），通过 `run_tests.py` 运行时会被重定向到对应输入文件旁的 `*.out` 文件（例如 `dataset/semantic/error_test2.txt.out`）。

## 3. 数据结构
- **符号表**: `SymbolTable` 类，内部使用 `unordered_map<string, Symbol>`。
    - `Symbol` 结构体包含 `name`, `type` (INT/REAL), `value`。
- **中间代码生成器**: `IRGenerator` 类，维护 `vector<Quadruple>`。
    - `Quadruple`: `{op, arg1, arg2, result}`。

## 4. 实现算法
采用两阶段的语法制导翻译流程：
- **阶段 1：声明段**  
  顺序读取形如 `int a = 1;`、`real c = 3.0;` 的声明语句，在符号表中注册变量并生成对应的 `DECL_INT`、`DECL_REAL` 四元式。
- **阶段 2：语句段**  
  从第二行开始按行处理复合语句块 `{ ... }` 内的赋值语句和 `if-else` 语句，使用 `solve_opt_wrapper`、`solveif_opt` 等过程函数逐行解析，一边计算表达式值并更新符号表，一边发射 `=`, `+`, `-`, `*`, `/` 等四元式，实现“解释执行 + 中间代码生成并行”。
- **优化读取**: 通过 `BufferedReader` 和 `read_safe` 封装底层输入流，只保留字母、数字与运算符字符，跳过空白和无效符号，提高了语义分析阶段的输入稳定性和 I/O 效率。

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

4. **基于 Hash 的符号表与顺序扫描实现的对比**
   符号表底层使用 `unordered_map<string, Symbol>` 存储标识符信息，相较于简单的顺序数组/链表符号表，每次查找/插入的期望时间复杂度由 $O(n)$ 降为接近 $O(1)$，在包含较多变量和多次查表的算术/分支组合场景中显著降低了语义分析阶段的总开销。同时，通过为四元式序列采用顺序存储的 `vector<Quadruple>` 结构，既方便按照生成顺序线性输出，又便于后续进行简单的遍历式优化（如死代码删除或常量折叠），相比在链表结构上操作具有更好的缓存友好性和遍历性能，使得“解释执行 + IR 记录”在工程上更加轻量高效。

## 7. 测试与验证

- 语义分析测试集中包含了 `test1.txt` 等正常用例，用于验证在多语句、多分支情况下符号表最终状态与生成的四元式是否与手算结果一致。
- 通过新增 `test2.txt` 和 `error_test2.txt` 等用例覆盖更多算术组合和运行时错误场景，其中 `error_test2.txt` 专门构造了除零等非法运算，用于验证除零检测与错误信息中的行号是否与实际源码位置一致，并检查在出现错误后仍能正确生成符号表快照与中间代码文件。
