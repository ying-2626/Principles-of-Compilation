// C 语言 LR(0)/SLR 语法分析器 - 基于自动生成的分析表驱动
#include <cstdio>
#include <cstring>
#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <stack>
#include <utility>
#include <fstream>
#include <algorithm>
#include "../Visualizer.h"
#include "LRTable.h" // 包含自动生成的Action/Goto表

using namespace std;

// 全局类型定义：TokenInfo = <符号名, 深度>
typedef pair<string, int> TokenInfo;

// 语法树结点定义
struct ASTNode
{
    string symbol;
    vector<ASTNode *> children;

    ASTNode(string s) : symbol(s) {}

    ~ASTNode()
    {
        for (auto c : children)
            delete c;
    }
};

// 解析过程中用到的全局数据结构
struct ParserData
{
    map<string, char> tokenToChar;   // 词法记号名 → LR 表中使用的单字符编码，例如 "if" -> 'y'
    map<char, string> charToToken;   // 单字符编码 → 可读记号名，例如 'y' -> "if"
    vector<TokenInfo> derivationSeq; // 展平后的推导序列（用于可视化）
    int lineNum = 0;                 // 当前行号（错误报告用）
};

// 函数声明
void initMappings(ParserData &data);
int skipWhiteSpaces(const string &source, int pos, int &lineNum);
char getNextTokenChar(const string &source, int pos, ParserData &data);
void flattenAST(ASTNode *root, int depth, vector<TokenInfo> &seq);

/* 标准输入函数 - 改为支持流输入 */
void read_prog(string &prog, istream &in)
{
    char c;
    while (in.get(c) && c != '$')
    {
        prog += c;
    }
}

/* 初始化终结符/非终结符 与 单字符编码之间的映射关系 */
void initMappings(ParserData &data)
{
    // 初始化 token 到 单字符 的映射（用于在 ACTION / GOTO 表中查表）
    data.tokenToChar["program"] = 'A'; // 文法开始符号 A（增广文法 S->A）
    // 终结符
    data.tokenToChar[">"] = '!';
    data.tokenToChar["ID"] = '[';
    data.tokenToChar["NUM"] = ']';
    data.tokenToChar["=="] = '.';
    data.tokenToChar[">="] = 'z';
    data.tokenToChar["<="] = 'x';
    data.tokenToChar["if"] = 'y';
    data.tokenToChar["else"] = 'u';
    data.tokenToChar["then"] = 'v';
    data.tokenToChar["while"] = 'w';

    // 反向映射：用于将内部单字符还原为可读记号名称
    for (auto it = data.tokenToChar.begin(); it != data.tokenToChar.end(); ++it)
    {
        data.charToToken[it->second] = it->first;
    }

    // 非终结符名称（用于显示，可选）
    data.charToToken['S'] = "program"; // 增广开始符号
    data.charToToken['A'] = "program";
    data.charToToken['B'] = "compoundstmt";
    data.charToToken['C'] = "stmt";
    data.charToToken['D'] = "ifstmt";
    data.charToToken['E'] = "whilestmt";
    data.charToToken['F'] = "assgstmt";
    data.charToToken['G'] = "stmts";
    data.charToToken['H'] = "boolexpr";
    data.charToToken['I'] = "arithexprprime";
    data.charToToken['J'] = "multexpr";
    data.charToToken['K'] = "simpleexpr";
    data.charToToken['L'] = "arithexpr";
    data.charToToken['M'] = "multexprprime";
    data.charToToken['N'] = "boolop";
}

/* 跳过空白字符，更新行号，返回下一个非空白字符的位置 */
int skipWhiteSpaces(const string &source, int pos, int &lineNum)
{
    int i = pos;
    int len = source.length();

    while (i < len)
    {
        if (source[i] == '\n')
        {
            lineNum++;
            i++;
        }
        else if (source[i] == ' ' || source[i] == '\t' ||
                 source[i] == '\r' || source[i] == '\v')
        {
            i++;
        }
        else
        {
            break;
        }
    }
    return i;
}

