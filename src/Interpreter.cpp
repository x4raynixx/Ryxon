#include "Interpreter.h"
#include "libraries/math/MathLibrary.h"
#include "libraries/colors/ColorLibrary.h"
#include "libraries/time/TimeLibrary.h"
#include "libraries/system/SystemLibrary.h"
#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <sstream>
#include <cmath>
#include <random>
#include <unordered_map>
#include <chrono>
#include <ctime>
#include <iomanip>

// Helper functions for value creation
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

// Global helper functions for libraries
double valueToNumber(const Value& value) {
    if (value->type == ValueData::NUMBER) {
        return value->number;
    }
    
    switch (value->type) {
        case ValueData::STRING: {
            const std::string& str = *value->string;
            if (str.empty()) return 0.0;
            
            // Szybka ścieżka dla prostych liczb
            if (str.size() < 20) {
                bool isNumeric = true;
                bool hasDecimal = false;
                bool hasSign = (str[0] == '-' || str[0] == '+');
                
                for (size_t i = hasSign ? 1 : 0; i < str.size(); ++i) {
                    if (str[i] == '.') {
                        if (hasDecimal) {
                            isNumeric = false;
                            break;
                        }
                        hasDecimal = true;
                    } else if (str[i] < '0' || str[i] > '9') {
                        isNumeric = false;
                        break;
                    }
                }
                
                if (isNumeric) {
                    try {
                        return std::stod(str);
                    } catch (...) {
                        return 0.0;
                    }
                }
            }
            
            // Fallback dla złożonych przypadków
            try {
                return std::stod(str);
            } catch (...) {
                return 0.0;
            }
        }
        case ValueData::ARRAY:
        case ValueData::OBJECT:
            return 0.0;
        default:
            return 0.0;
    }
}

