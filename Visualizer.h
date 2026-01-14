#ifndef VISUALIZER_H
#define VISUALIZER_H

#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <stack>
#include <utility>

using namespace std;

// Shared definition for TokenInfo if not already available
// In the original code, TokenInfo is typedef pair<string, int> TokenInfo;
// We assume this structure is passed to us.

class Visualizer
{
public:
    // Generate DOT file from derivation sequence (Token, Depth)
    // filename: output path (e.g., "output.dot")
    // seq: vector of <TokenName, Depth>
    static void generateDOT(const string &filename, const vector<pair<string, int>> &seq)
    {
        ofstream out(filename);
        if (!out)
        {
            cerr << "Error: Could not create DOT file: " << filename << endl;
            return;
        }

        out << "digraph SyntaxTree {" << endl;
        out << "    node [shape=box, fontname=\"Arial\"];" << endl;
        out << "    edge [dir=none];" << endl;

        // We need to reconstruct the tree from the pre-order traversal with depth info.
        // Stack stores pairs of <NodeID, Depth>
        stack<pair<int, int>> parentStack;

        int nodeId = 0;

        for (const auto &token : seq)
        {
            string label = token.first;
            int depth = abs(token.second); // Assuming depth is stored in absolute value based on existing code

            // Skip empty/epsilon if desired, or visualize them
            if (label.empty())
                continue;

            int currentId = ++nodeId;

            // Escape quote characters in label for DOT format
            string displayLabel = label;
            size_t pos = 0;
            while ((pos = displayLabel.find("\"", pos)) != string::npos)
            {
                displayLabel.replace(pos, 1, "\\\"");
                pos += 2;
            }

            out << "    node" << currentId << " [label=\"" << displayLabel << "\"];" << endl;

            // Find parent
            while (!parentStack.empty() && parentStack.top().second >= depth)
            {
                parentStack.pop();
            }

            if (!parentStack.empty())
            {
                int parentId = parentStack.top().first;
                out << "    node" << parentId << " -> node" << currentId << ";" << endl;
            }

            // Push current node as potential parent for next nodes
            parentStack.push({currentId, depth});
        }

        out << "}" << endl;
        out.close();
        // cout << "Generated DOT file: " << filename << endl;
    }
};

#endif // VISUALIZER_H
