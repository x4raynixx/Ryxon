#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "AST.h"
#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <vector>
#include <string>

// Forward declarations
struct ValueData;
using Value = std::shared_ptr<ValueData>;

// Value types
struct ValueData {
    enum Type { STRING, NUMBER, ARRAY, OBJECT };
    Type type;
    
    std::unique_ptr<std::string> string;
    double number;
    std::unique_ptr<std::vector<Value>> array;
    std::unique_ptr<std::unordered_map<std::string, Value>> object;
    
    ValueData(const std::string& s) : type(STRING), string(std::make_unique<std::string>(s)), number(0) {}
    ValueData(double n) : type(NUMBER), number(n) {}
    ValueData(const std::vector<Value>& a) : type(ARRAY), number(0), array(std::make_unique<std::vector<Value>>(a)) {}
    ValueData(const std::unordered_map<std::string, Value>& o) : type(OBJECT), number(0), object(std::make_unique<std::unordered_map<std::string, Value>>(o)) {}
};

// Global helper functions
Value makeNumber(double n);
Value makeString(const std::string& s);
Value makeArray(const std::vector<Value>& a);
Value makeObject(const std::unordered_map<std::string, Value>& o);
double valueToNumber(const Value& value);
std::string valueToString(const Value& value);

struct Function {
    std::vector<std::string> parameters;
    const std::vector<StatementPtr>* body;
    
    Function() : body(nullptr) {}
    Function(const std::vector<std::string>& params, const std::vector<StatementPtr>* b) 
        : parameters(params), body(b) {}
};

class Interpreter {
private:
    std::unordered_map<std::string, Value> variables;
    std::unordered_map<std::string, Function> functions;
    std::unordered_set<std::string> importedLibraries;
    std::unordered_map<std::string, Value> savedValues;
    
    bool returnFlag;
    Value returnValue;

public:
    Interpreter();
    void interpret(Program* program);
    
    Value evaluate(Expression* expr);
    void execute(Statement* stmt);
    void executeBlock(const std::vector<StatementPtr>& statements);
    
    Value callFunction(const std::string& name, const std::vector<Value>& args);
    Value callMethod(const Value& object, const std::string& methodName, const std::vector<Value>& args);
    
    void importLibrary(const std::string& libraryName);
    
    bool valueToBoolean(const Value& value);
    std::string getValueType(const Value& value);
    
    void setVariable(const std::string& name, const Value& value);
    Value getVariable(const std::string& name);
};

#endif
