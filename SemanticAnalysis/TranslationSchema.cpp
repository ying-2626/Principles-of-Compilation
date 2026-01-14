#include "TranslationSchemaOptimized.h"
#include <fstream>
#include <string>

int main(int argc, char** argv)
{
    if (argc > 1) {
        std::ifstream fin(argv[1]);
        if (fin) {
            std::string path = argv[1];
            std::string base = path;
            std::size_t pos = base.find_last_of("/\\");
            if (pos != std::string::npos) {
                base = base.substr(pos + 1);
            }
            pos = base.find_last_of('.');
            if (pos != std::string::npos) {
                base = base.substr(0, pos);
            }
            AnalysisOptimized(fin, base);
        } else {
            std::cerr << "Cannot open file: " << argv[1] << std::endl;
            return 1;
        }
    } else {
        AnalysisOptimized(cin, "stdin");
    }
    return 0;
}
