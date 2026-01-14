#ifndef VISUALIZER_H
#define VISUALIZER_H

#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <stack>
#include <utility>

using namespace std;

// 若外部未定义 TokenInfo，这里仅共享其含义：
// 在原始代码中，TokenInfo 等价于 pair<string, int>
// 本类假定调用方传入的序列遵循该约定

class Visualizer
{
public:
    // 根据推导序列（记号名，深度）生成 DOT 文件
    // filename: 输出文件路径（如 "output.dot"）
    // seq: 保存 <记号名称, 深度> 的向量
    static void generateDOT(const string &filename, const vector<pair<string, int>> &seq)
    {
        ofstream out(filename);
        if (!out)
        {
            cerr << "错误: 无法创建 DOT 文件: " << filename << endl;
            return;
        }

        out << "digraph SyntaxTree {" << endl;
        out << "    node [shape=box, fontname=\"Arial\"];" << endl;
        out << "    edge [dir=none];" << endl;

        // 需要根据带有深度信息的先序遍历序列重建语法树
        // 栈中存储 <节点编号, 深度> 对
        stack<pair<int, int>> parentStack;

        int nodeId = 0;

        for (const auto &token : seq)
        {
            string label = token.first;
            int depth = abs(token.second); // 假定深度以非负整数形式记录

            // 需要时可以跳过空/ε 结点，也可以选择将其可视化
            if (label.empty())
                continue;

            int currentId = ++nodeId;

            // 将字符串中的引号转义，满足 DOT 语法要求
            string displayLabel = label;
            size_t pos = 0;
            while ((pos = displayLabel.find("\"", pos)) != string::npos)
            {
                displayLabel.replace(pos, 1, "\\\"");
                pos += 2;
            }

            out << "    node" << currentId << " [label=\"" << displayLabel << "\"];" << endl;

            // 查找当前结点在栈中的父结点
            while (!parentStack.empty() && parentStack.top().second >= depth)
            {
                parentStack.pop();
            }

            if (!parentStack.empty())
            {
                int parentId = parentStack.top().first;
                out << "    node" << parentId << " -> node" << currentId << ";" << endl;
            }

            // 将当前结点压栈，作为后续结点的候选父结点
            parentStack.push({currentId, depth});
        }

        out << "}" << endl;
        out.close();
        // 如需调试，可在此输出生成完成信息
    }
};

#endif
