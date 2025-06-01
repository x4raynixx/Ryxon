#ifndef PARSER_H
#define PARSER_H

#include "Token.h"
#include "AST.h"
#include <vector>
#include <memory>

class Parser {
private:
    std::vector<Token> tokens;
    size_t current;
    
    Token peek(int offset = 0) const;
    Token advance();
    bool match(TokenType type);
    bool check(TokenType type) const;
    bool isAtEnd() const;
    void consume(TokenType type, const std::string& message);
    void skipNewlines();
    
    // Parsing methods
    std::unique_ptr<Program> parseProgram();
    StatementPtr parseStatement();
    StatementPtr parseFunctionStatement();
    ExpressionPtr parseFunctionExpression();
    StatementPtr parseIfStatement();
    StatementPtr parseWhileStatement();
    StatementPtr parseReturnStatement();
    StatementPtr parseSystemCallStatement();
    StatementPtr parseImportStatement();
    StatementPtr parseNewVariableStatement();
    StatementPtr parseAssignmentOrExpression();
    std::vector<StatementPtr> parseBlock();
    
    ExpressionPtr parseExpression();
    ExpressionPtr parseComparison();
    ExpressionPtr parseTerm();
    ExpressionPtr parseFactor();
    ExpressionPtr parsePrimary();
    ExpressionPtr parsePostfix(ExpressionPtr expr);
    ExpressionPtr parseStringInterpolation();
    
public:
    Parser(const std::vector<Token>& tokens);
    std::unique_ptr<Program> parse();
};

#endif
