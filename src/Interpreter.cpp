#include "Interpreter.h"
#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <sstream>
#include <cmath>
#include <random>
#include <unordered_map>

// hepler functions for value creation
Value makeNumber(double n) {
    return std::make_shared<ValueData>(n);
}

Value makeString(const std::string& s) {
    return std::make_shared<ValueData>(s);
}

Value makeArray(const std::vector<Value>& a) {
    return std::make_shared<ValueData>(a);
}

Value makeObject(const std::unordered_map<std::string, Value>& o) {
    return std::make_shared<ValueData>(o);
}

Interpreter::Interpreter() : returnFlag(false), returnValue(makeNumber(0.0)) {}

void Interpreter::interpret(Program* program) {
    try {
        executeBlock(program->statements);
    } catch (const std::exception& e) {
        std::cerr << "Runtime error: " << e.what() << std::endl;
    }
}

Value Interpreter::evaluate(Expression* expr) {
    if (auto numExpr = dynamic_cast<NumberExpression*>(expr)) {
        return makeNumber(numExpr->value);
    }
    
    if (auto strExpr = dynamic_cast<StringExpression*>(expr)) {
        return makeString(strExpr->value);
    }
    
    if (auto colorExpr = dynamic_cast<ColorStringExpression*>(expr)) {
        // ret color string with ANSI for immediate printing
        std::string result;
        
        // ANSI color codes
        if (colorExpr->color == "red") result += "\033[31m";
        else if (colorExpr->color == "green") result += "\033[32m";
        else if (colorExpr->color == "yellow") result += "\033[33m";
        else if (colorExpr->color == "blue") result += "\033[34m";
        else if (colorExpr->color == "magenta") result += "\033[35m";
        else if (colorExpr->color == "cyan") result += "\033[36m";
        else if (colorExpr->color == "white") result += "\033[37m";
        else if (colorExpr->color == "black") result += "\033[30m";
        
        result += colorExpr->text + "\033[0m"; // add text and reset color
        return makeString(result);
    }
    
    if (auto arrayExpr = dynamic_cast<ArrayExpression*>(expr)) {
        std::vector<Value> elements;
        for (const auto& elem : arrayExpr->elements) {
            elements.push_back(evaluate(elem.get()));
        }
        return makeArray(elements);
    }
    
    if (auto objExpr = dynamic_cast<ObjectExpression*>(expr)) {
        std::unordered_map<std::string, Value> properties;
        for (const auto& prop : objExpr->properties) {
            if (auto funcExpr = dynamic_cast<FunctionExpression*>(prop.second.get())) {
                // store function in functions map with unique name
                std::string funcName = "__obj_method_" + prop.first + "_" + std::to_string(reinterpret_cast<uintptr_t>(funcExpr));
                functions[funcName] = Function(funcExpr->parameters, &funcExpr->body);
                properties[prop.first] = makeString(funcName); // stire function name as reference
            } else {
                properties[prop.first] = evaluate(prop.second.get());
            }
        }
        return makeObject(properties);
    }
    
    if (auto funcExpr = dynamic_cast<FunctionExpression*>(expr)) {
        // create unique function name and store it
        std::string funcName = "__lambda_" + std::to_string(reinterpret_cast<uintptr_t>(funcExpr));
        functions[funcName] = Function(funcExpr->parameters, &funcExpr->body);
        return makeString(funcName);
    }
    
    if (auto idExpr = dynamic_cast<IdentifierExpression*>(expr)) {
        // chcek if its a saved value first
        auto savedIt = savedValues.find(idExpr->name);
        if (savedIt != savedValues.end()) {
            return savedIt->second;
        }
        return getVariable(idExpr->name);
    }
    
    if (auto memberExpr = dynamic_cast<MemberExpression*>(expr)) {
        Value object = evaluate(memberExpr->object.get());
        
        if (object->type == ValueData::OBJECT) {
            auto it = object->object->find(memberExpr->property);
            if (it != object->object->end()) {
                return it->second;
            }
            throw std::runtime_error("Property '" + memberExpr->property + "' not found");
        }
        
        if (object->type == ValueData::ARRAY && memberExpr->property == "value") {
            return object; // ret the array itself when accessing .value
        }
        
        throw std::runtime_error("Cannot access property of non-object");
    }
    
    if (auto indexExpr = dynamic_cast<IndexExpression*>(expr)) {
        Value object = evaluate(indexExpr->object.get());
        Value index = evaluate(indexExpr->index.get());
        
        if (object->type == ValueData::ARRAY) {
            int idx = static_cast<int>(valueToNumber(index));
            if (idx >= 0 && idx < static_cast<int>(object->array->size())) {
                return (*object->array)[idx];
            }
            throw std::runtime_error("Array index out of bounds");
        }
        
        throw std::runtime_error("Cannot index non-array value");
    }
    
    if (auto typeofExpr = dynamic_cast<TypeofExpression*>(expr)) {
        Value val = evaluate(typeofExpr->expression.get());
        return makeString(getValueType(val));
    }
    
    if (auto binExpr = dynamic_cast<BinaryExpression*>(expr)) {
        Value left = evaluate(binExpr->left.get());
        Value right = evaluate(binExpr->right.get());
        
        if (binExpr->operator_ == "+") {
            if (left->type == ValueData::STRING || right->type == ValueData::STRING) {
                return makeString(valueToString(left) + valueToString(right));
            }
            return makeNumber(valueToNumber(left) + valueToNumber(right));
        }
        if (binExpr->operator_ == "-") {
            return makeNumber(valueToNumber(left) - valueToNumber(right));
        }
        if (binExpr->operator_ == "*") {
            return makeNumber(valueToNumber(left) * valueToNumber(right));
        }
        if (binExpr->operator_ == "/") {
            double rightVal = valueToNumber(right);
            if (rightVal == 0) throw std::runtime_error("Division by zero");
            return makeNumber(valueToNumber(left) / rightVal);
        }
        if (binExpr->operator_ == "%") {
            return makeNumber(std::fmod(valueToNumber(left), valueToNumber(right)));
        }
        if (binExpr->operator_ == "==") {
            return makeNumber((valueToString(left) == valueToString(right)) ? 1.0 : 0.0);
        }
        if (binExpr->operator_ == "!=") {
            return makeNumber((valueToString(left) != valueToString(right)) ? 1.0 : 0.0);
        }
        if (binExpr->operator_ == "<") {
            return makeNumber((valueToNumber(left) < valueToNumber(right)) ? 1.0 : 0.0);
        }
        if (binExpr->operator_ == ">") {
            return makeNumber((valueToNumber(left) > valueToNumber(right)) ? 1.0 : 0.0);
        }
        if (binExpr->operator_ == "<=") {
            return makeNumber((valueToNumber(left) <= valueToNumber(right)) ? 1.0 : 0.0);
        }
        if (binExpr->operator_ == ">=") {
            return makeNumber((valueToNumber(left) >= valueToNumber(right)) ? 1.0 : 0.0);
        }
    }
    
    if (auto callExpr = dynamic_cast<CallExpression*>(expr)) {
        std::vector<Value> args;
        for (const auto& arg : callExpr->arguments) {
            args.push_back(evaluate(arg.get()));
        }
        return callFunction(callExpr->functionName, args);
    }
    
    if (auto methodCallExpr = dynamic_cast<MethodCallExpression*>(expr)) {
        Value object = evaluate(methodCallExpr->object.get());
        std::vector<Value> args;
        for (const auto& arg : methodCallExpr->arguments) {
            args.push_back(evaluate(arg.get()));
        }
        return callMethod(object, methodCallExpr->methodName, args);
    }
    
    throw std::runtime_error("Unknown expression type");
}

