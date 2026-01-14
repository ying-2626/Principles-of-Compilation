// C语言 LR(0)/SLR 分析器 - 基于自动生成的分析表
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

// 全局类型定义
typedef pair<string, int> TokenInfo;

// 树节点定义
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

// 全局数据结构
struct ParserData
{
    map<string, char> tokenToChar; // "if" -> 'y'
    map<char, string> charToToken; // 'y' -> "if"
    vector<TokenInfo> derivationSeq;
    int lineNum = 0;
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

/* 初始化映射 */
void initMappings(ParserData &data)
{
    // 初始化 token 到 单字符 的映射 (用于查表)
    data.tokenToChar["program"] = 'A'; // Start symbol in grammar is 'A' (S->A)
    // Terminals
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

    // Reverse map for display
    for (auto it = data.tokenToChar.begin(); it != data.tokenToChar.end(); ++it)
    {
        data.charToToken[it->second] = it->first;
    }

    // Non-terminals names (Optional, for display)
    data.charToToken['S'] = "program"; // Augmented start
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

/* 跳过空白字符 */
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

/* 获取下一个Token对应的单字符 */
char getNextTokenChar(const string &source, int pos, ParserData &data)
{
    if (pos >= source.length())
        return '#'; // End

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
        return '!';

    return c;
}

/* 消费Token，返回新的pos */
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

/* 将AST转化为推导序列 (Pre-order) */
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

/* 主分析函数 */
void Analysis(istream &in = cin)
{
    string prog;
    read_prog(prog, in);

    // 初始化表和映射
    initLRTable();
    ParserData data;
    initMappings(data);

    stack<int> stateStack;
    stateStack.push(0);

    stack<ASTNode *> nodeStack;

    int pos = 0;
    int len = prog.length();
    bool accepted = false;

    cout << "program => " << endl;

    while (true)
    {
        pos = skipWhiteSpaces(prog, pos, data.lineNum);
        char lookahead = getNextTokenChar(prog, pos, data);

        int currentState = stateStack.top();
        Action act = ACTION[currentState][(unsigned char)lookahead];

        if (act.type == 0)
        { // Shift
            // Create leaf node
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

            stateStack.push(act.val);
            pos = consumeToken(prog, pos);
        }
        else if (act.type == 1)
        { // Reduce
            int prodIndex = act.val;
            string lhs = PRODUCTIONS[prodIndex].left;
            string rhs = PRODUCTIONS[prodIndex].right;
            int rhsLen = (rhs == "") ? 0 : rhs.length();

            // Pop items
            vector<ASTNode *> children;
            for (int i = 0; i < rhsLen; i++)
            {
                if (stateStack.size() > 1)
                    stateStack.pop(); // Don't pop initial state 0 if error? No, always valid
                children.push_back(nodeStack.top());
                nodeStack.pop();
            }
            // Children are popped in reverse order
            reverse(children.begin(), children.end());

            // Create new non-terminal node
            string lhsName = lhs;
            if (data.charToToken.count(lhs[0]))
                lhsName = data.charToToken[lhs[0]];

            ASTNode *newNode = new ASTNode(lhsName);
            // If epsilon reduction, add E child?
            if (rhsLen == 0)
            {
                newNode->children.push_back(new ASTNode("E")); // "E" for epsilon/empty in visualization
            }
            else
            {
                newNode->children = children;
            }

            nodeStack.push(newNode);

            // Goto
            int topState = stateStack.top();
            int nextState = GOTO[topState][(unsigned char)lhs[0]];
            if (nextState == -1)
            {
                cout << "GOTO Error at state " << topState << " symbol " << lhs << endl;
                break;
            }
            stateStack.push(nextState);

            // Output step (Optional: print reduction)
            // cout << "Reduce: " << lhs << " -> " << (rhs==""?"@":rhs) << endl;
        }
        else if (act.type == 2)
        { // Accept
            accepted = true;
            break;
        }
        else
        { // Error
            cout << "语法错误，第" << data.lineNum << "行，遇到符号: " << lookahead << endl;
            break;
        }
    }

    if (accepted && !nodeStack.empty())
    {
        ASTNode *root = nodeStack.top();

        // Flatten for Visualizer
        flattenAST(root, 0, data.derivationSeq);

        // Generate output for simple tests (Just root name usually? Or sequence?)
        // Print textual derivation sequence to stdout
        for (size_t i = 1; i < data.derivationSeq.size(); ++i)
        { // Skip root "program"
            const auto &item = data.derivationSeq[i];
            for (int k = 0; k < item.second; k++)
            {
                cout << "\t";
            }
            cout << item.first << endl;
        }

        // We can reuse outputDerivation from original code if we want exact format.
        // But for now, let's just ensure we generate the DOT file.
        Visualizer::generateDOT("lr_tree.dot", data.derivationSeq);

        // Clean up
        delete root;
    }
}