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

// 输出 token 的函数
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

/* 基于 DFA 表驱动的词法分析器 */
const int MAX_STATES = 500;
const int ASCII_SIZE = 256;

struct DFA
{
    int trans[MAX_STATES][ASCII_SIZE];
    int accept[MAX_STATES]; // 0 表示非接收态，>0 表示某类记号类别
    int state_count;

    void init()
    {
        memset(trans, -1, sizeof(trans));
        memset(accept, 0, sizeof(accept));
        state_count = 1; // 0 号状态作为起始状态
    }

    int add_state()
    {
        return state_count++;
    }

    void add_transition(int from, char c, int to)
    {
        trans[from][(unsigned char)c] = to;
    }
} dfa;

// DFA 中的记号类别标签
const int CAT_ID = 1;
const int CAT_NUM = 2;
const int CAT_STR = 3;
const int CAT_CHAR = 4;
const int CAT_OP = 5;
const int CAT_COMMENT_LINE = 6;
const int CAT_COMMENT_BLOCK = 7;
const int CAT_WHITESPACE = 8;

void init_dfa()
{
    dfa.init();

    // State 0: Start
    int s_start = 0;

    // 1. 空白字符
    int s_ws = dfa.add_state();
    dfa.accept[s_ws] = CAT_WHITESPACE;
    string ws = " \t\n\r";
    for (char c : ws)
    {
        dfa.add_transition(s_start, c, s_ws);
        dfa.add_transition(s_ws, c, s_ws);
    }

    // 2. 标识符 / 关键字（以字母或下划线开头）
    int s_id = dfa.add_state();
    dfa.accept[s_id] = CAT_ID;
    for (int c = 0; c < 256; c++)
    {
        if (isLetter(c))
            dfa.add_transition(s_start, c, s_id);
        if (isLetter(c) || isDigit(c))
            dfa.add_transition(s_id, c, s_id);
    }

    // 3. 数字（以数字开头）
    int s_num = dfa.add_state();
    dfa.accept[s_num] = CAT_NUM;
    for (int c = 0; c < 256; c++)
    {
        if (isDigit(c))
        {
            dfa.add_transition(s_start, c, s_num);
            dfa.add_transition(s_num, c, s_num);
        }
    }
    // 浮点数与科学计数法使用更多状态（这里采用适度精简但足够鲁棒的方案）
    // 为浮点数添加小数点状态
    int s_dot = dfa.add_state();
    dfa.add_transition(s_num, '.', s_dot);
    int s_float = dfa.add_state();
    dfa.accept[s_float] = CAT_NUM;
    for (int c = 0; c < 256; c++)
        if (isDigit(c))
            dfa.add_transition(s_dot, c, s_float);
    for (int c = 0; c < 256; c++)
        if (isDigit(c))
            dfa.add_transition(s_float, c, s_float);

    // 科学计数法（e/E）
    int s_sci = dfa.add_state();
    int s_sci_sign = dfa.add_state();
    int s_sci_val = dfa.add_state();
    dfa.accept[s_sci_val] = CAT_NUM;

    dfa.add_transition(s_num, 'e', s_sci);
    dfa.add_transition(s_num, 'E', s_sci);
    dfa.add_transition(s_float, 'e', s_sci);
    dfa.add_transition(s_float, 'E', s_sci);

    dfa.add_transition(s_sci, '+', s_sci_sign);
    dfa.add_transition(s_sci, '-', s_sci_sign);
    for (int c = 0; c < 256; c++)
        if (isDigit(c))
        {
            dfa.add_transition(s_sci, c, s_sci_val);
            dfa.add_transition(s_sci_sign, c, s_sci_val);
            dfa.add_transition(s_sci_val, c, s_sci_val);
        }
    // 数值后缀（f、l 等）
    int s_suffix = dfa.add_state();
    dfa.accept[s_suffix] = CAT_NUM;
    string suffix = "flFL";
    for (char c : suffix)
    {
        dfa.add_transition(s_num, c, s_suffix);
        dfa.add_transition(s_float, c, s_suffix);
        dfa.add_transition(s_sci_val, c, s_suffix);
    }

    // 4. 字符串（以双引号开头）
    int s_str_start = dfa.add_state();
    int s_str_content = dfa.add_state();
    int s_str_esc = dfa.add_state();
    int s_str_end = dfa.add_state();
    dfa.accept[s_str_end] = CAT_STR; // 仅在遇到闭合引号时视为接收态

    dfa.add_transition(s_start, '"', s_str_start); // 起始引号
    // 从起始引号进入内容或空串直接闭合
    dfa.add_transition(s_str_start, '"', s_str_end); // 空串形式
    for (int c = 0; c < 256; c++)
    {
        if (c != '"' && c != '\\')
            dfa.add_transition(s_str_start, c, s_str_content);
    }

    // 字符串内容部分
    for (int c = 0; c < 256; c++)
    {
        if (c == '"')
            dfa.add_transition(s_str_content, c, s_str_end);
        else if (c == '\\')
            dfa.add_transition(s_str_content, c, s_str_esc);
        else
            dfa.add_transition(s_str_content, c, s_str_content);
    }

    // 转义字符后回到内容状态
    for (int c = 0; c < 256; c++)
        dfa.add_transition(s_str_esc, c, s_str_content);

    // 5. 字符常量（以单引号开头）
    int s_char_start = dfa.add_state();
    int s_char_content = dfa.add_state();
    int s_char_esc = dfa.add_state();
    int s_char_end = dfa.add_state();
    dfa.accept[s_char_end] = CAT_CHAR;

    dfa.add_transition(s_start, '\'', s_char_start);
    for (int c = 0; c < 256; c++)
    {
        if (c != '\'' && c != '\\')
            dfa.add_transition(s_char_start, c, s_char_content);
    }
    dfa.add_transition(s_char_start, '\\', s_char_esc); // 处理转义字符

    for (int c = 0; c < 256; c++)
    {
        if (c == '\'')
            dfa.add_transition(s_char_content, c, s_char_end);
        else
            dfa.add_transition(s_char_content, c, s_char_content); // 按理字符长度应为 1，此处放宽以方便错误恢复
    }
    for (int c = 0; c < 256; c++)
        dfa.add_transition(s_char_esc, c, s_char_content);

    // 6. 注释（以 / 开头）
    int s_slash = dfa.add_state();
    dfa.accept[s_slash] = CAT_OP; // 如果后面不是注释起始，则单独视为运算符 /
    dfa.add_transition(s_start, '/', s_slash);

    // 行注释 //
    int s_line_com = dfa.add_state();
    dfa.accept[s_line_com] = CAT_COMMENT_LINE; // 在遇到换行之前都视为处于注释内部
    dfa.add_transition(s_slash, '/', s_line_com);
    for (int c = 0; c < 256; c++)
    {
        if (c != '\n')
            dfa.add_transition(s_line_com, c, s_line_com);
    }

    // 块注释 /*
    int s_block_com = dfa.add_state();
    int s_block_com_star = dfa.add_state();
    int s_block_com_end = dfa.add_state();
    dfa.accept[s_block_com_end] = CAT_COMMENT_BLOCK;

    dfa.add_transition(s_slash, '*', s_block_com);
    for (int c = 0; c < 256; c++)
    {
        if (c == '*')
            dfa.add_transition(s_block_com, c, s_block_com_star);
        else
            dfa.add_transition(s_block_com, c, s_block_com);
    }

    for (int c = 0; c < 256; c++)
    {
        if (c == '/')
            dfa.add_transition(s_block_com_star, c, s_block_com_end);
        else if (c == '*')
            dfa.add_transition(s_block_com_star, c, s_block_com_star); // 保持在连续 * 的状态
        else
            dfa.add_transition(s_block_com_star, c, s_block_com); // 回到注释内容状态
    }

    // 7. 运算符
    // 为所有运算符在 DFA 中构建类似 Trie 的状态路径
    // 注意：以 / 开头的情形已经由 s_slash 处理，这里只补充其余情况；/=
    // 通过单独逻辑挂接到 s_slash 之后

    for (map<string, int>::const_iterator it = operators.begin(); it != operators.end(); ++it)
    {
        const string &op = it->first;
        int code = it->second;

        if (op[0] == '/')
        {
            // 对 /= 做特殊处理，将其连接到 s_slash 之后
            if (op == "/=")
            {
                int s_diveq = dfa.add_state();
                dfa.accept[s_diveq] = CAT_OP;
                dfa.add_transition(s_slash, '=', s_diveq);
            }
            continue; // 单独的 / 已经在前面处理
        }

        int curr = s_start;
        for (size_t idx = 0; idx < op.size(); ++idx)
        {
            char c = op[idx];
            int next = dfa.trans[curr][(unsigned char)c];
            if (next == -1)
            {
                next = dfa.add_state();
                dfa.add_transition(curr, c, next);
            }
            curr = next;
        }
        dfa.accept[curr] = CAT_OP;
    }
}

