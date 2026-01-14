#include <cstdio>
#include <cstring>
#include <iostream>
#include <map>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <stack>
#include <utility>
#include "../Visualizer.h"

using namespace std;

/* 标准输入函数 - 改为支持流输入 */
void read_prog(string &prog, istream &in)
{
    char c;
    while (in.get(c) && c != '$')
    {
        prog += c;
    }
}

/* Trie树结构，用于快速查找关键字和运算符 */
const int SIGMA_SIZE = 256; // ASCII字符集大小
const int MAXNODE = 50010;  // Trie树最大节点数

struct Trie
{
    int ch[MAXNODE][SIGMA_SIZE]; // 子节点数组
    int val[MAXNODE];            // 节点值，存储关键字对应的单字符映射
    int sz;                      // 当前节点数

    // 初始化Trie树
    void init()
    {
        sz = 1;
        memset(ch[0], 0, sizeof(ch[0]));
        memset(val, 0, sizeof(val));
    }

    // 插入字符串到Trie树
    void insert(const char *s, int v)
    {
        int u = 0, n = strlen(s);
        for (int i = 0; i < n; i++)
        {
            int c = s[i];
            if (!ch[u][c])
            {
                memset(ch[sz], 0, sizeof(ch[sz]));
                val[sz] = 0;
                ch[u][c] = sz++;
            }
            u = ch[u][c];
        }
        val[u] = v; // 在字符串末尾节点存储对应的值
    }

    // 查找字符串前缀匹配，返回最长匹配的长度和对应的值
    pair<int, int> query(const string &s, int pos)
    {
        int u = 0;
        int maxLen = 0;
        int matchedVal = 0;

        for (int i = pos; i < s.length(); i++)
        {
            int c = (unsigned char)s[i];
            if (!ch[u][c])
                break;
            u = ch[u][c];
            if (val[u])
            {
                maxLen = i - pos + 1;
                matchedVal = val[u];
            }
        }
        return {maxLen, matchedVal};
    }
} ac; // 全局Trie树实例

/* 初始化关键字和运算符表 */
void init_()
{
    ac.init();

    // 插入映射 (Token -> Single Char Representation)
    // 对应 LL(1) 表中的终结符
    ac.insert("if", 'y');
    ac.insert("else", 'u');
    ac.insert("then", 'v');
    ac.insert("while", 'w');
    ac.insert("ID", '[');
    ac.insert("NUM", ']');
    ac.insert(">=", 'z');
    ac.insert("<=", 'x');
    ac.insert("==", '.');
    ac.insert(">", '!');
}

/* LL(1)语法分析表和相关数据结构 */
map<string, string> a[30]; // LL(1)分析表，索引为非终结符编号
map<string, string> p;     // 符号名称映射表

// 计算非终结符的数组索引
int cal(string x)
{
    return x[0] - 'A'; // 将A-Z映射到0-25
}

