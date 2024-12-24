#include "MacroManager.h"
#include <fstream>
#include <sstream>

void SaveMacro(const Macro& macro) {
    std::ofstream file("macros.txt", std::ios::app);
    if (file.is_open()) {
        file << macro.name << "\n";
        file << macro.delay << "\n";
        for (WORD key : macro.keys) {
            file << key << " ";
        }
        file << "\n";
    }
}

std::vector<Macro> LoadMacros() {
    std::vector<Macro> macros;
    std::ifstream file("macros.txt");
    if (file.is_open()) {
        std::string line;
        while (std::getline(file, line)) {
            Macro macro;
            macro.name = line;

            std::getline(file, line);
            std::stringstream(line) >> macro.delay;

            std::getline(file, line);
            std::stringstream ss(line);
            WORD key;
            while (ss >> key) {
                macro.keys.push_back(key);
            }

            macros.push_back(macro);
        }
    }
    return macros;
}
