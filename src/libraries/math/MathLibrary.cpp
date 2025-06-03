#include "MathLibrary.h"
#include <cmath>
#include <random>
#include <unordered_map>
#include <stdexcept>
#include <algorithm>

// Static variables for random number generation
static std::random_device rd;
static std::mt19937 gen(rd());
static std::uniform_real_distribution<> dis(0.0, 1.0);

void MathLibrary::initialize() {
    gen.seed(rd());
}

Value MathLibrary::callFunction(const std::string& name, const std::vector<Value>& args) {
    if (name == "sqrt") {
        if (args.size() != 1) throw std::runtime_error("sqrt expects 1 argument");
        return makeNumber(std::sqrt(valueToNumber(args[0])));
    }
    if (name == "pow") {
        if (args.size() != 2) throw std::runtime_error("pow expects 2 arguments");
        return makeNumber(std::pow(valueToNumber(args[0]), valueToNumber(args[1])));
    }
    if (name == "sin") {
        if (args.size() != 1) throw std::runtime_error("sin expects 1 argument");
        return makeNumber(std::sin(valueToNumber(args[0])));
    }
    if (name == "cos") {
        if (args.size() != 1) throw std::runtime_error("cos expects 1 argument");
        return makeNumber(std::cos(valueToNumber(args[0])));
    }
    if (name == "tan") {
        if (args.size() != 1) throw std::runtime_error("tan expects 1 argument");
        return makeNumber(std::tan(valueToNumber(args[0])));
    }
    if (name == "abs") {
        if (args.size() != 1) throw std::runtime_error("abs expects 1 argument");
        return makeNumber(std::abs(valueToNumber(args[0])));
    }
    if (name == "floor") {
        if (args.size() != 1) throw std::runtime_error("floor expects 1 argument");
        return makeNumber(std::floor(valueToNumber(args[0])));
    }
    if (name == "ceil") {
        if (args.size() != 1) throw std::runtime_error("ceil expects 1 argument");
        return makeNumber(std::ceil(valueToNumber(args[0])));
    }
    if (name == "round") {
        if (args.size() != 1) throw std::runtime_error("round expects 1 argument");
        return makeNumber(std::round(valueToNumber(args[0])));
    }
    if (name == "min") {
        if (args.size() != 2) throw std::runtime_error("min expects 2 arguments");
        double a = valueToNumber(args[0]);
        double b = valueToNumber(args[1]);
        return makeNumber((a < b) ? a : b);
    }
    if (name == "max") {
        if (args.size() != 2) throw std::runtime_error("max expects 2 arguments");
        double a = valueToNumber(args[0]);
        double b = valueToNumber(args[1]);
        return makeNumber((a > b) ? a : b);
    }
    if (name == "random") {
        return makeNumber(dis(gen));
    }
    if (name == "log") {
        if (args.size() != 1) throw std::runtime_error("log expects 1 argument");
        return makeNumber(std::log(valueToNumber(args[0])));
    }
    if (name == "log10") {
        if (args.size() != 1) throw std::runtime_error("log10 expects 1 argument");
        return makeNumber(std::log10(valueToNumber(args[0])));
    }
    if (name == "exp") {
        if (args.size() != 1) throw std::runtime_error("exp expects 1 argument");
        return makeNumber(std::exp(valueToNumber(args[0])));
    }
    
    throw std::runtime_error("Unknown math function: " + name);
}