// 初始化LL(1)分析表和符号名称映射
void init()
{
    // 初始化LL(1)分析表
    // 格式: a[非终结符索引][向前看符号] = 产生式

    // 起始符号A
    a[cal("A")]["{"] = "B";

    // 复合语句B
    a[cal("B")]["{"] = "{G}";

    // 语句C
    a[cal("C")]["w"] = "E"; // while语句
    a[cal("C")]["y"] = "D"; // if语句
    a[cal("C")]["{"] = "B"; // 复合语句
    a[cal("C")]["["] = "F"; // 赋值语句

    // if语句D
    a[cal("D")]["y"] = "y(H)vCuC";

    // while语句E
    a[cal("E")]["w"] = "w(H)C";

    // 赋值语句F
    a[cal("F")]["["] = "[=L;";

    // 语句序列G
    a[cal("G")]["w"] = "CG";
    a[cal("G")]["y"] = "CG";
    a[cal("G")]["{"] = "CG";
    a[cal("G")]["}"] = "@"; // 空产生式
    a[cal("G")]["["] = "CG";

    // 布尔表达式H
    a[cal("H")]["("] = "LNL";
    a[cal("H")]["["] = "LNL";
    a[cal("H")]["]"] = "LNL";

    // 算术表达式右部I
    a[cal("I")]["z"] = "@";
    a[cal("I")]["x"] = "@";
    a[cal("I")]["."] = "@";
    a[cal("I")][")"] = "@";
    a[cal("I")]["<"] = "@";
    a[cal("I")]["+"] = "+JI";
    a[cal("I")]["-"] = "-JI";
    a[cal("I")][";"] = "@";
    a[cal("I")]["!"] = "@";
    a[cal("I")]["}"] = "@";

    // 乘法表达式J
    a[cal("J")]["("] = "KM";
    a[cal("J")]["["] = "KM";
    a[cal("J")]["]"] = "KM";

    // 简单表达式K
    a[cal("K")]["("] = "(L)";
    a[cal("K")]["["] = "[";
    a[cal("K")]["]"] = "]";

    // 算术表达式L
    a[cal("L")]["("] = "JI";
    a[cal("L")]["["] = "JI";
    a[cal("L")]["]"] = "JI";

    // 乘法表达式右部M
    a[cal("M")]["z"] = "@";
    a[cal("M")]["x"] = "@";
    a[cal("M")]["."] = "@";
    a[cal("M")][")"] = "@";
    a[cal("M")]["<"] = "@";
    a[cal("M")]["+"] = "@";
    a[cal("M")]["-"] = "@";
    a[cal("M")][";"] = "@";
    a[cal("M")]["!"] = "@";
    a[cal("M")]["/"] = "/KM";
    a[cal("M")]["*"] = "*KM";
    a[cal("M")]["}"] = "@";

    // 布尔运算符N
    a[cal("N")]["z"] = "z";
    a[cal("N")]["x"] = "x";
    a[cal("N")]["."] = ".";
    a[cal("N")]["<"] = "<";
    a[cal("N")]["!"] = "!";

    // 初始化符号名称映射
    // 将内部符号映射为可读的名称
    p["A"] = "program";
    p["B"] = "compoundstmt";
    p["C"] = "stmt";
    p["D"] = "ifstmt";
    p["E"] = "whilestmt";
    p["F"] = "assgstmt";
    p["G"] = "stmts";
    p["H"] = "boolexpr";
    p["I"] = "arithexprprime";
    p["J"] = "multexpr";
    p["K"] = "simpleexpr";
    p["L"] = "arithexpr";
    p["M"] = "multexprprime";
    p["N"] = "boolop";

    // 运算符映射
    p["!"] = ">";
    p["["] = "ID";
    p["]"] = "NUM";
    p["."] = "==";
    p["z"] = ">=";
    p["x"] = "<=";
    p["y"] = "if";
    p["u"] = "else";
    p["v"] = "then";
    p["w"] = "while";
}

int line; // 当前行号，用于错误报告

// Global vector to store derivation for visualization
vector<pair<string, int>> llDerivationSeq;

/* 语法分析主函数：构建语法分析树并输出 */
int solve(string s, int &cur, int l, int d)
{
    int len = s.length();
    int i = l;

    // 跳过空白字符和换行符
    while (1)
    {
        if (s[i] == '\n')
        {
            line++;
            i++;
        }
        else if (s[i] == ' ' || s[i] == '\t' || s[i] == '\r' || s[i] == '\v')
        {
            i++;
        }
        else
        {
            break;
        }
    }

    // 获取向前看符号
    string x = "";

    // 使用Trie树查找
    pair<int, int> match = ac.query(s, i);
    if (match.first > 0)
    {
        char mappedChar = (char)match.second;
        string key(1, mappedChar);
        x = a[cur][key];
    }
    else
    {
        // 单字符查找
        x += s[i];
        x = a[cur][x];
    }

    // 处理产生式
    for (int j = 0; j < x.length(); j++)
    {
        if (x[j] >= 'A' && x[j] <= 'Z')
        { // 非终结符
            // 打印缩进和符号名称
            for (int k = 1; k <= d; k++)
                putchar('\t');
            string y = "";
            y += x[j];
            cout << p[y] << endl;

            // Record for visualization
            llDerivationSeq.push_back({p[y], d});

            // 递归处理非终结符
            cur = cal(y);
            i = solve(s, cur, i, d + 1);
        }
        else if (x[j] == '@')
        { // 空产生式
            for (int k = 1; k <= d; k++)
                putchar('\t');
            puts("E");

            // Record for visualization
            llDerivationSeq.push_back({"E", d});
        }
        else
        { // 终结符
            for (int k = 1; k <= d; k++)
                putchar('\t');
            string y = "";
            y += x[j];
            string mappedName = y;

            // 将内部符号映射为实际符号
            if (x[j] == '!' || x[j] == '[' || x[j] == ']' || x[j] == '.' ||
                x[j] == 'z' || x[j] == 'x' || x[j] == 'y' ||
                x[j] == 'u' || x[j] == 'v' || x[j] == 'w')
            {
                mappedName = p[y];
            }

            // Record for visualization
            llDerivationSeq.push_back({mappedName, d});

            // 匹配输入中的终结符
            int flag = 0;
            for (int k = 0; k < mappedName.length(); k++)
            {
                // 跳过空白字符
                while (1)
                {
                    if (s[i] == '\n')
                    {
                        line++;
                        i++;
                    }
                    else if (s[i] == ' ' || s[i] == '\n' || s[i] == '\t' || s[i] == '\r' || s[i] == '\v')
                    {
                        i++;
                    }
                    else
                    {
                        break;
                    }
                }

                // 检查是否匹配
                if (s[i] != mappedName[k])
                {
                    flag = 1;
                    i--;
                }
                i++;
            }

            // 输出终结符
            cout << mappedName << endl;
        }
    }

    return i;
}

