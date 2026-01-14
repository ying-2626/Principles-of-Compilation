#include <iostream>
#include <cstdio>
#include <algorithm>
#include <cstring>
#include <cctype>
#include <vector>
#include <string>
#include <queue>
#include <map>
#include <set>
#include <sstream>
#include <fstream>
#define MAX 507
// #define DEBUG
/*Author : byj*/
using namespace std;

// 文法产生式（With Form）：left -> right，back 为在原文法中的编号
class WF
{
public:
    string left, right; // 产生式左部与右部
    int back;           // 指向在原始文法数组中的下标
    int id;             // 在 items 向量中的编号
    WF(const char *s1, const char *s2, int x, int y)
    {
        left = s1;
        right = s2;
        back = x;
        id = y;
    }
    WF(const string &s1, const string &s2, int x, int y)
    {
        left = s1;
        right = s2;
        back = x;
        id = y;
    }
    bool operator<(const WF &a) const
    {
        if (left == a.left)
            return right < a.right;
        return left < a.left;
    }
    bool operator==(const WF &a) const
    {
        return (left == a.left) && (right == a.right);
    }
    void print()
    {
        printf("%s->%s\n", left.c_str(), right.c_str());
    }
};

// 项目集闭包
class Closure
{
public:
    vector<WF> element; // 项目集合
    void print(string str)
    {
        printf("%-15s%-15s\n", "", str.c_str());
        for (int i = 0; i < element.size(); i++)
            element[i].print();
    }
    bool operator==(const Closure &a) const
    {
        if (a.element.size() != element.size())
            return false;
        for (int i = 0; i < a.element.size(); i++)
            if (element[i] == a.element[i])
                continue;
            else
                return false;
        return true;
    }
};

// LR 分析表中单个单元格的动作
struct Content
{
    int type; // 0: 移进 shift，1: 规约 reduce，2: 接受 acc，-1: 出错
    int num;
    string out;
    Content() { type = -1; }
    Content(int a, int b)
        : type(a), num(b) {}
};

// 全局数据结构
vector<WF> wf;                   // 原始文法产生式
map<string, vector<int>> dic;    // 非终结符 -> 含该左部的项目在 items 中的位置
map<string, vector<int>> VN_set; // 非终结符 -> 含该左部的产生式在 wf 中的位置
map<string, bool> vis;           // DFS 计算 FIRST 时的访问标记
string start = "S";              // 增广文法的开始符号
vector<Closure> collection;      // LR(0) 项目集族
vector<WF> items;                // 带点项目集合
char CH = '^';                   // 使用 '^' 作为项目中的点，避免与终结符 '.' 冲突
int go[MAX][256];                // 项目集间的 GOTO 转移，按 ASCII 编码索引
int to[MAX];                     // 用于记录相邻点位项目之间的“前驱”关系
vector<char> V;                  // 文法符号集合（终结符 + 非终结符）
bool used[256];                  // 记录某个字符是否已经加入 V
Content action[MAX][256];        // ACTION 表的中间表示
int Goto[MAX][256];              // GOTO 表的中间表示
map<string, set<char>> first;    // FIRST 集
map<string, set<char>> follow;   // FOLLOW 集

// 构造所有带点项目 items，并建立 VN_set / dic / to 等辅助结构
void make_item()
{
    memset(to, -1, sizeof(to));
    // 记录每个非终结符作为左部出现在哪些产生式中
    for (int i = 0; i < wf.size(); i++)
        VN_set[wf[i].left].push_back(i);

    // 对每条产生式 A -> α，在右部每一个位置插入点，形成带点项目
    for (int i = 0; i < wf.size(); i++)
        for (int j = 0; j <= wf[i].right.length(); j++)
        {
            string temp = wf[i].right;
            if (temp == "@")
            { // 处理空产生式 A -> @
                temp = "";
                temp.insert(0, 1, CH);
                dic[wf[i].left].push_back(items.size());
                items.push_back(WF(wf[i].left, temp, i, items.size()));
                continue;
            }

            // 在第 j 个位置前插入点
            temp.insert(temp.begin() + j, CH);
            dic[wf[i].left].push_back(items.size());
            if (j)
                to[items.size() - 1] = items.size(); // 记录同一产生式中相邻点位项目
            items.push_back(WF(wf[i].left, temp, i, items.size()));
        }
}