void Interpreter::execute(Statement* stmt) {
    if (returnFlag) return;
    
    if (auto exprStmt = dynamic_cast<ExpressionStatement*>(stmt)) {
        evaluate(exprStmt->expression.get());
        return;
    }
    
    if (auto assignStmt = dynamic_cast<AssignmentStatement*>(stmt)) {
        Value value = evaluate(assignStmt->value.get());
        setVariable(assignStmt->variable, value);
        return;
    }
    
    if (auto newVarStmt = dynamic_cast<NewVariableStatement*>(stmt)) {
        Value value = evaluate(newVarStmt->value.get());
        setVariable(newVarStmt->variable, value);
        return;
    }
    
    if (auto ifStmt = dynamic_cast<IfStatement*>(stmt)) {
        Value condition = evaluate(ifStmt->condition.get());
        if (valueToBoolean(condition)) {
            executeBlock(ifStmt->thenBranch);
        } else if (!ifStmt->elseBranch.empty()) {
            executeBlock(ifStmt->elseBranch);
        }
        return;
    }
    
    if (auto whileStmt = dynamic_cast<WhileStatement*>(stmt)) {
        while (valueToBoolean(evaluate(whileStmt->condition.get())) && !returnFlag) {
            executeBlock(whileStmt->body);
        }
        return;
    }
    
    if (auto funcStmt = dynamic_cast<FunctionStatement*>(stmt)) {
        functions[funcStmt->name] = Function(funcStmt->parameters, &funcStmt->body);
        return;
    }
    
    if (auto retStmt = dynamic_cast<ReturnStatement*>(stmt)) {
        if (retStmt->value) {
            returnValue = evaluate(retStmt->value.get());
        } else {
            returnValue = makeNumber(0.0);
        }
        returnFlag = true;
        return;
    }
    
    if (auto sysStmt = dynamic_cast<SystemCallStatement*>(stmt)) {
        Value command = evaluate(sysStmt->command.get());
        std::string cmdStr = valueToString(command);
        int result = std::system(cmdStr.c_str());
        if (result != 0) {
            std::cout << "command exited with code: " << result << std::endl;
        }
        return;
    }
    
    if (auto importStmt = dynamic_cast<ImportStatement*>(stmt)) {
        importLibrary(importStmt->libraryName);
        return;
    }
    
    if (auto saveStmt = dynamic_cast<SaveStatement*>(stmt)) {
        // just mark that this save slot exists
        savedValues[saveStmt->saveName] = makeString("");
        return;
    }
    
    if (auto msgStmt = dynamic_cast<MessageStatement*>(stmt)) {
        Value value = evaluate(msgStmt->value.get());
        savedValues[msgStmt->messageName] = value;
        return;
    }
    
    throw std::runtime_error("Unknown statement type");
}

