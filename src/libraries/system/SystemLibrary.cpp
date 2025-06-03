#include "SystemLibrary.h"
#include <iostream>
#include <cstdlib>
#include <stdexcept>

void SystemLibrary::initialize() {
    // Initialize system-specific resources if needed
}

Value SystemLibrary::callFunction(const std::string& name, const std::vector<Value>& args) {
    if (name == "print") {
        for (size_t i = 0; i < args.size(); ++i) {
            if (i > 0) std::cout << " ";
            std::cout << valueToString(args[i]);
        }
        std::cout << std::endl;
        return makeNumber(0.0);
    }
    
    if (name == "ifu") {
        if (!args.empty()) {
            std::cout << valueToString(args[0]);
        }
        std::string line;
        std::getline(std::cin, line);
        return makeString(line);
    }
    
    if (name == "ec") {
        if (args.empty()) return makeNumber(0.0);
        std::string cmdStr = valueToString(args[0]);
        int result = std::system(cmdStr.c_str());
        return makeNumber(static_cast<double>(result));
    }
    
    throw std::runtime_error("Unknown system function: " + name);
}
