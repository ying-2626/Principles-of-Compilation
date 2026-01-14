// C语言词法分析器
#include <cstdio>
#include <cstring>
#include <iostream>
#include <map>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
using namespace std;
/* 标准输入函数 - 改为支持流输入 */
void read_prog(string &prog, istream &in)
{
    char c;
    while (in.get(c))
    {
        prog += c;
    }
}

/* 你可以添加其他函数 */
// 定义关键字映射表
map<string, int> keywords = {
    {"auto", 1}, {"break", 2}, {"case", 3}, {"char", 4}, {"const", 5}, {"continue", 6}, {"default", 7}, {"do", 8}, {"double", 9}, {"else", 10}, {"enum", 11}, {"extern", 12}, {"float", 13}, {"for", 14}, {"goto", 15}, {"if", 16}, {"int", 17}, {"long", 18}, {"register", 19}, {"return", 20}, {"short", 21}, {"signed", 22}, {"sizeof", 23}, {"static", 24}, {"struct", 25}, {"switch", 26}, {"typedef", 27}, {"union", 28}, {"unsigned", 29}, {"void", 30}, {"volatile", 31}, {"while", 32}};

// 定义运算符和界符映射表 按c_keys.txt文件
map<string, int> operators = {
    // 单字符运算符
    {"-", 33},
    {"!", 37},
    {"%", 39},
    {"&", 41},
    {"(", 44},
    {")", 45},
    {"*", 46},
    {",", 48},
    {".", 49},
    {"/", 50},
    {":", 52},
    {";", 53},
    {"?", 54},
    {"[", 55},
    {"]", 56},
    {"^", 57},
    {"{", 59},
    {"|", 60},
    {"}", 63},
    {"~", 64},
    {"+", 65},
    {"<", 68},
    {"=", 72},
    {">", 74},
    {"\"", 78},
    // 双字符运算符
    {"--", 34},
    {"-=", 35},
    {"->", 36},
    {"!=", 38},
    {"%=", 40},
    {"&&", 42},
    {"&=", 43},
    {"*=", 47},
    {"/=", 51},
    {"^=", 58},
    {"||", 61},
    {"|=", 62},
    {"++", 66},
    {"+=", 67},
    {"<<", 69},
    {"<<=", 70},
    {"<=", 71},
    {"==", 73},
    {">=", 75},
    {">>", 76},
    {">>=", 77}};

int tokenCount = 0;

// 输出token的函数
void printToken(const string &name, int code)
{
    tokenCount++;
    cout << tokenCount << ": <" << name << "," << code << ">" << endl;
}

