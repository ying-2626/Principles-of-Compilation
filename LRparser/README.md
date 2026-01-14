# LR(1) 语法分析器实验报告

## 1. 功能描述
本实验实现了一个基于 **LR 分析表** (实际上是 SLR 或 LALR，取决于生成的表) 的语法分析器。
支持与 LL 分析器相同的文法子集（赋值、循环、条件、块结构），但采用自底向上的分析方式。

## 2. 输入输出
- **输入**: 源代码，实验中测试文件位于 `dataset/parser/lr` 目录，例如 `lr_test1.txt`、`lr_test2.txt`、`lr_error_test1.txt`、`lr_error_test2.txt` 等。
- **输出**:
    - 语法错误提示（如有），默认打印到标准输出；通过 `run_tests.py` 运行时会被重定向到对应输入文件旁的 `*.out` 文件（例如 `dataset/parser/lr/lr_test1.txt.out`）。
    - 控制台输出的语法树结构文本表示（缩进形式，同样会被写入 `.out` 文件中，便于与不同测试用例对比）。
    - 语法树的可视化 `.dot` 文件，默认名为 `lr_tree.dot`，在配合测试脚本运行时会被移动并重命名为与输入一一对应的 `*.dot`（例如 `dataset/parser/lr/lr_test1.txt.dot`），如果环境中安装了 Graphviz，还会进一步生成同名 `.png` 图片。

## 3. 数据结构
- **ACTION 表**: `struct Action { int type; int val; }`，二维数组，存储移进、规约、接受动作。
- **GOTO 表**: 二维数组，存储非终结符的状态转移。
- **状态栈**: `stack<int> stateStack`，维护自动机状态。
- **节点栈**: `stack<ASTNode *> nodeStack`，用于在规约时构建语法树节点。

## 4. 实现算法
采用 **移进-规约 (Shift-Reduce) 分析算法**：
1. **主循环**: 根据栈顶状态和当前向前看符号查 `ACTION` 表。
2. **移进 (Shift)**: 将新状态压栈，创建终结符叶节点压入节点栈，消费输入。
3. **规约 (Reduce)**:
    - 弹出 `|右部|` 个状态和节点。
    - 创建新的非终结符父节点，将其子节点指向弹出的节点。
    - 查 `GOTO` 表压入新状态。
4. **接受 (Accept)**: 分析成功结束。

## 5. 错误处理
实现了 **虚拟插入 (Virtual Insertion)** 与 **恐慌模式** 结合的策略：
1. **虚拟插入**: 当查表失败（报错）时，尝试在输入流前“假想”插入常见的缺失符号（如 `;`, `)`, `]`, `}`）。
    - 如果插入后能查到有效动作，则执行该动作并报错，但不消耗实际输入。
2. **恐慌模式**: 如果虚拟插入失败，则跳过当前非法 Token。
3. **行号修正**: 同样引入 `lastAcceptedTokenLine`，确保报错行号准确对应上一个有效 Token。

## 6. 亮点概述
1. **分析表生成与解析分离的生产者–消费者架构**
   LR 分析流程被拆分为两个低耦合模块：生成端 `maker.cpp` 从文法出发计算 FIRST/FOLLOW 集、构造项目集族与 DFA，并生成 `LRTable.h`；消费端 LR 解析器仅依赖 `LRTable.h` 中硬编码的 ACTION/GOTO 表完成移进–规约分析，实现了典型的生产者–消费者模式，文法变化时只需重新生成表而无需修改解析核心。

2. **自动化表生成与代码导出 (Table Generation & Export)**
   编写了 `maker.cpp` 工具，实现了从文法定义到 SLR 分析表的完整自动化构建流程（计算 First/Follow 集、构建项目集闭包、生成 DFA）。更进一步，该工具直接生成 C++ 头文件 `LRTable.h`，将计算好的 ACTION/GOTO 表硬编码为数组，极大地提高了运行时效率。
   **代码证据**:
   ```cpp
   // 自动生成 C++ 代码
   cout << "Action ACTION[STATE_COUNT][TERM_COUNT];" << endl;
   cout << "void initLRTable() {" << endl;
   // ... 遍历表生成赋值语句 ...
   cout << "    ACTION[" << i << "][" << j << "] = {" << act.type << ", " << act.val << "};" << endl;
   ```

3. **语法树构建与可视化**
   在自底向上的规约过程中同步构建 AST 节点，并维护了节点栈。分析完成后，利用 `flattenAST` 将树结构序列化，并对接 `Visualizer` 模块生成可视化图表，清晰展示了自底向上的建树逻辑。
   **代码证据**:
   ```cpp
   // 规约时构建树节点
   ASTNode *newNode = new ASTNode(lhsName);
   newNode->children = children; // 将弹出的子节点挂载
   nodeStack.push(newNode);
   ```
4.  **深度前瞻错误验证机制 (Deep Lookahead Validation)**
   设计了 `isCandidateValid` 函数，不仅检查当前状态是否允许插入符号，还会**模拟**解析过程（执行一系列 Reduce 动作），直到确认能够成功 Shift 该符号。
   **代码证据**:
   ```cpp
   // 模拟解析过程验证候选符号
   bool isCandidateValid(int startState, char candidate, const stack<int>& originalStack) {
       // ... 复制栈状态 ...
       while (steps < MAX_STEPS) {
           if (act.type == Shift) return true; // 成功 Shift，验证通过
           else if (act.type == Reduce) { ... } // 模拟规约
           else return false; // 遇到错误，验证失败
       }
   }
   ```

5. **行号追踪与多错误场景验证**
   在移进过程中维护 `lastAcceptedTokenLine`，记录上一次成功移进的 Token 行号，在报告缺少 `;`、`)` 等错误时以此作为基准行号，从而避免错误行号落在下一条语句或空行上。通过同时缺少多种符号的错误用例（`lr_error_test1.txt`、`lr_error_test2.txt` 等），验证了错误恢复策略在复杂场景下仍能给出稳定且不重复的错误提示。

6. **数组化 ACTION/GOTO 表与泛型容器实现的性能权衡**
   运行时解析阶段完全依赖 `ACTION[状态][终结符]` 和 `GOTO[状态][非终结符]` 这两张静态数组表，查表过程退化为简单的下标寻址。与基于平衡树/哈希表等泛型容器的“映射式表结构”相比，这种数组化布局更加贴近 LR 理论中的抽象矩阵形式，能够最大限度地利用 CPU 缓存局部性，减少指针跳转开销。因此，在状态数和终结符/非终结符集合规模中等的教学场景下，该实现不仅比链式结构更高效，也比完全手写的 `switch-case` 规约逻辑更易于自动生成和维护，在**可维护性**与**解析性能**之间取得了良好的平衡。

## 7. 测试与验证

- 为 LR 分析器构造了包含完全正确输入和多类错误输入的测试集，包括基础用例 `lr_test1.txt`、`lr_test2.txt`，以及缺少 `;`、缺少 `)` 等典型语法错误的组合用例 `lr_error_test1.txt`、`lr_error_test2.txt`。这些用例同时被用于验证深度前瞻错误恢复与行号追踪机制的效果。
