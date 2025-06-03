#ifndef MATH_LIBRARY_H
#define MATH_LIBRARY_H

#include "../../Interpreter.h"
#include <vector>
#include <string>

class MathLibrary {
public:
    static void initialize();
    static Value callFunction(const std::string& name, const std::vector<Value>& args);
};

#endif
