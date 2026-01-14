**课程名称**：**编译原理与技术实践**  
**实验日期**：**2026年01月13日**

**运行说明**

- **环境依赖**
  - 操作系统：Windows 11
  - 编译器：基于 VS Code 的 C/C++ 扩展和 MinGW `g++`（编译器路径在 [`.vscode/c_cpp_properties.json`](https://github.com/ying-2626/Principles-of-Compilation/blob/main/.vscode/c_cpp_properties.json) 中目前配置为 `D:/vscodeENV/MinGW/bin/g++.exe`，更改为实际编译器路径）
  - Python：推荐 **Python 3.8+**，用于运行一键测试脚本 `run_tests.py`
  - 可选：安装 Graphviz 并将 `dot` 命令加入环境变量，可自动将语法树 DOT 转换为 PNG 图片；如果本地暂未安装 Graphviz，也可以将生成的 `.dot` 文件内容复制到 <https://dreampuf.github.io/GraphvizOnline/> 在线查看语法树

- **VS Code 工程环境说明**
  - `.vscode/c_cpp_properties.json` 中预置了 MinGW 的头文件和库目录，保证 IntelliSense 和编译选项与本地环境一致。
  - 推荐通过 VS Code 打开本仓库后，直接使用内置终端运行下面的命令；如需在其他环境编译，请确保 `g++` 和 Python 均已正确安装并加入 `PATH`。

- **单个模块手动编译与运行**
  下面命令均在项目根目录下执行：

  1. **词法分析器（LexicalAnalysis）**
     ```bash
     g++ LexicalAnalysis/main.cpp -I LexicalAnalysis -o LexicalAnalysis/output/main.exe
     LexicalAnalysis/output/main.exe dataset/lexical/test1.c
     ```
     - 输入：`dataset/lexical/*.c` 源文件
     - 输出：Token 序列打印到控制台；如需保存，可重定向到文件，例如：  
       `LexicalAnalysis/output/main.exe dataset/lexical/test1.c > dataset/lexical/test1.c.out`

  2. **LL(1) 语法分析器（LLparser）**
     ```bash
     g++ LLparser/LLparserMain.cpp -I LLparser -o LLparser/output/LLparser.exe
     LLparser/output/LLparser.exe dataset/parser/ll/ll_test1.txt
     ```
     - 输入：`dataset/parser/ll/ll_*.txt`
     - 输出：控制台打印语法分析结果和语法树文本；可选生成 `ll_tree.dot`，用于语法树可视化。

  3. **LR 语法分析器（LRparser）**
     ```bash
     g++ LRparser/LRparserMain.cpp -I LRparser -o LRparser/output/LRparser.exe
     LRparser/output/LRparser.exe dataset/parser/lr/lr_test1.txt
     ```
     - 输入：`dataset/parser/lr/lr_*.txt`
     - 输出：控制台打印语法分析结果和语法树文本；可选生成 `lr_tree.dot`。

  4. **语义分析与中间代码生成（SemanticAnalysis）**
     ```bash
     g++ SemanticAnalysis/TranslationSchema.cpp -I SemanticAnalysis -o SemanticAnalysis/main.exe
     SemanticAnalysis/main.exe dataset/semantic/test1.txt
     ```
     - 输入：`dataset/semantic/*.txt`
     - 输出：  
       - `SemanticAnalysis/output/<输入文件名前缀>_ir_code.txt`：中间代码  
       - `SemanticAnalysis/output/<输入文件名前缀>_symbol_table_snapshot.csv`：符号表快照  
       - 控制台打印语义错误信息（如除零）

- **一键编译与测试（推荐）**
  - 在项目根目录下执行：
    ```bash
    python run_tests.py
    ```
  - 功能说明：
    - 自动为四个模块依次调用 `g++` 完成编译（命令配置见 [`run_tests.py`](https://github.com/ying-2626/Principles-of-Compilation/blob/main/run_tests.py) 中的 `PROJECTS` 字典）
    - 遍历对应测试数据目录：
      - 词法分析：`dataset/lexical`
      - LL 语法分析：`dataset/parser/ll`
      - LR 语法分析：`dataset/parser/lr`
      - 语义分析：`dataset/semantic`
    - 对每个测试文件运行相应可执行程序，并将标准输出与错误输出写入同目录下的 `.out` 文件，例如：  
      `dataset/semantic/error_test2.txt.out`
    - 对 LL/LR 模块，如果生成了 `ll_tree.dot`/`lr_tree.dot`，脚本会将其重命名并移动到对应测试文件旁边（如 `ll_test1.txt.dot`），并在系统安装 Graphviz 时自动生成 `.png` 图片

**项目结构**

```text
lab_pro/
├─ LexicalAnalysis/                 词法分析模块源码与可执行文件
│  ├─ main.cpp                      词法分析器入口
│  ├─ LexAnalysis.h                 词法分析核心逻辑（DFA、Token 输出等）
│  ├─ README.md                     词法分析实验报告与说明
│  └─ output/                       词法分析可执行文件与输出目录
├─ LLparser/                        LL(1) 语法分析模块
│  ├─ LLparserMain.cpp              LL 语法分析器入口
│  ├─ LLparser.h                    LL 分析核心（预测分析表、Trie、错误恢复等）
│  ├─ README.md                     LL 实验报告与说明
│  └─ output/                       LL 语法分析器可执行文件与输出
├─ LRparser/                        LR 语法分析模块
│  ├─ LRparserMain.cpp              LR 语法分析器入口
│  ├─ LRparser.h                    LR 分析核心（移进–规约、错误恢复等）
│  ├─ LRTable.h                     由 maker.cpp 自动生成的 LR 分析表
│  ├─ maker.cpp                     LR 分析表生成工具
│  ├─ README.md                     LR 实验报告与说明
│  └─ output/                       LR 语法分析器可执行文件与输出
├─ SemanticAnalysis/                语义分析与中间代码生成模块
│  ├─ TranslationSchema.cpp         语义分析与翻译主程序入口
│  ├─ TranslationSchema.h           语义分析相关类型与接口声明
│  ├─ TranslationSchemaOptimized.h  语法制导翻译与 IR 生成的优化实现
│  ├─ README.md                     语义分析实验报告与说明
│  └─ output/                       IR 与符号表导出目录
├─ dataset/                    各阶段测试用例集合
│  ├─ lexical/                 词法分析测试 C 源文件
│  ├─ parser/
│  │  ├─ ll/                   LL 语法分析测试用例
│  │  └─ lr/                   LR 语法分析测试用例
│  └─ semantic/                语义分析测试用例
├─ .vscode/                    VS Code 配置
│  └─ c_cpp_properties.json    C/C++ 扩展的 IntelliSense 与编译配置
├─ run_tests.py                一键编译并运行四个模块的测试脚本
└─ README.md                   项目运行说明
```

---
**实践亮点：**

1. **LR 分析器自动生成分析表**  
   在 **LR 分析器** 中，利用 `maker.cpp` 从文法自动生成 FIRST/FOLLOW 集和 SLR 分析表，并导出为头文件 `LRTable.h`，避免人工推导和运行时建表开销。(3)

2. **语法树可视化与对比**  
   在 **语法树可视化** 模块中，实现了自动生成 AST 的 Graphviz **DOT** 语言与图片导出，直观对比 **LL** 与 **LR** 的建树差异。(2、3)

3. **符号管理中的数据结构选择与复杂度控制**  
   在 **符号管理** 模块中，对不同阶段选择了更合适的数据结构：`std::map`、**Trie + 哈希表双向映射**、二维数组、`std::unordered_map` 等，提升效率并平衡可读性。(1、2、3、4)

4. **词法与语法分析的 DFA + 表驱动架构**  
   在 **词法与语法分析** 阶段，采用 **DFA + 表驱动** 架构：词法分析器用 DFA 实现复杂 Token 识别(1)，LL 分析器基于预测分析表实现无回溯的自顶向下解析(2)，LR 分析器依托 `ACTION`/`GOTO` 表完成自底向上的移进–规约解析(3)。(1、2、3)

5. **语义分析中的解释执行 + 中间代码生成**  
   在 **语义分析** 阶段，采用了解释器 + 中间代码生成并行的方式，实现了变量类型自动提升，并支持中间代码和符号表快照导出。(4)

6. **多层次错误恢复机制**  
   在错误处理环节中，设计了多层次错误恢复机制：语法层采用**恐慌模式**、**虚拟插入**与**前瞻验证**(2、3)；语义层实现了**类型检查**与**运行时错误检测**(4)。(2、3、4)

7. **精确行号追踪与错误定位**  
   在 **错误定位** 环节中，设计了精确行号追踪机制，在存在空行或跨行语句时仍能将错误定位到实际出错语句行。(2、3)

8. **自动化测试流水线**  
   在 **测试与验证** 中，构造了覆盖正确和多类错误用例的测试集，编写了自动化测试流水线，实现四个模块脚本一键编译、批量测试和结果归档。(1、2、3、4)

9. **模块化架构与可扩展流水线**  
   在项目架构上，采用高内聚低耦合的模块化结构：LR 分析表形成**生产者–消费者模式**；可视化模块独立封装，通过统一接口串联成可扩展流水线。(1、2、3、4)

10. **项目管理与版本控制**  
    在 **项目管理** 中，使用 Git 托管到 GitHub 仓库，便于版本管理（<https://github.com/ying-2626/Principles-of-Compilation>）。(1、2、3、4)