void Interpreter::executeBlock(const std::vector<StatementPtr>& statements) {
    for (const auto& stmt : statements) {
        execute(stmt.get());
        if (returnFlag) break;
    }
}

Value Interpreter::callMethod(const Value& object, const std::string& methodName, const std::vector<Value>& args) {
    if (object->type == ValueData::OBJECT) {
        auto it = object->object->find(methodName);
        if (it != object->object->end() && it->second->type == ValueData::STRING) {
            // ts is a method (function reference)
            std::string funcName = *it->second->string;
            
            // set up object context for method call
            auto savedVars = variables;
            
            // add object properties to methods scope
            for (const auto& prop : *object->object) {
                if (prop.first != methodName) { // dont overide the method itself
                    setVariable(prop.first, prop.second);
                }
            }
            
            Value result = callFunction(funcName, args);
            
            // restore variables but keep any changes to object properties
            variables = savedVars;
            
            return result;
        }
        throw std::runtime_error("Method '" + methodName + "' not found");
    }
    
    throw std::runtime_error("Cannot call method on non-object");
}

Value Interpreter::callFunction(const std::string& name, const std::vector<Value>& args) {
    // check library functions first
    if (importedLibraries.count("math") && 
        (name == "sqrt" || name == "pow" || name == "sin" || name == "cos" || 
         name == "tan" || name == "abs" || name == "floor" || name == "ceil" || name == "random" ||
         name == "log" || name == "log10" || name == "exp" || name == "min" || name == "max" || name == "round")) {
        return callMathFunction(name, args);
    }
    
    if (importedLibraries.count("sc") && (name == "ifu" || name == "print")) {
        return callScFunction(name, args);
    }
    
    if (importedLibraries.count("colors") && name == "print") {
        return callColorFunction(name, args);
    }
    
    // built-in functions (always available)
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
    
    // user-defined functions
    auto it = functions.find(name);
    if (it == functions.end()) {
        throw std::runtime_error("Undefined function: " + name);
    }
    
    Function& func = it->second;
    
    if (args.size() != func.parameters.size()) {
        throw std::runtime_error("Function " + name + " expects " + 
                                std::to_string(func.parameters.size()) + " arguments, got " + 
                                std::to_string(args.size()));
    }
    
    // save current state
    auto savedVars = variables;
    bool savedReturnFlag = returnFlag;
    Value savedReturnValue = returnValue;
    
    // set parameters
    for (size_t i = 0; i < func.parameters.size(); ++i) {
        setVariable(func.parameters[i], args[i]);
    }
    
    returnFlag = false;
    returnValue = makeNumber(0.0);
    
    // execute function body
    if (func.body) {
        executeBlock(*func.body);
    }
    
    Value result = returnValue;
    
    // restore state
    variables = savedVars;
    returnFlag = savedReturnFlag;
    returnValue = savedReturnValue;
    
    return result;
}

void Interpreter::importLibrary(const std::string& libraryName) {
    importedLibraries.insert(libraryName);
    // std::cout << "imported library: " << libraryName << std::endl;
}