// 判断是否为字母
bool isLetter(char c)
{
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

// 判断是否为数字
bool isDigit(char c)
{
    return c >= '0' && c <= '9';
}

// 判断是否为空白字符
bool isWhitespace(char c)
{
    return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

// 获取运算符，处理多字符运算符
string getOperator(string prog, int &pos)
{
    int n = prog.length();
    if (pos >= n)
        return "";

    char c1 = prog[pos];
    char c2 = (pos + 1 < n) ? prog[pos + 1] : '\0';
    char c3 = (pos + 2 < n) ? prog[pos + 2] : '\0';

    // 处理三字符运算符 <<= 和 >>=
    if (c1 == '<' && c2 == '<' && c3 == '=')
    {
        pos += 3;
        return "<<=";
    }
    if (c1 == '>' && c2 == '>' && c3 == '=')
    {
        pos += 3;
        return ">>=";
    }

    // 处理双字符运算符
    string twoChar = string(1, c1) + c2;
    if (operators.find(twoChar) != operators.end())
    {
        pos += 2;
        return twoChar;
    }

    // 单字符运算符
    string oneChar = string(1, c1);
    if (operators.find(oneChar) != operators.end())
    {
        pos += 1;
        return oneChar;
    }

    return "";
}

void Analysis(istream &in = cin)
{
    string prog;
    read_prog(prog, in);

    int i = 0;
    int n = prog.length();

    while (i < n)
    {
        // 跳过空白字符
        if (isWhitespace(prog[i]))
        {
            i++;
            continue;
        }

        // 处理//注释
        if (i + 1 < n && prog[i] == '/' && prog[i + 1] == '/')
        {
            int start = i;
            i += 2; // 跳过"//"
            // 获取完整的注释内容
            while (i < n && prog[i] != '\n')
            {
                i++;
            }
            string comment = prog.substr(start, i - start);
            // 根据题目要求，//注释映射到编号79
            printToken(comment, 79);
            if (i < n && prog[i] == '\n')
            {
                i++;
            }
            continue;
        }

        // 处理/* */注释
        if (i + 1 < n && prog[i] == '/' && prog[i + 1] == '*')
        {
            int start = i;
            i += 2; // 跳过"/*"
            // 找到注释结束
            while (i + 1 < n && !(prog[i] == '*' && prog[i + 1] == '/'))
            {
                i++;
            }
            if (i + 1 < n)
            {
                i += 2; // 跳过"*/"
            }
            string comment = prog.substr(start, i - start);
            // /*注释映射到编号79
            printToken(comment, 79);
            continue;
        }

        // 处理标识符或关键字
        if (isLetter(prog[i]))
        {
            int start = i;
            while (i < n && (isLetter(prog[i]) || isDigit(prog[i])))
            {
                i++;
            }
            string token = prog.substr(start, i - start);

            if (keywords.find(token) != keywords.end())
            {
                printToken(token, keywords[token]);
            }
            else
            {
                printToken(token, 81);
            }
            continue;
        }

        // 处理数字
        if (isDigit(prog[i]))
        {
            int start = i;
            while (i < n && isDigit(prog[i]))
            {
                i++;
            }
            // 处理浮点数
            if (i < n && prog[i] == '.')
            {
                i++;
                while (i < n && isDigit(prog[i]))
                {
                    i++;
                }
            }
            // 处理科学计数法
            if (i < n && (prog[i] == 'e' || prog[i] == 'E'))
            {
                i++;
                if (i < n && (prog[i] == '+' || prog[i] == '-'))
                {
                    i++;
                }
                while (i < n && isDigit(prog[i]))
                {
                    i++;
                }
            }
            // 处理后缀
            if (i < n && (prog[i] == 'f' || prog[i] == 'F' ||
                          prog[i] == 'l' || prog[i] == 'L'))
            {
                i++;
            }

            string token = prog.substr(start, i - start);
            printToken(token, 80);
            continue;
        }

        // 处理字符串常量
        if (prog[i] == '"')
        {
            // 输出开引号
            printToken("\"", 78);
            i++;

            int start = i;
            while (i < n && prog[i] != '"')
            {
                if (prog[i] == '\\' && i + 1 < n)
                {
                    i += 2; // 跳过转义字符
                }
                else
                {
                    i++;
                }
            }

            // 输出字符串内容
            if (start < i)
            {
                string strContent = prog.substr(start, i - start);
                if (!strContent.empty())
                {
                    printToken(strContent, 81);
                }
            }

            // 输出闭引号
            if (i < n && prog[i] == '"')
            {
                i++;
                printToken("\"", 78);
            }
            continue;
        }

        // 处理字符常量
        if (prog[i] == '\'')
        {
            // 开单引号
            printToken("'", 77);
            i++;

            int start = i;
            if (i < n)
            {
                if (prog[i] == '\\' && i + 1 < n)
                {
                    i += 2; // 处理转义字符
                }
                else
                {
                    i++;
                }
            }

            // 字符内容
            if (start < i)
            {
                string charContent = prog.substr(start, i - start);
                if (!charContent.empty())
                {
                    printToken(charContent, 81);
                }
            }

            // 闭单引号
            if (i < n && prog[i] == '\'')
            {
                i++;
                printToken("'", 77);
            }
            continue;
        }

        // 处理运算符和界符
        string op = getOperator(prog, i);
        if (!op.empty())
        {
            printToken(op, operators[op]);
            continue;
        }

        // 如果都不匹配，可能是未知字符，跳过
        i++;
    }
}