/* 获取当前位置的下一个 Token 在 LR 表中对应的单字符编码 */
char getNextTokenChar(const string &source, int pos, ParserData &data)
{
    if (pos >= source.length())
        return '#'; // 输入结束标记

    if (pos + 1 < source.length())
    {
        if (source[pos] == 'I' && source[pos + 1] == 'D')
            return '[';
        if (source[pos] == '>' && source[pos + 1] == '=')
            return 'z';
        if (source[pos] == '<' && source[pos + 1] == '=')
            return 'x';
        if (source[pos] == '=' && source[pos + 1] == '=')
            return '.';
        if (source[pos] == 'i' && source[pos + 1] == 'f')
            return 'y';
        if (source[pos] == 't' && source[pos + 1] == 'h')
            return 'v'; // then
        if (source[pos] == 'e' && source[pos + 1] == 'l')
            return 'u'; // else
        if (source[pos] == 'w' && source[pos + 1] == 'h')
            return 'w'; // while
    }
    if (pos + 2 < source.length())
    {
        if (source[pos] == 'N' && source[pos + 1] == 'U' && source[pos + 2] == 'M')
            return ']';
    }

    char c = source[pos];
    if (c == '>')
        return '!'; // > 映射为内部编码 '!'

    return c;
}

/* 消费一个 Token，返回消费之后的新位置（与 getNextTokenChar 逻辑保持一致） */
int consumeToken(const string &source, int pos)
{
    if (pos >= source.length())
        return pos;

    if (pos + 1 < source.length())
    {
        if (source[pos] == 'I' && source[pos + 1] == 'D')
            return pos + 2;
        if (source[pos] == '>' && source[pos + 1] == '=')
            return pos + 2;
        if (source[pos] == '<' && source[pos + 1] == '=')
            return pos + 2;
        if (source[pos] == '=' && source[pos + 1] == '=')
            return pos + 2;
        if (source[pos] == 'i' && source[pos + 1] == 'f')
            return pos + 2;
        if (source[pos] == 't' && source[pos + 1] == 'h')
            return pos + 4; // then
        if (source[pos] == 'e' && source[pos + 1] == 'l')
            return pos + 4; // else
        if (source[pos] == 'w' && source[pos + 1] == 'h')
            return pos + 5; // while
    }
    if (pos + 2 < source.length())
    {
        if (source[pos] == 'N' && source[pos + 1] == 'U' && source[pos + 2] == 'M')
            return pos + 3;
    }

    return pos + 1;
}

/* 将语法树按先序遍历展开为推导序列，记录 (符号名, 深度) */
void flattenAST(ASTNode *root, int depth, vector<TokenInfo> &seq)
{
    if (!root)
        return;
    seq.push_back({root->symbol, depth});
    for (auto child : root->children)
    {
        flattenAST(child, depth + 1, seq);
    }
}

