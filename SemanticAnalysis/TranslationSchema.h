// C语言词法分析器
#include <cstdio>
#include <cstring>
#include <iostream>
#include <map>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <cmath>
using namespace std;

/* 标准输入函数 - 改为支持流输入 */
void read_prog_sem(string &prog, istream &in)
{
    char c;
    while (in.get(c) && c != '$')
    {
        prog += c;
    }
}

/* 你可以添加其他函数 */
#define LL long long
int kkk;
string sss;
inline char nc()
{
    /*
  static char buf[100000],*p1=buf,*p2=buf;
  if (p1==p2) { p2=(p1=buf)+fread(buf,1,100000,stdin);if (p1==p2) return EOF;}
  return *p1++;
    */
    return sss[kkk++];
}

inline void read(int &x)
{
    char c = nc();
    int b = 1;
    for (; !(c >= '0' && c <= '9'); c = nc())
        if (c == '-')
            b = -1;
    for (x = 0; c >= '0' && c <= '9'; x = x * 10 + c - '0', c = nc())
        ;
    x *= b;
}

inline void read(LL &x)
{
    char c = nc();
    LL b = 1;
    for (; !(c >= '0' && c <= '9'); c = nc())
        if (c == '-')
            b = -1;
    for (x = 0; c >= '0' && c <= '9'; x = x * 10 + c - '0', c = nc())
        ;
    x *= b;
}

inline void read(char &x)
{
    for (x = nc(); !(x == '(' || x == ')'); x = nc())
        ;
}

inline int read(char *s)
{
    char c = nc();
    int len = 1;
    for (; !(c >= 'a' && c <= 'z' || c >= '0' && c <= '9' || c == '.' || c == '>' || c == '<' || c == '=' || c == '!' || c == '+' || c == '-' || c == '*' || c == '/'); c = nc())
    {
        if (c == '\n')
            return -1;
        if (c == '$')
            return 0;
    }
    for (; (c >= 'a' && c <= 'z' || c >= '0' && c <= '9' || c == '.' || c == '>' || c == '<' || c == '=' || c == '!' || c == '+' || c == '-' || c == '*' || c == '/'); s[len++] = c, c = nc())
        ;
    s[len++] = '\0';
    return len - 2;
}
int wt, ss[19];
inline void print(int x)
{
    if (x < 0)
        x = -x, putchar('-');
    if (!x)
        putchar(48);
    else
    {
        for (wt = 0; x; ss[++wt] = x % 10, x /= 10)
            ;
        for (; wt; putchar(ss[wt] + 48), wt--)
            ;
    }
}
inline void print(LL x)
{
    if (x < 0)
        x = -x, putchar('-');
    if (!x)
        putchar(48);
    else
    {
        for (wt = 0; x; ss[++wt] = x % 10, x /= 10)
            ;
        for (; wt; putchar(ss[wt] + 48), wt--)
            ;
    }
}

char s[100];
int a[100], c[100], flag;
double b[100];

bool check(char *s)
{
    int len = strlen(s + 1);
    for (int i = 1; i <= len; i++)
        if (s[i] == '.')
            return false;
    return true;
}

int calc(char *s)
{
    int len = strlen(s + 1), x = 0;
    for (int i = 1; i <= len; i++)
        x = x * 10 + s[i] - '0';
    return x;
}

double calc2(char *s)
{
    int len = strlen(s + 1), i = 1;
    double x = 0;
    for (; i <= len; i++)
    {
        if (s[i] == '.')
            break;
        x = x * 10.0 + s[i] - '0';
    }
    double y = 1;
    for (i++; i <= len; i++)
    {
        y = y / 10.0;
        x += y * (s[i] - '0');
    }
    return x;
}