Value Interpreter::callMathFunction(const std::string& name, const std::vector<Value>& args) {
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
    if (name == "random") {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        static std::uniform_real_distribution<> dis(0.0, 1.0);
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
    if (name == "min") {
        if (args.size() != 2) throw std::runtime_error("min expects 2 arguments");
        return makeNumber(std::min(valueToNumber(args[0]), valueToNumber(args[1])));
    }
    if (name == "max") {
        if (args.size() != 2) throw std::runtime_error("max expects 2 arguments");
        return makeNumber(std::max(valueToNumber(args[0]), valueToNumber(args[1])));
    }
    if (name == "round") {
        if (args.size() != 1) throw std::runtime_error("round expects 1 argument");
        return makeNumber(std::round(valueToNumber(args[0])));
    }
    
    throw std::runtime_error("Unknown math function: " + name);
}

Value Interpreter::callScFunction(const std::string& name, const std::vector<Value>& args) {
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
    
    throw std::runtime_error("Unknown sc function: " + name);
}

Value Interpreter::callColorFunction(const std::string& name, const std::vector<Value>& args) {
    if (name == "print") {
        for (size_t i = 0; i < args.size(); ++i) {
            if (i > 0) std::cout << " ";
            std::string text = valueToString(args[i]);
            
            // check for color syntax c@color"text"
            if (text.find("c@") == 0) {
                size_t colorEnd = text.find('"');
                if (colorEnd != std::string::npos) {
                    std::string color = text.substr(2, colorEnd - 2);
                    std::string content = text.substr(colorEnd + 1);
                    if (content.back() == '"') content.pop_back();
                    
                    // ANSI color codes
                    if (color == "red") std::cout << "\033[31m";
                    else if (color == "green") std::cout << "\033[32m";
                    else if (color == "yellow") std::cout << "\033[33m";
                    else if (color == "blue") std::cout << "\033[34m";
                    else if (color == "magenta") std::cout << "\033[35m";
                    else if (color == "cyan") std::cout << "\033[36m";
                    else if (color == "white") std::cout << "\033[37m";
                    else if (color == "black") std::cout << "\033[30m";
                    
                    std::cout << content << "\033[0m"; // reset color
                } else {
                    std::cout << text;
                }
            } else {
                std::cout << text;
            }
        }
        std::cout << std::endl;
        return makeNumber(0.0);
    }
    
    throw std::runtime_error("Unknown color function: " + name);
}

std::string Interpreter::valueToString(const Value& value) {
    switch (value->type) {
        case ValueData::STRING:
            return *value->string;
        case ValueData::NUMBER: {
            double num = value->number;
            if (num == static_cast<int>(num)) {
                return std::to_string(static_cast<int>(num));
            }
            return std::to_string(num);
        }
        case ValueData::ARRAY: {
            std::string result = "[";
            for (size_t i = 0; i < value->array->size(); ++i) {
                if (i > 0) result += ", ";
                result += valueToString((*value->array)[i]);
            }
            result += "]";
            return result;
        }
        case ValueData::OBJECT: {
            std::string result = "{";
            bool first = true;
            for (const auto& prop : *value->object) {
                if (!first) result += ", ";
                result += prop.first + ": " + valueToString(prop.second);
                first = false;
            }
            result += "}";
            return result;
        }
    }
    return "unknown";
}

double Interpreter::valueToNumber(const Value& value) {
    switch (value->type) {
        case ValueData::NUMBER:
            return value->number;
        case ValueData::STRING:
            try {
                return std::stod(*value->string);
            } catch (...) {
                return 0.0;
            }
        default:
            return 0.0;
    }
}

bool Interpreter::valueToBoolean(const Value& value) {
    switch (value->type) {
        case ValueData::NUMBER:
            return value->number != 0.0;
        case ValueData::STRING:
            return !value->string->empty();
        case ValueData::ARRAY:
            return !value->array->empty();
        case ValueData::OBJECT:
            return !value->object->empty();
    }
    return false;
}

std::string Interpreter::getValueType(const Value& value) {
    switch (value->type) {
        case ValueData::STRING:
            return "string";
        case ValueData::NUMBER: {
            double num = value->number;
            if (num == static_cast<int>(num)) {
                return "int";
            }
            return "float";
        }
        case ValueData::ARRAY:
            return "array";
        case ValueData::OBJECT:
            return "object";
    }
    return "unknown";
}

void Interpreter::setVariable(const std::string& name, const Value& value) {
    variables[name] = value;
}

Value Interpreter::getVariable(const std::string& name) {
    auto it = variables.find(name);
    if (it == variables.end()) {
        throw std::runtime_error("Undefined variable: " + name);
    }
    return it->second;
}
