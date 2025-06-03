#ifndef TIME_LIBRARY_H
#define TIME_LIBRARY_H

#include "../../Interpreter.h"
#include <vector>
#include <string>

class TimeLibrary {
public:
    static void initialize();
    static Value callFunction(const std::string& name, const std::vector<Value>& args);
};

#endif
