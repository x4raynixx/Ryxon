#include "AST.h"
#include <sstream>

std::string NumberExpression::toString() const {
    return std::to_string(value);
}

std::string StringExpression::toString() const {
    return "\"" + value + "\"";
}

std::string ColorStringExpression::toString() const {
    return "c@" + color + "\"" + text + "\"";
}

std::string ArrayExpression::toString() const {
    std::string result = "[";
    for (size_t i = 0; i < elements.size(); ++i) {
        if (i > 0) result += ", ";
        result += elements[i]->toString();
    }
    result += "]";
    return result;
}

std::string FunctionExpression::toString() const {
    std::string result = "ef(";
    for (size_t i = 0; i < parameters.size(); ++i) {
        if (i > 0) result += ", ";
        result += parameters[i];
    }
    result += ") {\n";
    for (const auto& stmt : body) {
        result += "  " + stmt->toString() + "\n";
    }
    result += "}";
    return result;
}

std::string ObjectExpression::toString() const {
    std::string result = "{";
    bool first = true;
    for (const auto& pair : properties) {
        if (!first) result += ", ";
        result += pair.first + ": " + pair.second->toString();
        first = false;
    }
    result += "}";
    return result;
}

std::string IdentifierExpression::toString() const {
    return name;
}

std::string MemberExpression::toString() const {
    return object->toString() + "." + property;
}

std::string IndexExpression::toString() const {
    return object->toString() + "[" + index->toString() + "]";
}

std::string BinaryExpression::toString() const {
    return "(" + left->toString() + " " + operator_ + " " + right->toString() + ")";
}

std::string CallExpression::toString() const {
    std::string result = functionName + "(";
    for (size_t i = 0; i < arguments.size(); ++i) {
        if (i > 0) result += ", ";
        result += arguments[i]->toString();
    }
    result += ")";
    return result;
}

std::string MethodCallExpression::toString() const {
    std::string result = object->toString() + "." + methodName + "(";
    for (size_t i = 0; i < arguments.size(); ++i) {
        if (i > 0) result += ", ";
        result += arguments[i]->toString();
    }
    result += ")";
    return result;
}

std::string TypeofExpression::toString() const {
    return "typeof(" + expression->toString() + ")";
}

std::string InterpolationExpression::toString() const {
    std::string result = "";
    for (const auto& part : parts) {
        result += part->toString();
    }
    return result;
}

std::string ExpressionStatement::toString() const {
    return expression->toString() + ";";
}

std::string AssignmentStatement::toString() const {
    return variable + " = " + value->toString() + ";";
}

std::string NewVariableStatement::toString() const {
    return "new " + variable + " = " + value->toString() + ";";
}

std::string IfStatement::toString() const {
    std::string result = "if (" + condition->toString() + ") {\n";
    for (const auto& stmt : thenBranch) {
        result += "  " + stmt->toString() + "\n";
    }
    result += "}";
    if (!elseBranch.empty()) {
        result += " diff {\n";
        for (const auto& stmt : elseBranch) {
            result += "  " + stmt->toString() + "\n";
        }
        result += "}";
    }
    return result;
}

std::string WhileStatement::toString() const {
    std::string result = "while (" + condition->toString() + ") {\n";
    for (const auto& stmt : body) {
        result += "  " + stmt->toString() + "\n";
    }
    result += "}";
    return result;
}

std::string FunctionStatement::toString() const {
    std::string result = "ef " + name + "(";
    for (size_t i = 0; i < parameters.size(); ++i) {
        if (i > 0) result += ", ";
        result += parameters[i];
    }
    result += ") {\n";
    for (const auto& stmt : body) {
        result += "  " + stmt->toString() + "\n";
    }
    result += "}";
    return result;
}

std::string ReturnStatement::toString() const {
    return "ret " + (value ? value->toString() : "") + ";";
}

std::string SystemCallStatement::toString() const {
    return "ec(" + command->toString() + ");";
}

std::string ImportStatement::toString() const {
    return "^" + libraryName;
}

std::string SaveStatement::toString() const {
    return "new Save(\"" + saveName + "\");";
}

std::string MessageStatement::toString() const {
    return "new Message(\"" + messageName + "\") = " + value->toString() + ";";
}

std::string Program::toString() const {
    std::string result;
    for (const auto& stmt : statements) {
        result += stmt->toString() + "\n";
    }
    return result;
}
