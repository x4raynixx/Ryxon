#ifndef COLOR_LIBRARY_H
#define COLOR_LIBRARY_H

#include "../../Interpreter.h"
#include <vector>
#include <string>

class ColorLibrary {
public:
    static void initialize();
    static Value callFunction(const std::string& name, const std::vector<Value>& args);
    static bool supportsColor();
};

#endif
