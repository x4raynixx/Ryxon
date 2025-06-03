#ifndef SYSTEM_LIBRARY_H
#define SYSTEM_LIBRARY_H

#include "../../Interpreter.h"
#include <vector>
#include <string>

class SystemLibrary {
public:
    static void initialize();
    static Value callFunction(const std::string& name, const std::vector<Value>& args);
};

#endif
