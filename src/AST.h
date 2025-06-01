#ifndef AST_H
#define AST_H

#include <memory>
#include <vector>
#include <string>
#include <variant>
#include <unordered_map>

class ASTNode;
class Expression;
class Statement;

using ASTNodePtr = std::unique_ptr<ASTNode>;
using ExpressionPtr = std::unique_ptr<Expression>;
using StatementPtr = std::unique_ptr<Statement>;

// base AST node
class ASTNode {
public:
    virtual ~ASTNode() = default;
    virtual std::string toString() const = 0;
};

// base expresion
class Expression : public ASTNode {
public:
    virtual ~Expression() = default;
};

// base statemenmt
class Statement : public ASTNode {
public:
    virtual ~Statement() = default;
};

// expressions
class NumberExpression : public Expression {
public:
    double value;
    NumberExpression(double val) : value(val) {}
    std::string toString() const override;
};

class StringExpression : public Expression {
public:
    std::string value;
    StringExpression(const std::string& val) : value(val) {}
    std::string toString() const override;
};

class ColorStringExpression : public Expression {
public:
    std::string color;
    std::string text;
    ColorStringExpression(const std::string& c, const std::string& t) : color(c), text(t) {}
    std::string toString() const override;
};

class ArrayExpression : public Expression {
public:
    std::vector<ExpressionPtr> elements;
    ArrayExpression(std::vector<ExpressionPtr> elems) : elements(std::move(elems)) {}
    std::string toString() const override;
};

class FunctionExpression : public Expression {
public:
    std::vector<std::string> parameters;
    std::vector<StatementPtr> body;
    
    FunctionExpression(std::vector<std::string> params, std::vector<StatementPtr> b)
        : parameters(std::move(params)), body(std::move(b)) {}
    std::string toString() const override;
};

class ObjectExpression : public Expression {
public:
    std::unordered_map<std::string, ExpressionPtr> properties;
    ObjectExpression(std::unordered_map<std::string, ExpressionPtr> props) : properties(std::move(props)) {}
    std::string toString() const override;
};

class IdentifierExpression : public Expression {
public:
    std::string name;
    IdentifierExpression(const std::string& n) : name(n) {}
    std::string toString() const override;
};

class MemberExpression : public Expression {
public:
    ExpressionPtr object;
    std::string property;
    MemberExpression(ExpressionPtr obj, const std::string& prop) : object(std::move(obj)), property(prop) {}
    std::string toString() const override;
};

class IndexExpression : public Expression {
public:
    ExpressionPtr object;
    ExpressionPtr index;
    IndexExpression(ExpressionPtr obj, ExpressionPtr idx) : object(std::move(obj)), index(std::move(idx)) {}
    std::string toString() const override;
};

class BinaryExpression : public Expression {
public:
    ExpressionPtr left;
    std::string operator_;
    ExpressionPtr right;
    
    BinaryExpression(ExpressionPtr l, const std::string& op, ExpressionPtr r)
        : left(std::move(l)), operator_(op), right(std::move(r)) {}
    std::string toString() const override;
};

class CallExpression : public Expression {
public:
    std::string functionName;
    std::vector<ExpressionPtr> arguments;
    
    CallExpression(const std::string& name, std::vector<ExpressionPtr> args)
        : functionName(name), arguments(std::move(args)) {}
    std::string toString() const override;
};

class MethodCallExpression : public Expression {
public:
    ExpressionPtr object;
    std::string methodName;
    std::vector<ExpressionPtr> arguments;
    
    MethodCallExpression(ExpressionPtr obj, const std::string& method, std::vector<ExpressionPtr> args)
        : object(std::move(obj)), methodName(method), arguments(std::move(args)) {}
    std::string toString() const override;
};

class TypeofExpression : public Expression {
public:
    ExpressionPtr expression;
    TypeofExpression(ExpressionPtr expr) : expression(std::move(expr)) {}
    std::string toString() const override;
};

class InterpolationExpression : public Expression {
public:
    std::vector<ExpressionPtr> parts;
    InterpolationExpression(std::vector<ExpressionPtr> p) : parts(std::move(p)) {}
    std::string toString() const override;
};

class ExpressionStatement : public Statement {
public:
    ExpressionPtr expression;
    ExpressionStatement(ExpressionPtr expr) : expression(std::move(expr)) {}
    std::string toString() const override;
};

class AssignmentStatement : public Statement {
public:
    std::string variable;
    ExpressionPtr value;
    
    AssignmentStatement(const std::string& var, ExpressionPtr val)
        : variable(var), value(std::move(val)) {}
    std::string toString() const override;
};

class NewVariableStatement : public Statement {
public:
    std::string variable;
    ExpressionPtr value;
    
    NewVariableStatement(const std::string& var, ExpressionPtr val)
        : variable(var), value(std::move(val)) {}
    std::string toString() const override;
};

class IfStatement : public Statement {
public:
    ExpressionPtr condition;
    std::vector<StatementPtr> thenBranch;
    std::vector<StatementPtr> elseBranch;
    
    IfStatement(ExpressionPtr cond, std::vector<StatementPtr> then, std::vector<StatementPtr> else_)
        : condition(std::move(cond)), thenBranch(std::move(then)), elseBranch(std::move(else_)) {}
    std::string toString() const override;
};

class WhileStatement : public Statement {
public:
    ExpressionPtr condition;
    std::vector<StatementPtr> body;
    
    WhileStatement(ExpressionPtr cond, std::vector<StatementPtr> b)
        : condition(std::move(cond)), body(std::move(b)) {}
    std::string toString() const override;
};

class FunctionStatement : public Statement {
public:
    std::string name;
    std::vector<std::string> parameters;
    std::vector<StatementPtr> body;
    
    FunctionStatement(const std::string& n, std::vector<std::string> params, std::vector<StatementPtr> b)
        : name(n), parameters(std::move(params)), body(std::move(b)) {}
    std::string toString() const override;
};

class ReturnStatement : public Statement {
public:
    ExpressionPtr value;
    ReturnStatement(ExpressionPtr val) : value(std::move(val)) {}
    std::string toString() const override;
};

class SystemCallStatement : public Statement {
public:
    ExpressionPtr command;
    SystemCallStatement(ExpressionPtr cmd) : command(std::move(cmd)) {}
    std::string toString() const override;
};

class ImportStatement : public Statement {
public:
    std::string libraryName;
    ImportStatement(const std::string& lib) : libraryName(lib) {}
    std::string toString() const override;
};

class SaveStatement : public Statement {
public:
    std::string saveName;
    SaveStatement(const std::string& name) : saveName(name) {}
    std::string toString() const override;
};

class MessageStatement : public Statement {
public:
    std::string messageName;
    ExpressionPtr value;
    MessageStatement(const std::string& name, ExpressionPtr val)
        : messageName(name), value(std::move(val)) {}
    std::string toString() const override;
};

class Program : public ASTNode {
public:
    std::vector<StatementPtr> statements;
    Program(std::vector<StatementPtr> stmts) : statements(std::move(stmts)) {}
    std::string toString() const override;
};

#endif
