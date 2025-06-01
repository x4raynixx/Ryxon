#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "AST.h"
#include <unordered_map>
#include <unordered_set>
#include <variant>
#include <functional>
#include <cmath>
#include <vector>
#include <memory>

// frwd declaration for recursive types
struct ValueData;
using Value = std::shared_ptr<ValueData>;

// valuie data structure that can hold different types
struct ValueData {
    enum Type { NUMBER, STRING, ARRAY, OBJECT };
    Type type;
    
    union {
        double number;
        std::string* string;
        std::vector<Value>* array;
        std::unordered_map<std::string, Value>* object;
    };
    
    ValueData(double n) : type(NUMBER), number(n) {}
    ValueData(const std::string& s) : type(STRING), string(new std::string(s)) {}
    ValueData(const std::vector<Value>& a) : type(ARRAY), array(new std::vector<Value>(a)) {}
    ValueData(const std::unordered_map<std::string, Value>& o) : type(OBJECT), object(new std::unordered_map<std::string, Value>(o)) {}
    
    ~ValueData() {
        switch (type) {
            case STRING: delete string; break;
            case ARRAY: delete array; break;
            case OBJECT: delete object; break;
            default: break;
        }
    }
    
    // copy constructor
    ValueData(const ValueData& other) : type(other.type) {
        switch (type) {
            case NUMBER: number = other.number; break;
            case STRING: string = new std::string(*other.string); break;
            case ARRAY: array = new std::vector<Value>(*other.array); break;
            case OBJECT: object = new std::unordered_map<std::string, Value>(*other.object); break;
        }
    }
    
    // assigment operator
    ValueData& operator=(const ValueData& other) {
        if (this != &other) {
            this->~ValueData();
            new (this) ValueData(other);
        }
        return *this;
    }
};

// helper functions for Value creation
Value makeNumber(double n);
Value makeString(const std::string& s);
Value makeArray(const std::vector<Value>& a);
Value makeObject(const std::unordered_map<std::string, Value>& o);

struct Function {
    std::vector<std::string> parameters;
    std::vector<StatementPtr>* body;
    std::unordered_map<std::string, Value> closure; // for object method context
    
    Function() : body(nullptr) {}
    Function(const std::vector<std::string>& params, std::vector<StatementPtr>* b)
        : parameters(params), body(b) {}
    Function(const std::vector<std::string>& params, std::vector<StatementPtr>* b, const std::unordered_map<std::string, Value>& c)
        : parameters(params), body(b), closure(c) {}
    Function(const Function& other) : parameters(other.parameters), body(other.body), closure(other.closure) {}
    Function(Function&& other) noexcept : parameters(std::move(other.parameters)), body(other.body), closure(std::move(other.closure)) {
        other.body = nullptr;
    }
    Function& operator=(const Function& other) {
        if (this != &other) {
            parameters = other.parameters;
            body = other.body;
            closure = other.closure;
        }
        return *this;
    }
    Function& operator=(Function&& other) noexcept {
        if (this != &other) {
            parameters = std::move(other.parameters);
            body = other.body;
            closure = std::move(other.closure);
            other.body = nullptr;
        }
        return *this;
    }
};

class Interpreter {
private:
    std::unordered_map<std::string, Value> variables;
    std::unordered_map<std::string, Function> functions;
    std::unordered_map<std::string, Value> savedValues;  // for save system
    std::unordered_set<std::string> importedLibraries;   // track imported libraries
    bool returnFlag;
    Value returnValue;
    
    Value evaluate(Expression* expr);
    void execute(Statement* stmt);
    void executeBlock(const std::vector<StatementPtr>& statements);
    
    Value callFunction(const std::string& name, const std::vector<Value>& args);
    Value callMethod(const Value& object, const std::string& methodName, const std::vector<Value>& args);
    std::string valueToString(const Value& value);
    double valueToNumber(const Value& value);
    bool valueToBoolean(const Value& value);
    std::string getValueType(const Value& value);
    
    // library functions
    void importLibrary(const std::string& libraryName);
    Value callMathFunction(const std::string& name, const std::vector<Value>& args);
    Value callEcFunction(const std::string& name, const std::vector<Value>& args);
    Value callScFunction(const std::string& name, const std::vector<Value>& args);
    Value callColorFunction(const std::string& name, const std::vector<Value>& args);
    
public:
    Interpreter();
    void interpret(Program* program);
    void setVariable(const std::string& name, const Value& value);
    Value getVariable(const std::string& name);
};

#endif