void Analysis(istream &in = cin)
{
    init_dfa();

    string prog;
    read_prog(prog, in);

    int i = 0;
    int n = prog.length();

    while (i < n)
    {
        int curr = 0; // Start state
        int last_accept_state = -1;
        int last_accept_pos = -1;
        int p = i;

        // Run DFA
        while (p < n)
        {
            char c = prog[p];
            int next = dfa.trans[curr][(unsigned char)c];

            if (next != -1)
            {
                curr = next;
                if (dfa.accept[curr] > 0)
                {
                    last_accept_state = curr;
                    last_accept_pos = p;
                }
                p++;
            }
            else
            {
                break;
            }
        }

        if (last_accept_state != -1)
        {
            // Token found
            string token = prog.substr(i, last_accept_pos - i + 1);
            int cat = dfa.accept[last_accept_state];

            if (cat == CAT_WHITESPACE)
            {
                // Ignore
            }
            else if (cat == CAT_COMMENT_LINE || cat == CAT_COMMENT_BLOCK)
            {
                printToken(token, 79);
            }
            else if (cat == CAT_ID)
            {
                if (keywords.find(token) != keywords.end())
                {
                    printToken(token, keywords[token]);
                }
                else
                {
                    printToken(token, 81);
                }
            }
            else if (cat == CAT_NUM)
            {
                printToken(token, 80);
            }
            else if (cat == CAT_STR)
            {
                // Original logic output: " then content then "
                printToken("\"", 78);
                string content = token.substr(1, token.length() - 2);
                if (!content.empty())
                    printToken(content, 81);
                printToken("\"", 78);
            }
            else if (cat == CAT_CHAR)
            {
                printToken("'", 77);
                string content = token.substr(1, token.length() - 2);
                if (!content.empty())
                    printToken(content, 81);
                printToken("'", 77);
            }
            else if (cat == CAT_OP)
            {
                printToken(token, operators[token]);
            }

            i = last_accept_pos + 1;
        }
        else
        {
            // No token matched, skip one char (or error)
            i++;
        }
    }
}