/* 语法错误检查函数：检查输入是否符合文法，报告错误但不输出语法树 */
int error(string s, int &cur, int l, int d)
{
    int len = s.length();
    int i = l;

    // 跳过空白字符
    while (1)
    {
        if (s[i] == '\n')
        {
            line++;
            i++;
        }
        else if (s[i] == ' ' || s[i] == '\t' || s[i] == '\r' || s[i] == '\v')
        {
            i++;
        }
        else
        {
            break;
        }
    }

    // 获取向前看符号
    string x = "";

    // 使用Trie树查找
    pair<int, int> match = ac.query(s, i);
    if (match.first > 0)
    {
        char mappedChar = (char)match.second;
        string key(1, mappedChar);
        x = a[cur][key];
    }
    else
    {
        // 单字符查找
        x += s[i];
        x = a[cur][x];
    }

    // 处理产生式
    for (int j = 0; j < x.length(); j++)
    {
        if (x[j] >= 'A' && x[j] <= 'Z')
        { // 非终结符
            string y = "";
            y += x[j];
            cur = cal(y);
            i = error(s, cur, i, d + 1);
        }
        else if (x[j] == '@')
        { // 空产生式，不处理
        }
        else
        { // 终结符
            string y = "";
            y += x[j];
            string mappedName = y;

            // 映射内部符号
            if (x[j] == '!' || x[j] == '[' || x[j] == ']' || x[j] == '.' ||
                x[j] == 'z' || x[j] == 'x' || x[j] == 'y' ||
                x[j] == 'u' || x[j] == 'v' || x[j] == 'w')
            {
                mappedName = p[y];
            }

            // 匹配终结符
            int flag = 0;
            for (int k = 0; k < mappedName.length(); k++)
            {
                // 跳过空白字符
                while (1)
                {
                    if (s[i] == '\n')
                    {
                        line++;
                        i++;
                    }
                    else if (s[i] == ' ' || s[i] == '\n' || s[i] == '\t' || s[i] == '\r' || s[i] == '\v')
                    {
                        i++;
                    }
                    else
                    {
                        break;
                    }
                }

                // 检查是否匹配
                if (s[i] != mappedName[k])
                {
                    flag = 1;
                    i--;
                }
                i++;
            }

            // 报告语法错误
            if (flag == 1)
            {
                printf("语法错误,第%d行,缺少\"", line);
                cout << mappedName;
                printf("\"\n");
            }
        }
    }

    return i;
}

/* 主分析函数：执行词法分析和语法分析 */
void Analysis(istream &in = cin)
{
    string prog;
    read_prog(prog, in);

    /********* Begin *********/
    // 初始化LL(1)分析表
    init();
    // 初始化Trie
    init_();

    int len = prog.length();
    int cur = cal("A"); // 起始符号A
    line = 0;

    // 第一遍：检查语法错误
    for (int i = 0; i < len;)
    {
        i = error(prog, cur, i, 1);
    }

    // 第二遍：构建并输出语法分析树
    llDerivationSeq.clear();
    llDerivationSeq.push_back({"program", 0}); // Root

    cout << "program => " << endl; // 输出起始符号
    cur = cal("A");                // 重置为起始符号
    for (int i = 0; i < len;)
    {
        i = solve(prog, cur, i, 1);
    }

    // Generate DOT file
    Visualizer::generateDOT("ll_tree.dot", llDerivationSeq);

    /********* End *********/
}