void solvecheck(int line)
{
    double x = s[1] - 'a', y, z;
    int t;
    t = read(s);
    t = read(s);
    if (s[1] >= '0' && s[1] <= '9')
        y = calc2(s);
    else
    {
        y = s[1] - 'a';
        if (c[(int)y] == 1)
            y = a[(int)y];
        else
            y = b[(int)y];
    }
    t = read(s);
    char op = s[1];
    t = read(s);
    if (s[1] >= '0' && s[1] <= '9')
        z = calc2(s);
    else
    {
        z = s[1] - 'a';
        if (c[(int)z] == 1)
            z = a[(int)z];
        else
            z = b[(int)z];
    }
    // cout<<y<<" "<<op<<" "<<z<<endl;
    if (op == '/' && fabs(z) < 1e-6)
        flag = 0, printf("error message:line %d,division by zero\n", line);
}

void solve(int line)
{
    double x = s[1] - 'a', y, z;
    int t;
    t = read(s);
    t = read(s);
    if (s[1] >= '0' && s[1] <= '9')
        y = calc2(s);
    else
    {
        y = s[1] - 'a';
        if (c[(int)y] == 1)
            y = a[(int)y];
        else
            y = b[(int)y];
    }
    t = read(s);
    char op = s[1];
    t = read(s);
    if (s[1] >= '0' && s[1] <= '9')
        z = calc2(s);
    else
    {
        z = s[1] - 'a';
        if (c[(int)z] == 1)
            z = a[(int)z];
        else
            z = b[(int)z];
    }
    // cout<<y<<" "<<op<<" "<<z<<endl;
    if (op == '/' && fabs(z) < 1e-6)
        flag = 0, printf("error message:line %d,division by zero\n", line);
    if (c[(int)x] == 1)
    {
        if (op == '+')
            a[(int)x] = y + z;
        else if (op == '-')
            a[(int)x] = y - z;
        else if (op == '*')
            a[(int)x] = y * z;
        else if (op == '/')
            a[(int)x] = ((int)y) / ((int)z);
    }
    else
    {
        if (op == '+')
            b[(int)x] = y + z;
        else if (op == '-')
            b[(int)x] = y - z;
        else if (op == '*')
            b[(int)x] = y * z;
        else if (op == '/')
            b[(int)x] = y / z;
    }
    t = read(s);
    if (t == -1)
        return;
    if (s[1] == '+' || s[1] == '-' || s[1] == '*' || s[1] == '/')
        op = s[1];
    else
    {
        kkk -= 2;
        return;
    }
    t = read(s);
    if (s[1] >= '0' && s[1] <= '9')
        z = calc2(s);
    else
    {
        z = s[1] - 'a';
        if (c[(int)z] == 1)
            z = a[(int)z];
        else
            z = b[(int)z];
    }
    if (c[(int)x] == 1)
    {
        if (op == '+')
            a[(int)x] += z;
        else if (op == '-')
            a[(int)x] -= z;
        else if (op == '*')
            a[(int)x] *= z;
        else if (op == '/')
            a[(int)x] = ((int)y) / ((int)z);
    }
    else
    {
        if (op == '+')
            b[(int)x] += z;
        else if (op == '-')
            b[(int)x] -= z;
        else if (op == '*')
            b[(int)x] *= z;
        else if (op == '/')
            b[(int)x] = y / z;
    }
}