/* 主分析函数：基于 LR 分析表进行移进-规约并同时构造语法树 */
void Analysis(istream &in = cin)
{
    string prog;
    read_prog(prog, in);

    // 初始化 LR 表和符号映射
    initLRTable();
    ParserData data;
    initMappings(data);
    data.lineNum = 1;

    stack<int> stateStack; // 状态栈
    stateStack.push(0);

    stack<ASTNode *> nodeStack; // 语法树结点栈

    int pos = 0;
    int len = prog.length();
    bool accepted = false; // 标记是否成功接受输入

    cout << "program => " << endl; // 与 LL 版本保持一致的输出格式起始行

    char virtualToken = 0;         // 虚拟插入的符号（0 表示无）
    int lastAcceptedTokenLine = 1; // 上一个成功移进的 Token 所在的行号

    while (true)
    {
        // 先跳过空白，得到当前向前看符号
        int prevLine = data.lineNum; // 记录跳过空白前的行号
        pos = skipWhiteSpaces(prog, pos, data.lineNum);
        char lookahead;
        if (virtualToken != 0)
        {
            lookahead = virtualToken;
        }
        else
        {
            lookahead = getNextTokenChar(prog, pos, data);
        }

        int currentState = stateStack.top();
        Action act = ACTION[currentState][(unsigned char)lookahead]; // 查 ACTION 表，决定当前动作

        // 错误处理与恢复
        if (act.type != 0 && act.type != 1 && act.type != 2)
        {
            // 尝试虚拟插入恢复
            // 调整优先级：优先尝试闭括号，再尝试分号，以避免在 while(...) 中误报缺少分号
            char candidates[] = {')', ']', '}', ';'};
            bool recovered = false;
            for (char c : candidates)
            {
                Action recAct = ACTION[currentState][(unsigned char)c];
                if (recAct.type == 0 || recAct.type == 1 || recAct.type == 2)
                {
                    int reportLine = (data.lineNum > lastAcceptedTokenLine) ? lastAcceptedTokenLine : data.lineNum;
                    cout << "语法错误，第" << reportLine << "行，缺少符号: '" << c << "'" << endl;
                    act = recAct;
                    virtualToken = c; // 设置虚拟符号，后续循环将持续使用直到移进
                    lookahead = c;
                    recovered = true;
                    break;
                }
            }

            if (!recovered)
            {
                // 恐慌模式：跳过当前非法符号
                cout << "语法错误，第" << data.lineNum << "行，遇到非法符号: " << lookahead << " (跳过)" << endl;
                pos = consumeToken(prog, pos);
                continue;
            }
        }

        if (act.type == 0)
        { // 移进（shift）
            // 为当前读入的终结符创建叶子结点
            string tokenStr;
            if (data.charToToken.count(lookahead))
            {
                tokenStr = data.charToToken[lookahead];
            }
            else
            {
                tokenStr = string(1, lookahead);
            }

            ASTNode *leaf = new ASTNode(tokenStr);
            nodeStack.push(leaf);

            stateStack.push(act.val); // 压入新的自动机状态

            if (virtualToken != 0)
            {
                virtualToken = 0; // 虚拟符号已被移进消费
                // 虚拟符号虽然没有实际对应的 Token，但逻辑上它属于当前上下文
                // 我们可以选择更新 lastAcceptedTokenLine，也可以不更新
                // 这里选择不更新，因为它不是输入文件中的真实行
            }
            else
            {
                // 成功移进真实 Token，更新行号
                // 注意：data.lineNum 是 lookahead 的行号（即当前移进 Token 的行号）
                lastAcceptedTokenLine = data.lineNum;
                pos = consumeToken(prog, pos); // 消费一个输入记号
            }
        }
        else if (act.type == 1)
        { // 规约（reduce）
            int prodIndex = act.val;
            string lhs = PRODUCTIONS[prodIndex].left;
            string rhs = PRODUCTIONS[prodIndex].right;
            int rhsLen = (rhs == "") ? 0 : rhs.length(); // 右部长度（空产生式长度为 0）

            // 从状态栈和结点栈中弹出与右部对应的元素
            vector<ASTNode *> children;
            for (int i = 0; i < rhsLen; i++)
            {
                if (stateStack.size() > 1)
                    stateStack.pop(); // 始终保留栈底初始状态 0
                children.push_back(nodeStack.top());
                nodeStack.pop();
            }
            // 弹栈顺序与文法右部顺序相反，这里再反转一次
            reverse(children.begin(), children.end());

            // 创建新的非终结符结点
            string lhsName = lhs;
            if (data.charToToken.count(lhs[0]))
                lhsName = data.charToToken[lhs[0]];

            ASTNode *newNode = new ASTNode(lhsName);
            // 如果是对空产生式的规约，在树上加入一个 "E" 结点用于可视化
            if (rhsLen == 0)
            {
                newNode->children.push_back(new ASTNode("E")); // "E" for epsilon/empty in visualization
            }
            else
            {
                newNode->children = children;
            }

            nodeStack.push(newNode);

            // 根据当前栈顶状态和规约后的非终结符，查询 GOTO 表
            int topState = stateStack.top();
            int nextState = GOTO[topState][(unsigned char)lhs[0]];
            if (nextState == -1)
            {
                cout << "GOTO 错误：状态 " << topState << " 对符号 " << lhs << " 无定义" << endl;
                break;
            }
            stateStack.push(nextState);

            // 如需调试，可在此输出当前规约步骤
            // cout << "Reduce: " << lhs << " -> " << (rhs==""?"@":rhs) << endl;
        }
        else if (act.type == 2)
        { // 接受（accept）
            accepted = true;
            break;
        }
    }

    if (accepted && !nodeStack.empty())
    {
        ASTNode *root = nodeStack.top();

        // 将语法树展平为 <符号名, 深度> 序列，交给可视化模块
        flattenAST(root, 0, data.derivationSeq);

        // 将推导序列用缩进形式打印到标准输出，便于简单调试和对比
        for (size_t i = 1; i < data.derivationSeq.size(); ++i)
        { // 跳过根结点 "program" 本身，只打印其子树
            const auto &item = data.derivationSeq[i];
            for (int k = 0; k < item.second; k++)
            {
                cout << "\t";
            }
            cout << item.first << endl;
        }

        // 如需与原 LL 版本完全一致的格式，可复用原有输出函数；
        // 当前版本只保证生成 DOT 文件用于可视化。
        Visualizer::generateDOT("lr_tree.dot", data.derivationSeq);

        // Clean up
        delete root;
    }
}
