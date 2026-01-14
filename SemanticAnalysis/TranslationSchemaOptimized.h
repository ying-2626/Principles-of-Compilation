#ifndef TRANSLATION_SCHEMA_OPTIMIZED_H
#define TRANSLATION_SCHEMA_OPTIMIZED_H

#include <cstdio>
#include <cstring>
#include <iostream>
#include <map>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <cmath>
#include <unordered_map>
#include <iomanip>

using namespace std;

// --- 1. 基础设施：缓冲读取器 ---
class BufferedReader
{
private:
    istream &in;
    static const int BUFFER_SIZE = 4096;
    char buffer[BUFFER_SIZE];
    int pos;
    int size;
    bool eof;

public:
    BufferedReader(istream &input) : in(input), pos(0), size(0), eof(false) {}

    char get()
    {
        if (pos >= size)
        {
            if (eof)
                return 0;
            in.read(buffer, BUFFER_SIZE);
            size = in.gcount();
            pos = 0;
            if (size == 0)
            {
                eof = true;
                return 0;
            }
        }
        return buffer[pos++];
    }

    bool isEOF() const { return eof; }
};

// --- 2. 类型系统与符号表 ---
enum VarType
{
    TYPE_INT,
    TYPE_REAL,
    TYPE_UNKNOWN
};

struct Symbol
{
    string name;
    VarType type;
    union
    {
        int iVal;
        double rVal;
    };

    Symbol() : type(TYPE_UNKNOWN), iVal(0) {}
};

class SymbolTable
{
public:
    unordered_map<string, Symbol> table;

    void setInt(string name, int val)
    {
        Symbol &sym = table[name];
        sym.name = name;
        sym.type = TYPE_INT;
        sym.iVal = val;
    }

    void setReal(string name, double val)
    {
        Symbol &sym = table[name];
        sym.name = name;
        sym.type = TYPE_REAL;
        sym.rVal = val;
    }

    Symbol get(string name)
    {
        if (table.find(name) != table.end())
        {
            return table[name];
        }
        return Symbol(); // 未知
    }

    void dumpSnapshot(const string &filename)
    {
        ofstream out(filename);
        out << "Name,Type,Value" << endl;
        // 排序以保持输出一致性
        map<string, Symbol> sorted(table.begin(), table.end());
        for (auto &pair : sorted)
        {
            out << pair.first << ",";
            if (pair.second.type == TYPE_INT)
            {
                out << "int," << pair.second.iVal;
            }
            else if (pair.second.type == TYPE_REAL)
            {
                out << "real," << pair.second.rVal;
            }
            out << endl;
        }
        out.close();
    }
};

// --- 3. 中间代码生成 ---
struct Quadruple
{
    string op;
    string arg1;
    string arg2;
    string result;
};

class IRGenerator
{
public:
    vector<Quadruple> code;

    void emit(string op, string arg1, string arg2, string result)
    {
        code.push_back({op, arg1, arg2, result});
    }

    void dumpIR(const string &filename)
    {
        ofstream out(filename);
        for (int i = 0; i < code.size(); i++)
        {
            out << i << ": " << code[i].op << " " << code[i].arg1 << ", " << code[i].arg2 << ", " << code[i].result << endl;
        }
        out.close();
    }
};

// 全局上下文
BufferedReader *reader;
SymbolTable symTable;
IRGenerator irGen;
char next_char = 0;

char get_char()
{
    if (next_char != 0)
    {
        char c = next_char;
        next_char = 0;
        return c;
    }
    return reader->get();
}

void unget_char(char c)
{
    next_char = c;
}