void solveif(int line)
{
    double y, z;
    int t;
    t = read(s);
    if (s[1] >= '0' && s[1] <= '9')
        y = calc2(s);
    else
    {
        y = s[1] - 'a';
        if (c[(int)y] == 1)
            y = a[(int)y];
        else
            y = b[(int)y];
    }
    t = read(s);
    char op1 = s[1], op2 = s[2];
    t = read(s);
    if (s[1] >= '0' && s[1] <= '9')
        z = calc2(s);
    else
    {
        z = s[1] - 'a';
        if (c[(int)z] == 1)
            z = a[(int)z];
        else
            z = b[(int)z];
    }
    // cout<<y<<" "<<z<<" "<<op1<<" "<<op2<<endl;
    t = read(s);
    if (op1 == '>' && op2 == '=')
    {
        if (y >= z)
            t = read(s), solve(line), t = read(s), t = read(s), solvecheck(line);
        else
            t = read(s), solvecheck(line), t = read(s), t = read(s), solve(line);
    }
    else if (op1 == '<' && op2 == '=')
    {
        if (y <= z)
            t = read(s), solve(line), t = read(s), t = read(s), solvecheck(line);
        else
            t = read(s), solvecheck(line), t = read(s), t = read(s), solve(line);
    }
    else if (op1 == '=' && op2 == '=')
    {
        if (fabs(y - z) < 1e-6)
            t = read(s), solve(line), t = read(s), t = read(s), solvecheck(line);
        else
            t = read(s), solvecheck(line), t = read(s), t = read(s), solve(line);
    }
    else if (op1 == '<')
    {
        if (y < z)
            t = read(s), solve(line), t = read(s), t = read(s), solvecheck(line);
        else
            t = read(s), solvecheck(line), t = read(s), t = read(s), solve(line);
    }
    else if (op1 == '>')
    {
        if (y > z)
            t = read(s), solve(line), t = read(s), t = read(s), solvecheck(line);
        else
            t = read(s), solvecheck(line), t = read(s), t = read(s), solve(line);
    }
}

void Analysis(istream &in = cin)
{
    string prog;
    read_prog_sem(prog, in);
    /* 骚年们 请开始你们的表演 */
    /********* Begin *********/
    prog += '$';
    kkk = 0;
    sss = prog;
    flag = 1;
    int loop_limit = 0;
    const int MAX_LOOPS = 20000;

    while (1)
    {
        if (++loop_limit > MAX_LOOPS)
            break;
        int t = read(s);
        if (t == -1 || t == 0)
            break;
        if (s[1] == 'i')
        {
            t = read(s);
            int x, y;
            x = s[1] - 'a';
            t = read(s);
            t = read(s);
            if (check(s))
                a[x] = calc(s);
            else
                puts("error message:line 1,realnum can not be translated into int type"), flag = 0;
            c[x] = 1;
        }
        else if (s[1] == 'r')
        {
            t = read(s);
            int x, y;
            x = s[1] - 'a';
            t = read(s);
            t = read(s);
            b[x] = calc2(s);
            c[x] = 2;
        }
    }
    int line = 2;
    loop_limit = 0;
    while (1)
    {
        if (++loop_limit > MAX_LOOPS)
            break;
        line++;
        int t = -1;
        int inner_loop = 0;
        while (t < 0)
        {
            if (++inner_loop > 1000)
                break;
            t = read(s);
            if (t == 0)
                break; // Check for EOF inside loop
        }
        if (t == 0 || inner_loop > 1000)
            break;
        if (s[1] == 'i' && s[2] == 'f' && t == 2)
            solveif(line);
        else
            solve(line);
    }
    if (flag)
    {
        for (int i = 0; i < 26; i++)
            if (c[i] == 1)
                printf("%c: %d\n", i + 'a', a[i]);
            else if (c[i] == 2)
            {
                printf("%c: ", i + 'a');
                if (fabs(((double)((int)b[i])) - b[i]) < 1e-6)
                    printf("%.0f\n", b[i]);
                else if (fabs(((double)((int)(b[i] * 10))) - b[i] * 10) < 1e-6)
                    printf("%.1f\n", b[i]);
                else if (fabs(((double)((int)(b[i] * 100))) - b[i] * 100) < 1e-6)
                    printf("%.2f\n", b[i]);
                else if (fabs(((double)((int)(b[i] * 1000))) - b[i] * 1000) < 1e-6)
                    printf("%.3f\n", b[i]);
                else if (fabs(((double)((int)(b[i] * 10000))) - b[i] * 10000) < 1e-6)
                    printf("%.4f\n", b[i]);
            }
    }
    /********* End *********/
}
