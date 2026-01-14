# LR(1) 语法分析器实验报告

## 1. 功能描述
本实验实现了一个基于 **LR 分析表** (实际上是 SLR 或 LALR，取决于生成的表) 的语法分析器。
支持与 LL 分析器相同的文法子集（赋值、循环、条件、块结构），但采用自底向上的分析方式。

## 2. 输入输出
- **输入**: 源代码。
- **输出**:
    - 移进-规约过程的日志。
    - 语法树的可视化 `.dot` 文件。
    - 精确的错误报告。

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
    - 优先尝试插入闭括号 `)`，以解决 `while(...)` 结构中常见的误报问题。
    - 如果插入后能查到有效动作，则执行该动作并报错，但不消耗实际输入。
2. **恐慌模式**: 如果虚拟插入失败，则跳过当前非法 Token。
3. **行号修正**: 同样引入 `lastAcceptedTokenLine`，确保报错行号准确对应上一个有效 Token。

## 6. 亮点概述
1. **深度前瞻错误验证机制 (Deep Lookahead Validation)**
   为了解决传统 LR 错误恢复中“试图插入符号但后续仍失败”的问题，设计了 `isCandidateValid` 函数。该函数不仅检查当前状态是否允许插入符号，还会**模拟**解析过程（执行一系列 Reduce 动作），直到确认能够成功 Shift 该符号。这种机制彻底消除了将缺少 `)` 误报为缺少 `;` 的情况。
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

2. **自动化表生成与代码导出 (Table Generation & Export)**
   编写了 `maker.cpp` 工具，实现了从文法定义到 SLR 分析表的完整自动化构建流程（计算 First/Follow 集、构建项目集闭包、生成 DFA）。更进一步，该工具直接生成 C++ 头文件 `LRTable.h`，将计算好的 Action/Goto 表硬编码为数组，极大地提高了运行时效率。
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