// 深度优先搜索计算非终结符 x 的 FIRST 集
void dfs(const string &x)
{
    if (vis[x])
        return;
    vis[x] = 1;
    vector<int> &id = VN_set[x];
    for (int i = 0; i < id.size(); i++)
    {
        string &left = wf[id[i]].left;
        string &right = wf[id[i]].right;
        if (right == "@")
        { // 右部可为空，将 @ 加入 FIRST 集
            first[left].insert('@');
            continue;
        }

        for (int j = 0; j < right.length(); j++)
            if (isupper(right[j]))
            {
                dfs(right.substr(j, 1));
                set<char> &temp = first[right.substr(j, 1)];
                set<char>::iterator it = temp.begin();
                bool flag = true;
                for (; it != temp.end(); it++)
                {
                    if (*it == '@')
                        flag = false;
                    else
                        first[left].insert(*it);
                }
                if (flag)
                    break;
                // 能到达这里，说明该非终结符可以推出空串，继续考察右侧下一个符号
                if (j == right.length() - 1)
                {
                    first[left].insert('@');
                }
            }
            else
            {
                first[left].insert(right[j]);
                break;
            }
    }
}

// 计算文法中所有非终结符的 FIRST 集
void make_first()
{
    vis.clear();
    map<string, vector<int>>::iterator it2 = dic.begin();
    for (; it2 != dic.end(); it2++)
        if (vis[it2->first])
            continue;
        else
            dfs(it2->first);
}

// 将 Follow(str1) 合并进 Follow(str2)
void append(const string &str1, const string &str2)
{
    set<char> &from = follow[str1];
    set<char> &to = follow[str2];
    set<char>::iterator it = from.begin();
    for (; it != from.end(); it++)
        to.insert(*it);
}

// 检查给定产生式编号集合中，是否存在右部为 str 的产生式
bool _check(const vector<int> &id, const string str)
{
    for (int i = 0; i < id.size(); i++)
    {
        int x = id[i];
        if (wf[x].right == str)
            return true;
    }
    return false;
}

// 迭代计算所有非终结符的 FOLLOW 集
void make_follow()
{
    follow[start].insert('#'); // 文法开始符号的 FOLLOW 集中加入 #
    while (true)
    {
        bool goon = false;
        map<string, vector<int>>::iterator it2 = VN_set.begin();
        for (; it2 != VN_set.end(); it2++)
        {
            vector<int> &id = it2->second;
            for (int i = 0; i < id.size(); i++)
            {
                WF &tt = wf[id[i]];
                string &left = tt.left;
                const string &right = tt.right;
                if (right == "@")
                    continue;

                for (int j = right.length() - 1; j >= 0; j--)
                    if (isupper(right[j]))
                    {
                        // 当前非终结符为 right[j]，考察其右侧符号对 FOLLOW 的影响
                        bool all_derive_epsilon = true;

                        // 将 beta 的 FIRST 集（去掉 epsilon）加入 Follow(B)
                        for (int k = j + 1; k < right.length(); k++)
                        {
                            if (isupper(right[k]))
                            {
                                string next_sym = right.substr(k, 1);
                                set<char> &f = first[next_sym];
                                bool has_epsilon = false;
                                for (auto c : f)
                                {
                                    if (c == '@')
                                        has_epsilon = true;
                                    else
                                    {
                                        int old_size = follow[right.substr(j, 1)].size();
                                        follow[right.substr(j, 1)].insert(c);
                                        if (follow[right.substr(j, 1)].size() > old_size)
                                            goon = true;
                                    }
                                }
                                if (!has_epsilon)
                                {
                                    all_derive_epsilon = false;
                                    break;
                                }
                            }
                            else
                            {
                                int old_size = follow[right.substr(j, 1)].size();
                                follow[right.substr(j, 1)].insert(right[k]);
                                if (follow[right.substr(j, 1)].size() > old_size)
                                    goon = true;
                                all_derive_epsilon = false;
                                break;
                            }
                        }

                        // 如果 beta 能推导出空，则将 Follow(A) 加入 Follow(B)
                        if (all_derive_epsilon)
                        {
                            int old_size = follow[right.substr(j, 1)].size();
                            append(left, right.substr(j, 1));
                            if (follow[right.substr(j, 1)].size() > old_size)
                                goon = true;
                        }
                    }
            }
        }
        if (!goon)
            break;
    }
}