// Zoptymalizuj funkcję valueToString dla lepszej wydajności
std::string valueToString(const Value& value) {
    switch (value->type) {
        case ValueData::STRING:
            return *value->string;
        case ValueData::NUMBER: {
            double num = value->number;
            // Szybka ścieżka dla liczb całkowitych
            if (num == static_cast<int>(num)) {
                int intVal = static_cast<int>(num);
                // Predefiniowane stringi dla małych liczb całkowitych
                static const std::string smallInts[] = {
                    "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10",
                    "11", "12", "13", "14", "15", "16", "17", "18", "19", "20"
                };
                if (intVal >= 0 && intVal <= 20) {
                    return smallInts[intVal];
                }
                return std::to_string(intVal);
            }
            
            // Optymalizacja dla liczb zmiennoprzecinkowych
            char buffer[32];
            int len = snprintf(buffer, sizeof(buffer), "%.6g", num);
            return std::string(buffer, len);
        }
        case ValueData::ARRAY: {
            if (value->array->empty()) return "[]";
            
            std::string result = "[";
            for (size_t i = 0; i < value->array->size(); ++i) {
                if (i > 0) result += ", ";
                result += valueToString((*value->array)[i]);
            }
            result += "]";
            return result;
        }
        case ValueData::OBJECT: {
            if (value->object->empty()) return "{}";
            
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

Interpreter::Interpreter() : returnFlag(false), returnValue(makeNumber(0.0)) {
    // Initialize all libraries
    MathLibrary::initialize();
    ColorLibrary::initialize();
    TimeLibrary::initialize();
    SystemLibrary::initialize();
}

void Interpreter::interpret(Program* program) {
    try {
        executeBlock(program->statements);
    } catch (const std::exception& e) {
        std::cerr << "Runtime error: " << e.what() << std::endl;
    }
}

bool Interpreter::valueToBoolean(const Value& value) {
    if (value->type == ValueData::NUMBER) {
        return value->number != 0.0;
    }
    
    switch (value->type) {
        case ValueData::STRING:
            return !value->string->empty();
        case ValueData::ARRAY:
            return !value->array->empty();
        case ValueData::OBJECT:
            return !value->object->empty();
        default:
            return false;
    }
}

// Optimize binary operations for performance
Value Interpreter::evaluate(Expression* expr) {
    if (auto numExpr = dynamic_cast<NumberExpression*>(expr)) {
        return makeNumber(numExpr->value);
    }
    
    if (auto strExpr = dynamic_cast<StringExpression*>(expr)) {
        return makeString(strExpr->value);
    }
    
    if (auto colorExpr = dynamic_cast<ColorStringExpression*>(expr)) {
        // Return color string with ANSI for immediate printing
        std::string result;
        
        // ANSI color codes - optimized lookup
        static const std::unordered_map<std::string, std::string> colorCodes = {
            {"red", "\033[31m"},
            {"green", "\033[32m"},
            {"yellow", "\033[33m"},
            {"blue", "\033[34m"},
            {"magenta", "\033[35m"},
            {"cyan", "\033[36m"},
            {"white", "\033[37m"},
            {"black", "\033[30m"},
            {"bright_red", "\033[91m"},
            {"bright_green", "\033[92m"},
            {"bright_yellow", "\033[93m"},
            {"bright_blue", "\033[94m"},
            {"bright_magenta", "\033[95m"},
            {"bright_cyan", "\033[96m"},
            {"bright_white", "\033[97m"}
        };
        
        auto it = colorCodes.find(colorExpr->color);
        if (it != colorCodes.end()) {
            result += it->second;
        }
        
        result += colorExpr->text + "\033[0m"; // Add text and reset color
        return makeString(result);
    }
    
    if (auto arrayExpr = dynamic_cast<ArrayExpression*>(expr)) {
        std::vector<Value> elements;
        elements.reserve(arrayExpr->elements.size()); // Pre-allocate for performance
        for (const auto& elem : arrayExpr->elements) {
            elements.push_back(evaluate(elem.get()));
        }
        return makeArray(elements);
    }
    
    if (auto objExpr = dynamic_cast<ObjectExpression*>(expr)) {
        std::unordered_map<std::string, Value> properties;
        properties.reserve(objExpr->properties.size()); // Pre-allocate for performance
        for (const auto& prop : objExpr->properties) {
            if (auto funcExpr = dynamic_cast<FunctionExpression*>(prop.second.get())) {
                // Store function in functions map with unique name
                std::string funcName = "__obj_method_" + prop.first + "_" + std::to_string(reinterpret_cast<uintptr_t>(funcExpr));
                functions[funcName] = Function(funcExpr->parameters, &funcExpr->body);
                properties[prop.first] = makeString(funcName); // Store function name as reference
            } else {
                properties[prop.first] = evaluate(prop.second.get());
            }
        }
        return makeObject(properties);
    }
    
    if (auto funcExpr = dynamic_cast<FunctionExpression*>(expr)) {
        // Create unique function name and store it
        std::string funcName = "__lambda_" + std::to_string(reinterpret_cast<uintptr_t>(funcExpr));
        functions[funcName] = Function(funcExpr->parameters, &funcExpr->body);
        return makeString(funcName);
    }
    
    if (auto idExpr = dynamic_cast<IdentifierExpression*>(expr)) {
        // Check if it's a saved value first
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
            return object; // Return the array itself when accessing .value
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
        // Szybka ścieżka dla operacji numerycznych
        const std::string& op = binExpr->operator_;
        
        // Optymalizacja dla najczęstszych operacji w pętlach
        if (op == "+" || op == "-" || op == "*" || op == "/" || 
            op == "<" || op == ">" || op == "<=" || op == ">=") {
            
            // Sprawdź czy to prosta operacja na identyfikatorach i liczbach
            auto* leftId = dynamic_cast<IdentifierExpression*>(binExpr->left.get());
            auto* rightNum = dynamic_cast<NumberExpression*>(binExpr->right.get());
            
            // Szybka ścieżka dla i + 1, i - 1, i * 2, i / 2 itp.
            if (leftId && rightNum) {
                try {
                    Value leftVal = getVariable(leftId->name);
                    if (leftVal->type == ValueData::NUMBER) {
                        if (op == "+") {
                            return makeNumber(leftVal->number + rightNum->value);
                        } else if (op == "-") {
                            return makeNumber(leftVal->number - rightNum->value);
                        } else if (op == "*") {
                            return makeNumber(leftVal->number * rightNum->value);
                        } else if (op == "/") {
                            if (rightNum->value == 0) throw std::runtime_error("Division by zero");
                            return makeNumber(leftVal->number / rightNum->value);
                        } else if (op == "<") {
                            return makeNumber(leftVal->number < rightNum->value ? 1.0 : 0.0);
                        } else if (op == ">") {
                            return makeNumber(leftVal->number > rightNum->value ? 1.0 : 0.0);
                        } else if (op == "<=") {
                            return makeNumber(leftVal->number <= rightNum->value ? 1.0 : 0.0);
                        } else if (op == ">=") {
                            return makeNumber(leftVal->number >= rightNum->value ? 1.0 : 0.0);
                        }
                    }
                } catch (...) {
                    // Fallback do standardowej ścieżki
                }
            }
            
            // Szybka ścieżka dla 1 + i, 2 * i itp.
            auto* leftNum = dynamic_cast<NumberExpression*>(binExpr->left.get());
            auto* rightId = dynamic_cast<IdentifierExpression*>(binExpr->right.get());
            
            if (leftNum && rightId) {
                try {
                    Value rightVal = getVariable(rightId->name);
                    if (rightVal->type == ValueData::NUMBER) {
                        if (op == "+") {
                            return makeNumber(leftNum->value + rightVal->number);
                        } else if (op == "-") {
                            return makeNumber(leftNum->value - rightVal->number);
                        } else if (op == "*") {
                            return makeNumber(leftNum->value * rightVal->number);
                        } else if (op == "/") {
                            if (rightVal->number == 0) throw std::runtime_error("Division by zero");
                            return makeNumber(leftNum->value / rightVal->number);
                        } else if (op == "<") {
                            return makeNumber(leftNum->value < rightVal->number ? 1.0 : 0.0);
                        } else if (op == ">") {
                            return makeNumber(leftNum->value > rightVal->number ? 1.0 : 0.0);
                        } else if (op == "<=") {
                            return makeNumber(leftNum->value <= rightVal->number ? 1.0 : 0.0);
                        } else if (op == ">=") {
                            return makeNumber(leftNum->value >= rightVal->number ? 1.0 : 0.0);
                        }
                    }
                } catch (...) {
                    // Fallback do standardowej ścieżki
                }
            }
            
            // Szybka ścieżka dla i + j, i - j itp. (dwie zmienne)
            if (leftId && rightId) {
                try {
                    Value leftVal = getVariable(leftId->name);
                    Value rightVal = getVariable(rightId->name);
                    
                    if (leftVal->type == ValueData::NUMBER && rightVal->type == ValueData::NUMBER) {
                        if (op == "+") {
                            return makeNumber(leftVal->number + rightVal->number);
                        } else if (op == "-") {
                            return makeNumber(leftVal->number - rightVal->number);
                        } else if (op == "*") {
                            return makeNumber(leftVal->number * rightVal->number);
                        } else if (op == "/") {
                            if (rightVal->number == 0) throw std::runtime_error("Division by zero");
                            return makeNumber(leftVal->number / rightVal->number);
                        } else if (op == "<") {
                            return makeNumber(leftVal->number < rightVal->number ? 1.0 : 0.0);
                        } else if (op == ">") {
                            return makeNumber(leftVal->number > rightVal->number ? 1.0 : 0.0);
                        } else if (op == "<=") {
                            return makeNumber(leftVal->number <= rightVal->number ? 1.0 : 0.0);
                        } else if (op == ">=") {
                            return makeNumber(leftVal->number >= rightVal->number ? 1.0 : 0.0);
                        }
                    }
                } catch (...) {
                    // Fallback do standardowej ścieżki
                }
            }
        }
        
        // Standardowa ścieżka dla innych operacji
        Value left = evaluate(binExpr->left.get());
        Value right = evaluate(binExpr->right.get());
        
        if (op == "+") {
            // Szybka ścieżka dla number + number
            if (left->type == ValueData::NUMBER && right->type == ValueData::NUMBER) {
                return makeNumber(left->number + right->number);
            }
            
            // Obsługa konkatenacji stringów
            if (left->type == ValueData::STRING || right->type == ValueData::STRING) {
                return makeString(valueToString(left) + valueToString(right));
            }
            
            return makeNumber(valueToNumber(left) + valueToNumber(right));
        }
        if (op == "-") {
            // Szybka ścieżka dla number - number
            if (left->type == ValueData::NUMBER && right->type == ValueData::NUMBER) {
                return makeNumber(left->number - right->number);
            }
            return makeNumber(valueToNumber(left) - valueToNumber(right));
        }
        if (op == "*") {
            // Szybka ścieżka dla number * number
            if (left->type == ValueData::NUMBER && right->type == ValueData::NUMBER) {
                return makeNumber(left->number * right->number);
            }
            return makeNumber(valueToNumber(left) * valueToNumber(right));
        }
        if (op == "/") {
            // Szybka ścieżka dla number / number
            if (left->type == ValueData::NUMBER && right->type == ValueData::NUMBER) {
                if (right->number == 0) throw std::runtime_error("Division by zero");
                return makeNumber(left->number / right->number);
            }
            double rightVal = valueToNumber(right);
            if (rightVal == 0) throw std::runtime_error("Division by zero");
            return makeNumber(valueToNumber(left) / rightVal);
        }
        if (op == "%") {
            // Szybka ścieżka dla number % number
            if (left->type == ValueData::NUMBER && right->type == ValueData::NUMBER) {
                return makeNumber(std::fmod(left->number, right->number));
            }
            return makeNumber(std::fmod(valueToNumber(left), valueToNumber(right)));
        }
        if (op == "==") {
            // Szybka ścieżka dla number == number
            if (left->type == ValueData::NUMBER && right->type == ValueData::NUMBER) {
                return makeNumber((left->number == right->number) ? 1.0 : 0.0);
            }
            return makeNumber((valueToString(left) == valueToString(right)) ? 1.0 : 0.0);
        }
        if (op == "!=") {
            // Szybka ścieżka dla number != number
            if (left->type == ValueData::NUMBER && right->type == ValueData::NUMBER) {
                return makeNumber((left->number != right->number) ? 1.0 : 0.0);
            }
            return makeNumber((valueToString(left) != valueToString(right)) ? 1.0 : 0.0);
        }
        if (op == "<") {
            // Szybka ścieżka dla number < number
            if (left->type == ValueData::NUMBER && right->type == ValueData::NUMBER) {
                return makeNumber((left->number < right->number) ? 1.0 : 0.0);
            }
            return makeNumber((valueToNumber(left) < valueToNumber(right)) ? 1.0 : 0.0);
        }
        if (op == ">") {
            // Szybka ścieżka dla number > number
            if (left->type == ValueData::NUMBER && right->type == ValueData::NUMBER) {
                return makeNumber((left->number > right->number) ? 1.0 : 0.0);
            }
            return makeNumber((valueToNumber(left) > valueToNumber(right)) ? 1.0 : 0.0);
        }
        if (op == "<=") {
            // Szybka ścieżka dla number <= number
            if (left->type == ValueData::NUMBER && right->type == ValueData::NUMBER) {
                return makeNumber((left->number <= right->number) ? 1.0 : 0.0);
            }
            return makeNumber((valueToNumber(left) <= valueToNumber(right)) ? 1.0 : 0.0);
        }
        if (op == ">=") {
            // Szybka ścieżka dla number >= number
            if (left->type == ValueData::NUMBER && right->type == ValueData::NUMBER) {
                return makeNumber((left->number >= right->number) ? 1.0 : 0.0);
            }
            return makeNumber((valueToNumber(left) >= valueToNumber(right)) ? 1.0 : 0.0);
        }
    }
    
    if (auto callExpr = dynamic_cast<CallExpression*>(expr)) {
        std::vector<Value> args;
        args.reserve(callExpr->arguments.size()); // Pre-allocate for performance
        for (const auto& arg : callExpr->arguments) {
            args.push_back(evaluate(arg.get()));
        }
        return callFunction(callExpr->functionName, args);
    }
    
    if (auto methodCallExpr = dynamic_cast<MethodCallExpression*>(expr)) {
        Value object = evaluate(methodCallExpr->object.get());
        std::vector<Value> args;
        args.reserve(methodCallExpr->arguments.size()); // Pre-allocate for performance
        for (const auto& arg : methodCallExpr->arguments) {
            args.push_back(evaluate(arg.get()));
        }
        return callMethod(object, methodCallExpr->methodName, args);
    }
    
    throw std::runtime_error("Unknown expression type");
}

// Optimize while loop execution
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
        // Optymalizacja dla typowych pętli numerycznych
        auto* condExpr = dynamic_cast<BinaryExpression*>(whileStmt->condition.get());
        
        // Sprawdź czy to prosta pętla numeryczna typu "while (i < 1000)"
        if (condExpr && (condExpr->operator_ == "<" || condExpr->operator_ == "<=" || 
                         condExpr->operator_ == ">" || condExpr->operator_ == ">=")) {
            
            auto* leftId = dynamic_cast<IdentifierExpression*>(condExpr->left.get());
            auto* rightNum = dynamic_cast<NumberExpression*>(condExpr->right.get());
            
            // Szybka ścieżka dla pętli z jedną zmienną i stałą granicą
            if (leftId && rightNum) {
                std::string varName = leftId->name;
                double limit = rightNum->value;
                
                // Pobierz początkową wartość
                Value varValue;
                try {
                    varValue = getVariable(varName);
                } catch (const std::exception&) {
                    // Zmienna nie istnieje, użyj standardowej ścieżki
                    goto regular_while_execution;
                }
                
                // Optymalizuj tylko jeśli zmienna jest liczbą
                if (varValue->type == ValueData::NUMBER) {
                    double value = varValue->number;
                    
                    // Sprawdź czy ciało pętli zawiera tylko proste inkrementacje
                    bool hasSimpleIncrement = false;
                    double incrementValue = 1.0;
                    
                    if (whileStmt->body.size() == 1 || whileStmt->body.size() == 2) {
                        // Sprawdź czy ostatnia instrukcja to i = i + 1 lub podobna
                        auto* lastStmt = dynamic_cast<AssignmentStatement*>(whileStmt->body.back().get());
                        if (lastStmt && lastStmt->variable == varName) {
                            auto* binExpr = dynamic_cast<BinaryExpression*>(lastStmt->value.get());
                            if (binExpr && binExpr->operator_ == "+") {
                                auto* leftIdExpr = dynamic_cast<IdentifierExpression*>(binExpr->left.get());
                                auto* rightNumExpr = dynamic_cast<NumberExpression*>(binExpr->right.get());
                                
                                if (leftIdExpr && leftIdExpr->name == varName && rightNumExpr) {
                                    hasSimpleIncrement = true;
                                    incrementValue = rightNumExpr->value;
                                }
                            }
                        }
                    }
                    
                    // Ultra-szybka ścieżka dla prostych pętli z inkrementacją
                    if (hasSimpleIncrement) {
                        // Wykonaj ciało pętli bez inkrementacji dla każdej iteracji
                        int maxIterations = 1000000; // Zabezpieczenie przed nieskończoną pętlą
                        int iterations = 0;
                        
                        while (((condExpr->operator_ == "<" && value < limit) ||
                               (condExpr->operator_ == "<=" && value <= limit) ||
                               (condExpr->operator_ == ">" && value > limit) ||
                               (condExpr->operator_ == ">=" && value >= limit)) && 
                               !returnFlag && iterations < maxIterations) {
                            
                            // Ustaw zmienną na aktualną wartość
                            setVariable(varName, makeNumber(value));
                            
                            // Wykonaj wszystkie instrukcje oprócz ostatniej (inkrementacji)
                            for (size_t i = 0; i < whileStmt->body.size() - 1; ++i) {
                                execute(whileStmt->body[i].get());
                                if (returnFlag) break;
                            }
                            
                            // Ręcznie wykonaj inkrementację
                            value += incrementValue;
                            iterations++;
                        }
                        
                        // Ustaw końcową wartość zmiennej
                        setVariable(varName, makeNumber(value));
                        return;
                    }
                    
                    // Standardowa optymalizacja dla innych pętli numerycznych
                    int maxIterations = 1000000; // Zabezpieczenie przed nieskończoną pętlą
                    int iterations = 0;
                    
                    while (((condExpr->operator_ == "<" && value < limit) ||
                           (condExpr->operator_ == "<=" && value <= limit) ||
                           (condExpr->operator_ == ">" && value > limit) ||
                           (condExpr->operator_ == ">=" && value >= limit)) && 
                           !returnFlag && iterations < maxIterations) {
                        
                        // Ustaw zmienną na aktualną wartość
                        setVariable(varName, makeNumber(value));
                        executeBlock(whileStmt->body);
                        
                        // Pobierz zaktualizowaną wartość po wykonaniu ciała
                        try {
                            varValue = getVariable(varName);
                            if (varValue->type != ValueData::NUMBER) {
                                // Typ zmiennej się zmienił, wyjdź z optymalizowanej pętli
                                break;
                            }
                            value = varValue->number;
                        } catch (const std::exception&) {
                            // Zmienna została usunięta, wyjdź z optymalizowanej pętli
                            break;
                        }
                        
                        iterations++;
                    }
                    return;
                }
            }
        }
        
    regular_while_execution:
        // Standardowe wykonanie pętli while (fallback)
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
            std::cout << "Command exited with code: " << result << std::endl;
        }
        return;
    }
    
    if (auto importStmt = dynamic_cast<ImportStatement*>(stmt)) {
        importLibrary(importStmt->libraryName);
        return;
    }
    
    if (auto saveStmt = dynamic_cast<SaveStatement*>(stmt)) {
        // Just mark that this save slot exists
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

// Improve library organization
void Interpreter::importLibrary(const std::string& libraryName) {
    // Add the library to the imported set
    importedLibraries.insert(libraryName);
    
    // Initialize standard libraries with their categories
    static const std::unordered_map<std::string, std::string> libraryCategories = {
        {"math", "Mathematics"},
        {"colors", "Text Formatting"},
        {"sc", "System and Console"},
        {"fs", "File System"},
        {"net", "Networking"},
        {"time", "Date and Time"},
        {"json", "Data Formats"},
        {"crypto", "Cryptography"}
    };
    
    // Print library information when imported
    auto it = libraryCategories.find(libraryName);
    if (it != libraryCategories.end()) {
        std::cout << "Imported " << libraryName << " library (" << it->second << " category)" << std::endl;
    }
}

// Optimize callFunction to handle library functions more efficiently
Value Interpreter::callFunction(const std::string& name, const std::vector<Value>& args) {
    // Cache dla często wywoływanych funkcji systemowych
    static std::unordered_set<std::string> systemFunctions = {
        "print", "ifu", "ec"
    };
    
    static std::unordered_set<std::string> mathFunctions = {
        "sqrt", "pow", "sin", "cos", "tan", "abs", "floor", "ceil", "round",
        "min", "max", "random", "log", "log10", "exp"
    };
    
    static std::unordered_set<std::string> colorFunctions = {
        "print", "colorize", "list_colors", "supports_color"
    };
    
    static std::unordered_set<std::string> timeFunctions = {
        "now", "timestamp", "format", "diff", "sleep", "date_parts", "add", "subtract",
        "is_leap_year", "days_in_month"
    };
    
    // Szybka ścieżka dla funkcji systemowych
    if (systemFunctions.count(name) > 0) {
        try {
            return SystemLibrary::callFunction(name, args);
        } catch (const std::runtime_error&) {
            // Funkcja nie znaleziona, kontynuuj
        }
    }
    
    // Szybka ścieżka dla funkcji matematycznych
    if (importedLibraries.count("math") && mathFunctions.count(name) > 0) {
        try {
            return MathLibrary::callFunction(name, args);
        } catch (const std::runtime_error&) {
            // Funkcja nie znaleziona, kontynuuj
        }
    }
    
    // Szybka ścieżka dla funkcji kolorów
    if (importedLibraries.count("colors") && colorFunctions.count(name) > 0) {
        try {
            return ColorLibrary::callFunction(name, args);
        } catch (const std::runtime_error&) {
            // Funkcja nie znaleziona, kontynuuj
        }
    }
    
    // Szybka ścieżka dla funkcji czasu
    if (importedLibraries.count("time") && timeFunctions.count(name) > 0) {
        try {
            return TimeLibrary::callFunction(name, args);
        } catch (const std::runtime_error&) {
            // Funkcja nie znaleziona, kontynuuj
        }
    }
    
    // Standardowa ścieżka dla bibliotek
    if (importedLibraries.count("math")) {
        try {
            return MathLibrary::callFunction(name, args);
        } catch (const std::runtime_error&) {
            // Funkcja nie znaleziona, kontynuuj
        }
    }
    
    if (importedLibraries.count("colors")) {
        try {
            return ColorLibrary::callFunction(name, args);
        } catch (const std::runtime_error&) {
            // Funkcja nie znaleziona, kontynuuj
        }
    }
    
    if (importedLibraries.count("time")) {
        try {
            return TimeLibrary::callFunction(name, args);
        } catch (const std::runtime_error&) {
            // Funkcja nie znaleziona, kontynuuj
        }
    }
    
    if (importedLibraries.count("sc")) {
        try {
            return SystemLibrary::callFunction(name, args);
        } catch (const std::runtime_error&) {
            // Funkcja nie znaleziona, kontynuuj
        }
    }
    
    // Wbudowane funkcje (zawsze dostępne)
    try {
        return SystemLibrary::callFunction(name, args);
    } catch (const std::runtime_error&) {
        // Funkcja nie znaleziona, kontynuuj
    }
    
    // Funkcje zdefiniowane przez użytkownika
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
    
    // Zapisz aktualny stan
    auto savedVars = variables;
    bool savedReturnFlag = returnFlag;
    Value savedReturnValue = returnValue;
    
    // Ustaw parametry
    for (size_t i = 0; i < func.parameters.size(); ++i) {
        setVariable(func.parameters[i], args[i]);
    }
    
    returnFlag = false;
    returnValue = makeNumber(0.0);
    
    // Wykonaj ciało funkcji
    if (func.body) {
        executeBlock(*func.body);
    }
    
    Value result = returnValue;
    
    // Przywróć stan
    variables = savedVars;
    returnFlag = savedReturnFlag;
    returnValue = savedReturnValue;
    
    return result;
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
        default:
            return "unknown";
    }
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