// 优化的读取函数，跳过无效字符（与原始版本一致）
int read_safe(char *s)
{
    char c = get_char();
    // 跳过任何非允许集中的字符，换行符和 $ 除外
    // 允许: a-z, 0-9, ., >, <, =, !, +, -, *, /
    while (true)
    {
        if (c == 0)
            return 0;
        if (c == '$')
            return 0;
        if (c == '\n')
            return -1;

        bool allowed = (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') ||
                       c == '.' || c == '>' || c == '<' || c == '=' ||
                       c == '!' || c == '+' || c == '-' || c == '*' || c == '/';

        if (allowed)
            break;
        c = get_char();
    }

    int len = 0;
    while (true)
    {
        bool allowed = (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') ||
                       c == '.' || c == '>' || c == '<' || c == '=' ||
                       c == '!' || c == '+' || c == '-' || c == '*' || c == '/';
        if (!allowed)
            break;

        s[len++] = c;
        c = get_char();
    }
    unget_char(c); // 放回分隔符

    s[len] = '\0';
    return len;
}

double parseValue(char *s)
{
    if (isdigit(s[0]))
    {
        return atof(s);
    }
    Symbol sym = symTable.get(s);
    if (sym.type == TYPE_INT)
        return (double)sym.iVal;
    if (sym.type == TYPE_REAL)
        return sym.rVal;
    return 0;
}

int flag_err = 1;

// 前向声明
void solve_opt(int line, bool execute);

// 检查/跳过语句的辅助函数
void solvecheck_opt(int line)
{
    solve_opt(line, false);
}

// execute=true 表示更新符号表，false 表示仅解析
void solve_opt(int line, bool execute)
{
    char s[100];
    string arg1, arg2, op, result;

    // 1. 结果变量
    int t = read_safe(s);
    result = s;

    read_safe(s); // =

    // 2. 操作数 1
    read_safe(s);
    arg1 = s;
    double val1 = parseValue(s);

    // 3. 运算符或结束
    read_safe(s);
    if (strlen(s) == 0 || s[0] == '\n')
    {
    }

    bool is_op = (s[0] == '+' || s[0] == '-' || s[0] == '*' || s[0] == '/');

    if (!is_op)
    {
        // 赋值: result = arg1
        if (execute)
        {
            Symbol sym = symTable.get(result);
            if (sym.type == TYPE_INT || sym.type == TYPE_UNKNOWN)
            {
                // 推导
                bool is_int = true;
                for (char c : arg1)
                    if (c == '.')
                        is_int = false;
                if (is_int)
                    symTable.setInt(result, (int)val1);
                else
                    symTable.setReal(result, val1);
            }
            else
            {
                if (sym.type == TYPE_INT)
                    symTable.setInt(result, (int)val1);
                else
                    symTable.setReal(result, val1);
            }
            irGen.emit("=", arg1, "", result);
        }
    }
    else
    {
        op = s;
        read_safe(s); // arg2
        arg2 = s;
        double val2 = parseValue(s);

        if (execute)
        {
            double res = 0;
            if (op == "+")
                res = val1 + val2;
            else if (op == "-")
                res = val1 - val2;
            else if (op == "*")
                res = val1 * val2;
            else if (op == "/")
            {
                if (abs(val2) < 1e-6)
                {
                    printf("error message:line %d,division by zero\n", line);
                    flag_err = 0;
                }
                else
                {
                    res = val1 / val2;
                }
            }

            Symbol sym = symTable.get(result);
            if (sym.type == TYPE_INT)
                symTable.setInt(result, (int)res);
            else
                symTable.setReal(result, res);

            irGen.emit(op, arg1, arg2, result);
        }
    }
}

string pending_token = "";

int read_token_wrapper(char *s)
{
    if (!pending_token.empty())
    {
        strcpy(s, pending_token.c_str());
        pending_token = "";
        return strlen(s);
    }
    return read_safe(s);
}

void solve_opt_wrapper(int line, bool execute)
{
    char s[100];
    string arg1, arg2, op, result;

    // 结果
    int t = read_token_wrapper(s);
    result = s;

    read_token_wrapper(s); // =

    read_token_wrapper(s); // arg1
    arg1 = s;
    double val1 = parseValue(s);

    t = read_token_wrapper(s); // Op?
    if (t <= 0)
    { // EOF or Newline
      // 语句结束
    }
    else
    {
        bool is_op = (s[0] == '+' || s[0] == '-' || s[0] == '*' || s[0] == '/');
        if (!is_op)
        {
            pending_token = s; // 不是操作符，留给下一次
        }
        else
        {
            op = s;
            read_token_wrapper(s); // arg2
            arg2 = s;
            double val2 = parseValue(s);

            if (execute)
            {
                double res = 0;
                if (op == "+")
                    res = val1 + val2;
                else if (op == "-")
                    res = val1 - val2;
                else if (op == "*")
                    res = val1 * val2;
                else if (op == "/")
                {
                    if (abs(val2) < 1e-6)
                    {
                        printf("error message:line %d,division by zero\n", line);
                        flag_err = 0;
                    }
                    else
                        res = val1 / val2;
                }

                Symbol sym = symTable.get(result);
                if (sym.type == TYPE_INT)
                    symTable.setInt(result, (int)res);
                else
                    symTable.setReal(result, res);

                irGen.emit(op, arg1, arg2, result);
            }
            return; // 完成
        }
    }

    // 仅赋值情况
    if (execute)
    {
        Symbol sym = symTable.get(result);
        if (sym.type == TYPE_INT || sym.type == TYPE_UNKNOWN)
        {
            bool is_int = true;
            for (char c : arg1)
                if (c == '.')
                    is_int = false;
            if (is_int)
                symTable.setInt(result, (int)val1);
            else
                symTable.setReal(result, val1);
        }
        else
        {
            if (sym.type == TYPE_INT)
                symTable.setInt(result, (int)val1);
            else
                symTable.setReal(result, val1);
        }
        irGen.emit("=", arg1, "", result);
    }
}

void solveif_opt(int line)
{
    char s[100];
    read_token_wrapper(s); // a
    string arg1 = s;
    double v1 = parseValue(s);

    read_token_wrapper(s); // op
    string op = s;

    read_token_wrapper(s); // b
    string arg2 = s;
    double v2 = parseValue(s);

    bool cond = false;
    if (op == ">")
        cond = v1 > v2;
    else if (op == "<")
        cond = v1 < v2;
    else if (op == "==")
        cond = abs(v1 - v2) < 1e-6;

    irGen.emit("IF_" + op, arg1, arg2, "GOTO_TRUE");

    read_token_wrapper(s); // then

    if (cond)
    {
        solve_opt_wrapper(line, true);
        // 检查 else
        read_token_wrapper(s);
        if (strcmp(s, "else") == 0)
        {
            solve_opt_wrapper(line, false); // 跳过 else 块
        }
        else
        {
            pending_token = s; // 不是 else，推回
        }
    }
    else
    {
        solve_opt_wrapper(line, false); // 跳过 then 块
        read_token_wrapper(s);
        if (strcmp(s, "else") == 0)
        {
            solve_opt_wrapper(line, true); // 执行 else 块
        }
        else
        {
            pending_token = s;
        }
    }
}

void AnalysisOptimized(istream &in, const string &baseName)
{
    reader = new BufferedReader(in);
    char s[100];

    // 阶段 1: 声明
    while (true)
    {
        int t = read_token_wrapper(s);
        if (t <= 0)
            break;

        if (strcmp(s, "int") == 0)
        {
            read_token_wrapper(s);
            string name = s;
            read_token_wrapper(s); // =
            read_token_wrapper(s); // val
            int val = atoi(s);
            symTable.setInt(name, val);
            irGen.emit("DECL_INT", s, "", name);
        }
        else if (strcmp(s, "real") == 0)
        {
            read_token_wrapper(s);
            string name = s;
            read_token_wrapper(s); // =
            read_token_wrapper(s); // val
            double val = atof(s);
            symTable.setReal(name, val);
            irGen.emit("DECL_REAL", s, "", name);
        }
        else
        {
            pending_token = s; // 语句开始
            break;
        }
    }

    // 阶段 2: 语句
    int line = 2;
    int loop_limit = 0;
    while (true)
    {
        if (++loop_limit > 10000)
            break;

        int t = -1;
        // 跳过换行/空
        while (true)
        {
            t = read_token_wrapper(s);
            if (t != -1)
                break; // -1 是换行
        }
        if (t == 0)
            break; // EOF

        if (strcmp(s, "if") == 0)
        {
            solveif_opt(line);
        }
        else
        {
            pending_token = s;
            solve_opt_wrapper(line, true);
        }
        line++;
    }

    // 输出
    if (flag_err)
    {
        map<string, Symbol> sorted(symTable.table.begin(), symTable.table.end());
        for (auto &pair : sorted)
        {
            if (pair.second.type != TYPE_UNKNOWN)
            {
                cout << pair.first << ": ";
                if (pair.second.type == TYPE_INT)
                    cout << pair.second.iVal << endl;
                else
                    cout << pair.second.rVal << endl;
            }
        }
    }

    string csvPath = "SemanticAnalysis/output/" + baseName + "_symbol_table_snapshot.csv";
    string irPath = "SemanticAnalysis/output/" + baseName + "_ir_code.txt";
    symTable.dumpSnapshot(csvPath);
    irGen.dumpIR(irPath);

    delete reader;
}

void AnalysisOptimized(istream &in)
{
    AnalysisOptimized(in, "default");
}

#endif