// 基于 items 构造 LR(0) 项目集族 collection
void make_set()
{
    bool has[MAX];
    for (int i = 0; i < items.size(); i++)
        if (items[i].left[0] == 'S' && items[i].right[0] == CH)
        {
            Closure temp; // 初始闭包，只包含 S' -> ·A 这一项目
            string &str = items[i].right;
            vector<WF> &element = temp.element;
            element.push_back(items[i]);
            int x = 0;
            for (x = 0; x < str.length(); x++)
                if (str[x] == CH)
                    break;

            memset(has, 0, sizeof(has));
            has[i] = 1;
            if (x != str.length() - 1)
            {
                queue<string> q;
                q.push(str.substr(x + 1, 1));
                while (!q.empty())
                {
                    string u = q.front();
                    q.pop();
                    vector<int> &id = dic[u]; // 所有以 u 为左部的项目
                    for (int j = 0; j < id.size(); j++)
                    {
                        int tx = id[j];
                        if (items[tx].right[0] == CH)
                        {
                            if (has[tx])
                                continue;
                            has[tx] = 1;
                            if (isupper(items[tx].right[1]))
                                q.push(items[tx].right.substr(1, 1));
                            element.push_back(items[tx]);
                        }
                    }
                }
            }
            collection.push_back(temp);
        }
    for (int i = 0; i < collection.size(); i++)
    {
        map<int, Closure> temp;
        for (int j = 0; j < collection[i].element.size(); j++)
        {
            string str = collection[i].element[j].right; // 当前项目右部
            int x = 0;
            for (; x < str.length(); x++)
                if (str[x] == CH)
                    break;
            if (x == str.length() - 1)
                continue;
            int y = str[x + 1]; // 点后面的符号，作为 GOTO 的输入
            int ii;

            str.erase(str.begin() + x);
            str.insert(str.begin() + x + 1, CH);

            WF cmp = WF(collection[i].element[j].left, str, -1, -1);
            for (int k = 0; k < items.size(); k++)
                if (items[k] == cmp)
                {
                    ii = k;
                    break;
                }

            memset(has, 0, sizeof(has));
            vector<WF> &element = temp[y].element;
            element.push_back(items[ii]);
            has[ii] = 1;
            x++;

            if (x != str.length() - 1)
            {
                queue<string> q;
                q.push(str.substr(x + 1, 1));
                while (!q.empty())
                {
                    string u = q.front();
                    q.pop();
                    vector<int> &id = dic[u];
                    for (int j = 0; j < id.size(); j++)
                    {
                        int tx = id[j];
                        if (items[tx].right[0] == CH)
                        {
                            if (has[tx])
                                continue;
                            has[tx] = 1;
                            if (isupper(items[tx].right[1]))
                                q.push(items[tx].right.substr(1, 1));
                            element.push_back(items[tx]);
                        }
                    }
                }
            }
        }
        map<int, Closure>::iterator it = temp.begin();
        for (; it != temp.end(); it++)
            collection.push_back(it->second);
        for (int i = 0; i < collection.size(); i++)
            sort(collection[i].element.begin(), collection[i].element.end());
        for (int i = 0; i < collection.size(); i++)
            for (int j = i + 1; j < collection.size(); j++)
                if (collection[i] == collection[j])
                    collection.erase(collection.begin() + j);
    }
}

