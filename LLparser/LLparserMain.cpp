#include "LLparser.h"
#include <fstream>

int main(int argc, char** argv)
{
    if (argc > 1) {
        std::ifstream fin(argv[1]);
        if (fin) {
            Analysis(fin);
        } else {
            std::cerr << "Cannot open file: " << argv[1] << std::endl;
            return 1;
        }
    } else {
        // Default behavior (read from stdin)
        // If user wants to simulate old behavior, they can use redirection
        Analysis(cin);
    }
    return 0;
}
