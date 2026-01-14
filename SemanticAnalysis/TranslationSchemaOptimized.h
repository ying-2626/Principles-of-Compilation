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

// --- 1. Infrastructure: Buffered Reader ---
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

// --- 2. Type System & Symbol Table ---
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
        return Symbol(); // Unknown
    }

    void dumpSnapshot(const string &filename)
    {
        ofstream out(filename);
        out << "Name,Type,Value" << endl;
        // Sort for consistent output
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

// --- 3. IR Generation ---
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

// Global Context
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

// Optimized read function that skips invalid chars (like original)
int read_safe(char *s)
{
    char c = get_char();
    // Skip ANY character that is not in the allowed set, except newline and $
    // Allowed: a-z, 0-9, ., >, <, =, !, +, -, *, /
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
    unget_char(c); // Put back delimiter

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

// Forward declaration
void solve_opt(int line, bool execute);

// Helper to check/skip statement
void solvecheck_opt(int line)
{
    solve_opt(line, false);
}

// execute=true means update symbol table, false means just parse
void solve_opt(int line, bool execute)
{
    char s[100];
    string arg1, arg2, op, result;

    // 1. Result Variable
    int t = read_safe(s);
    result = s;

    read_safe(s); // =

    // 2. Operand 1
    read_safe(s);
    arg1 = s;
    double val1 = parseValue(s);

    // 3. Operator or End
    read_safe(s);
    if (strlen(s) == 0 || s[0] == '\n')
    { // Check for empty/newline handled by read_safe return
        // Assignment only: a = 10
        // But wait, read_safe returns -1 on newline.
        // If s is empty string, it means we hit a delimiter that wasn't allowed?
        // Actually, read_safe returns len.
        // Let's rely on checking if s is operator.
    }

    bool is_op = (s[0] == '+' || s[0] == '-' || s[0] == '*' || s[0] == '/');

    if (!is_op)
    {
        // It was the end of statement or something else.
        // If it's not an op, we assume it's next token (or end).
        // Original logic checked delimiters.
        // Here, if s is not op, we should probably unget it?
        // But read_safe already consumed it.
        // Actually, simplified grammar: Assignment is either "v = x" or "v = x op y".
        // If s is NOT an op, it must be that the statement ended.
        // But wait, read_safe skips delimiters like ';'.
        // So if we read "then" or "else", that's not op.
        // So if s is not op, we have consumed the first token of NEXT statement (or then/else).
        // We need to unget the whole token string? Not supported.
        // Hack: Global 'pushed_back_token'.
        // BUT, for test1.txt: "a = a + 1", s will be "+".
        // "c = c / 2", s will be "/".
        // "int a = 1", s will be "int" (next decl) or empty?

        // Let's refine:
        // If s is empty, it means read_safe hit \n or $.
    }

    if (!is_op)
    {
        // Assignment: result = arg1
        if (execute)
        {
            Symbol sym = symTable.get(result);
            if (sym.type == TYPE_INT || sym.type == TYPE_UNKNOWN)
            {
                // Inference
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

        // Since we read 's' and it wasn't an op, we might have over-read if it wasn't empty.
        // But in test1.txt, statements end with ; which is skipped.
        // So "a = 1 ; int ..." -> read "a", "=", "1". Next read gets "int".
        // So s would be "int".
        // We need to push back "int" so main loop can handle it?
        // Or solve_opt is called when we KNOW it's a statement.
        // The issue is distinguishing "v=x" from "v=x+y".
        // If we read "int", it's not op.
        // We need a mechanism to push back token.
        // Let's add that to BufferedReader/Global.
        // But simpler: "s" is the lookahead.
        // If we are in solve_opt, we assume we are parsing assignment.
        // If s is not op, we assume end of assignment.
        // We need to handle 's' being the start of next thing.
        // Let's use a global 'pending_token' string.
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

    // Result
    int t = read_token_wrapper(s);
    result = s;

    read_token_wrapper(s); // =

    read_token_wrapper(s); // arg1
    arg1 = s;
    double val1 = parseValue(s);

    t = read_token_wrapper(s); // Op?
    if (t <= 0)
    { // EOF or Newline
        // End of stmt
    }
    else
    {
        bool is_op = (s[0] == '+' || s[0] == '-' || s[0] == '*' || s[0] == '/');
        if (!is_op)
        {
            pending_token = s; // Not op, save for next
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
            return; // Done
        }
    }

    // Assignment only case
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
        // Check for else
        read_token_wrapper(s);
        if (strcmp(s, "else") == 0)
        {
            solve_opt_wrapper(line, false); // Skip else block
        }
        else
        {
            pending_token = s; // Not else, push back
        }
    }
    else
    {
        solve_opt_wrapper(line, false); // Skip then block
        read_token_wrapper(s);
        if (strcmp(s, "else") == 0)
        {
            solve_opt_wrapper(line, true); // Execute else block
        }
        else
        {
            pending_token = s;
        }
    }
}

void AnalysisOptimized(istream &in)
{
    reader = new BufferedReader(in);
    char s[100];

    // Phase 1: Declarations
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
            pending_token = s; // Start of statements
            break;
        }
    }

    // Phase 2: Statements
    int line = 2;
    int loop_limit = 0;
    while (true)
    {
        if (++loop_limit > 10000)
            break;

        int t = -1;
        // Skip newlines/empty
        while (true)
        {
            t = read_token_wrapper(s);
            if (t != -1)
                break; // -1 is newline
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

    // Output
    if (flag_err)
    {
        // To verify test1.txt output: "a: 2\nb: 4\nc: 1.5" (if logic correct)
        // Original output was "a: 10\nb: 3.14".
        // Let's print sorted map.
        // Wait, the original code printed 'a', 'b', 'c' ... in order of 'a'-'z'.
        // My snapshot prints in key order (sorted map).
        // Let's print to stdout for test verification.
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

    // Dump Snapshot & IR
    // Use relative path assuming running from project root or SemanticAnalysis dir
    // But to be safe and match user request, let's try to put them in SemanticAnalysis folder relative to CWD?
    // The CWD is usually project root. So "SemanticAnalysis/filename".
    // Check if directory exists? It should.
    symTable.dumpSnapshot("SemanticAnalysis/symbol_table_snapshot.csv");
    irGen.dumpIR("SemanticAnalysis/ir_code.txt");

    delete reader;
}

#endif