void make_V()
{
    memset(used, 0, sizeof(used));
    // 扫描所有产生式，收集出现过的终结符和非终结符，构成符号集合 V
    for (int i = 0; i < wf.size(); i++)
    {
        string &str = wf[i].left;
        for (int j = 0; j < str.length(); j++)
        {
            if (used[str[j]])
                continue;
            used[str[j]] = 1;
            V.push_back(str[j]);
        }
        string &str1 = wf[i].right;
        if (str1 == "@")
            continue;
        for (int j = 0; j < str1.length(); j++)
        {
            if (used[str1[j]])
                continue;
            used[str1[j]] = 1;
            V.push_back(str1[j]);
        }
    }
    sort(V.begin(), V.end());
    V.push_back('#');
}

void make_cmp(vector<WF> &cmp1, int i, char ch)
{
    // 在项目集 I_i 中，找到所有对符号 ch 进行 GOTO 转移后的项目集合 cmp1
    for (int j = 0; j < collection[i].element.size(); j++)
    {
        string str = collection[i].element[j].right;
        int k;
        for (k = 0; k < str.length(); k++)
            if (str[k] == CH)
                break;
        if (k != str.length() - 1 && str[k + 1] == ch)
        {
            str.erase(str.begin() + k);
            str.insert(str.begin() + k + 1, CH);
            cmp1.push_back(WF(collection[i].element[j].left, str, -1, -1));
        }
    }
    sort(cmp1.begin(), cmp1.end());
}

void make_go()
{
    memset(go, -1, sizeof(go));
    int m = collection.size();

    for (int t = 0; t < V.size(); t++)
    {
        char ch = V[t];
        for (int i = 0; i < m; i++)
        {
            vector<WF> cmp1;
            make_cmp(cmp1, i, ch);
            if (cmp1.size() == 0)
                continue;
            // 查找已有的项目集 J，与 cmp1 完全一致则记为 go(I_i, ch) = J
            for (int j = 0; j < m; j++)
            {
                vector<WF> cmp2;
                for (int k = 0; k < collection[j].element.size(); k++)
                {
                    string &str = collection[j].element[k].right;
                    int x;
                    for (x = 0; x < str.length(); x++)
                        if (str[x] == CH)
                            break;
                    if (x && str[x - 1] == ch)
                        cmp2.push_back(WF(collection[j].element[k].left, str, -1, -1));
                }
                sort(cmp2.begin(), cmp2.end());

                bool flag = true;
                if (cmp2.size() != cmp1.size())
                    continue;

                for (int k = 0; k < cmp1.size(); k++)
                    if (cmp1[k] == cmp2[k])
                        continue;
                    else
                        flag = false;

                if (flag)
                    go[i][ch] = j;
            }
        }
    }
}

void make_table()
{
    memset(Goto, -1, sizeof(Goto));

    // 将移进状态 s 写入分析表
    for (int i = 0; i < collection.size(); i++)
        for (int j = 0; j < V.size(); j++)
        {
            char ch = V[j];
            int x = go[i][ch];
            if (x == -1)
                continue;
            if (!isupper(ch))
                action[i][ch] = Content(0, x);
            else
                Goto[i][ch] = x;
        }
    // 将规约 r 和接受 acc 写入分析表
    for (int i = 0; i < collection.size(); i++)
        for (int j = 0; j < collection[i].element.size(); j++)
        {
            WF &tt = collection[i].element[j];
            if (tt.right[tt.right.length() - 1] == CH)
            {
                if (tt.left[0] == 'S')
                    action[i]['#'] = Content(2, -1);
                else
                    for (int k = 0; k < V.size(); k++)
                    {
                        int y = V[k];
                        if (!follow[tt.left].count(V[k]))
                            continue;
                        action[i][y] = Content(1, tt.back);
                    }
            }
        }
}

void export_table()
{
    ofstream out("LRTable.h");
    out << "#ifndef LR_TABLE_H" << endl;
    out << "#define LR_TABLE_H" << endl;
    out << "#include <vector>" << endl;
    out << "#include <string>" << endl;
    out << "using namespace std;" << endl;
    out << "struct Action { int type; int val; };" << endl; // type: 0=shift, 1=reduce, 2=acc, -1=err（含义同生成器中）
    out << "struct Production { string left; string right; };" << endl;

    out << "const int STATE_COUNT = " << collection.size() << ";" << endl;
    out << "const int TERM_COUNT = 256;" << endl;

    out << "Action ACTION[STATE_COUNT][TERM_COUNT];" << endl;
    out << "int GOTO[STATE_COUNT][TERM_COUNT];" << endl;

    out << "void initLRTable() {" << endl;

    // 初始化数组
    out << "    for(int i=0; i<STATE_COUNT; i++) {" << endl;
    out << "        for(int j=0; j<TERM_COUNT; j++) {" << endl;
    out << "            ACTION[i][j] = {-1, -1};" << endl;
    out << "            GOTO[i][j] = -1;" << endl;
    out << "        }" << endl;
    out << "    }" << endl;

    // 填充 ACTION 表
    for (int i = 0; i < collection.size(); i++)
    {
        for (int j = 0; j < 256; j++)
        {
            if (action[i][j].type != -1)
            {
                out << "    ACTION[" << i << "][" << j << "] = {" << action[i][j].type << ", " << action[i][j].num << "};" << endl;
            }
        }
    }

    // 填充 GOTO 表
    for (int i = 0; i < collection.size(); i++)
    {
        for (int j = 0; j < 256; j++)
        {
            if (Goto[i][j] != -1)
            {
                out << "    GOTO[" << i << "][" << j << "] = " << Goto[i][j] << ";" << endl;
            }
        }
    }
    out << "}" << endl;

    // 导出产生式数组
    out << "vector<Production> PRODUCTIONS = {" << endl;
    for (int i = 0; i < wf.size(); i++)
    {
        out << "    {\"" << wf[i].left << "\", \"" << (wf[i].right == "@" ? "" : wf[i].right) << "\"}," << endl;
    }
    out << "};" << endl;

    out << "#endif" << endl;
    out.close();
}

void load_grammar()
{
    // S -> A
    wf.push_back(WF("S", "A", -1, -1));
    // A -> B
    wf.push_back(WF("A", "B", -1, -1));
    // B -> {G}
    wf.push_back(WF("B", "{G}", -1, -1));
    // C -> E | D | B | F
    wf.push_back(WF("C", "E", -1, -1));
    wf.push_back(WF("C", "D", -1, -1));
    wf.push_back(WF("C", "B", -1, -1));
    wf.push_back(WF("C", "F", -1, -1));
    // D -> y(H)vCuC
    wf.push_back(WF("D", "y(H)vCuC", -1, -1));
    // E -> w(H)C
    wf.push_back(WF("E", "w(H)C", -1, -1));
    // F -> [=L;
    wf.push_back(WF("F", "[=L;", -1, -1));
    // G -> CG | @
    wf.push_back(WF("G", "CG", -1, -1));
    wf.push_back(WF("G", "@", -1, -1));
    // H -> LNL
    wf.push_back(WF("H", "LNL", -1, -1));
    // I -> +JI | -JI | @
    wf.push_back(WF("I", "+JI", -1, -1));
    wf.push_back(WF("I", "-JI", -1, -1));
    wf.push_back(WF("I", "@", -1, -1));
    // J -> KM
    wf.push_back(WF("J", "KM", -1, -1));
    // K -> (L) | [ | ]
    wf.push_back(WF("K", "(L)", -1, -1));
    wf.push_back(WF("K", "[", -1, -1));
    wf.push_back(WF("K", "]", -1, -1));
    // L -> JI
    wf.push_back(WF("L", "JI", -1, -1));
    // M -> *KM | /KM | @
    wf.push_back(WF("M", "*KM", -1, -1));
    wf.push_back(WF("M", "/KM", -1, -1));
    wf.push_back(WF("M", "@", -1, -1));
    // N -> z | x | . | < | !
    wf.push_back(WF("N", "z", -1, -1));
    wf.push_back(WF("N", "x", -1, -1));
    wf.push_back(WF("N", ".", -1, -1));
    wf.push_back(WF("N", "<", -1, -1));
    wf.push_back(WF("N", "!", -1, -1));
}

int main()
{
    load_grammar();
    make_item();
    make_first();
    make_follow();
    make_set();
    make_V();
    make_go();
    make_table();
    export_table();
    cout << "LRTable.h generated successfully!" << endl;
    return 0;
}
