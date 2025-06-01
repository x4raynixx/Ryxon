#include "Parser.h"
#include <stdexcept>
#include <iostream>

Parser::Parser(const std::vector<Token>& tokens) : tokens(tokens), current(0) {}

Token Parser::peek(int offset) const {
    size_t pos = current + offset;
    if (pos >= tokens.size()) return Token(TokenType::EOF_TOKEN);
    return tokens[pos];
}

Token Parser::advance() {
    if (!isAtEnd()) current++;
    return tokens[current - 1];
}

bool Parser::match(TokenType type) {
    if (check(type)) {
        advance();
        return true;
    }
    return false;
}

bool Parser::check(TokenType type) const {
    if (isAtEnd()) return false;
    return peek().type == type;
}

bool Parser::isAtEnd() const {
    return current >= tokens.size() || peek().type == TokenType::EOF_TOKEN;
}

void Parser::consume(TokenType type, const std::string& message) {
    if (check(type)) {
        advance();
        return;
    }
    
    Token current_token = peek();
    throw std::runtime_error(message + " at line " + std::to_string(current_token.line) + 
                           ", got " + TokenHelper::tokenTypeToString(current_token.type));
}

void Parser::skipNewlines() {
    while (match(TokenType::NEWLINE)) {
        // Skip all newlines
    }
}

std::unique_ptr<Program> Parser::parse() {
    return parseProgram();
}

std::unique_ptr<Program> Parser::parseProgram() {
    std::vector<StatementPtr> statements;
    
    while (!isAtEnd()) {
        // skip newlines
        if (match(TokenType::NEWLINE)) continue;
        
        StatementPtr stmt = parseStatement();
        if (stmt) {
            statements.push_back(std::move(stmt));
        }
    }
    
    return std::make_unique<Program>(std::move(statements));
}

StatementPtr Parser::parseStatement() {
    if (match(TokenType::EF)) {
        return parseFunctionStatement();
    }
    if (match(TokenType::IF)) {
        return parseIfStatement();
    }
    if (match(TokenType::WHILE)) {
        return parseWhileStatement();
    }
    if (match(TokenType::RET)) {
        return parseReturnStatement();
    }
    if (match(TokenType::EC)) {
        return parseSystemCallStatement();
    }
    if (match(TokenType::IMPORT)) {
        return parseImportStatement();
    }
    if (match(TokenType::NEW)) {
        return parseNewVariableStatement();
    }
    
    return parseAssignmentOrExpression();
}

StatementPtr Parser::parseFunctionStatement() {
    consume(TokenType::IDENTIFIER, "Expected function name");
    std::string name = tokens[current - 1].value;
    
    consume(TokenType::LPAREN, "Expected '(' after function name");
    
    std::vector<std::string> parameters;
    if (!check(TokenType::RPAREN)) {
        do {
            consume(TokenType::IDENTIFIER, "Expected parameter name");
            parameters.push_back(tokens[current - 1].value);
        } while (match(TokenType::COMMA));
    }
    
    consume(TokenType::RPAREN, "Expected ')' after parameters");
    
    std::vector<StatementPtr> body = parseBlock();
    
    return std::make_unique<FunctionStatement>(name, std::move(parameters), std::move(body));
}

ExpressionPtr Parser::parseFunctionExpression() {
    consume(TokenType::LPAREN, "Expected '(' after 'ef'");
    
    std::vector<std::string> parameters;
    if (!check(TokenType::RPAREN)) {
        do {
            consume(TokenType::IDENTIFIER, "Expected parameter name");
            parameters.push_back(tokens[current - 1].value);
        } while (match(TokenType::COMMA));
    }
    
    consume(TokenType::RPAREN, "Expected ')' after parameters");
    
    std::vector<StatementPtr> body = parseBlock();
    
    return std::make_unique<FunctionExpression>(std::move(parameters), std::move(body));
}

StatementPtr Parser::parseIfStatement() {
    consume(TokenType::LPAREN, "Expected '(' after 'if'");
    ExpressionPtr condition = parseExpression();
    consume(TokenType::RPAREN, "Expected ')' after if condition");
    
    std::vector<StatementPtr> thenBranch = parseBlock();
    std::vector<StatementPtr> elseBranch;
    
    if (match(TokenType::DIFF)) {
        if (check(TokenType::IF)) {
            // handle "diff if" (else if)
            StatementPtr elseIfStmt = parseStatement();
            elseBranch.push_back(std::move(elseIfStmt));
        } else {
            // handle regular "diff" (else)
            elseBranch = parseBlock();
        }
    }
    
    return std::make_unique<IfStatement>(std::move(condition), std::move(thenBranch), std::move(elseBranch));
}

StatementPtr Parser::parseWhileStatement() {
    consume(TokenType::LPAREN, "Expected '(' after 'while'");
    ExpressionPtr condition = parseExpression();
    consume(TokenType::RPAREN, "Expected ')' after while condition");
    
    std::vector<StatementPtr> body = parseBlock();
    
    return std::make_unique<WhileStatement>(std::move(condition), std::move(body));
}

StatementPtr Parser::parseReturnStatement() {
    ExpressionPtr value = nullptr;
    if (!check(TokenType::SEMICOLON) && !check(TokenType::NEWLINE)) {
        value = parseExpression();
    }
    
    if (!match(TokenType::SEMICOLON)) {
        match(TokenType::NEWLINE);
    }
    
    return std::make_unique<ReturnStatement>(std::move(value));
}

StatementPtr Parser::parseSystemCallStatement() {
    consume(TokenType::LPAREN, "Expected '(' after 'ec'");
    ExpressionPtr command = parseExpression();
    consume(TokenType::RPAREN, "Expected ')' after ec command");
    
    if (!match(TokenType::SEMICOLON)) {
        match(TokenType::NEWLINE);
    }
    
    return std::make_unique<SystemCallStatement>(std::move(command));
}

StatementPtr Parser::parseImportStatement() {
    consume(TokenType::IDENTIFIER, "Expected library name after '^'");
    std::string libraryName = tokens[current - 1].value;
    
    if (!match(TokenType::SEMICOLON)) {
        match(TokenType::NEWLINE);
    }
    
    return std::make_unique<ImportStatement>(libraryName);
}

StatementPtr Parser::parseNewVariableStatement() {
    consume(TokenType::IDENTIFIER, "Expected variable name after 'new'");
    std::string variable = tokens[current - 1].value;
    
    consume(TokenType::ASSIGN, "Expected '=' after variable name");
    
    ExpressionPtr value = parseExpression();
    
    if (!match(TokenType::SEMICOLON)) {
        match(TokenType::NEWLINE);
    }
    
    return std::make_unique<NewVariableStatement>(variable, std::move(value));
}

StatementPtr Parser::parseAssignmentOrExpression() {
    if (check(TokenType::IDENTIFIER) && peek(1).type == TokenType::ASSIGN) {
        std::string variable = advance().value;
        advance(); // consume '='
        ExpressionPtr value = parseExpression();
        
        if (!match(TokenType::SEMICOLON)) {
            match(TokenType::NEWLINE);
        }
        
        return std::make_unique<AssignmentStatement>(variable, std::move(value));
    }
    
    ExpressionPtr expr = parseExpression();
    if (!match(TokenType::SEMICOLON)) {
        match(TokenType::NEWLINE);
    }
    
    return std::make_unique<ExpressionStatement>(std::move(expr));
}

std::vector<StatementPtr> Parser::parseBlock() {
    consume(TokenType::LBRACE, "Expected '{'");
    
    std::vector<StatementPtr> statements;
    
    while (!check(TokenType::RBRACE) && !isAtEnd()) {
        if (match(TokenType::NEWLINE)) continue;
        
        StatementPtr stmt = parseStatement();
        if (stmt) {
            statements.push_back(std::move(stmt));
        }
    }
    
    consume(TokenType::RBRACE, "Expected '}'");
    
    return statements;
}

ExpressionPtr Parser::parseExpression() {
    return parseComparison();
}

ExpressionPtr Parser::parseComparison() {
    ExpressionPtr expr = parseTerm();
    
    while (match(TokenType::EQUAL) || match(TokenType::NOT_EQUAL) ||
           match(TokenType::GREATER) || match(TokenType::GREATER_EQUAL) ||
           match(TokenType::LESS) || match(TokenType::LESS_EQUAL)) {
        
        std::string operator_ = tokens[current - 1].value;
        ExpressionPtr right = parseTerm();
        expr = std::make_unique<BinaryExpression>(std::move(expr), operator_, std::move(right));
    }
    
    return expr;
}

ExpressionPtr Parser::parseTerm() {
    ExpressionPtr expr = parseFactor();
    
    while (match(TokenType::PLUS) || match(TokenType::MINUS)) {
        std::string operator_ = tokens[current - 1].value;
        ExpressionPtr right = parseFactor();
        expr = std::make_unique<BinaryExpression>(std::move(expr), operator_, std::move(right));
    }
    
    return expr;
}

ExpressionPtr Parser::parseFactor() {
    ExpressionPtr expr = parsePrimary();
    
    while (match(TokenType::MULTIPLY) || match(TokenType::DIVIDE) || match(TokenType::MODULO)) {
        std::string operator_ = tokens[current - 1].value;
        ExpressionPtr right = parsePrimary();
        expr = std::make_unique<BinaryExpression>(std::move(expr), operator_, std::move(right));
    }
    
    return expr;
}

ExpressionPtr Parser::parseStringInterpolation() {
    std::vector<ExpressionPtr> parts;
    
    // parse string with interpolation
    while (!isAtEnd()) {
        if (match(TokenType::STRING)) {
            // regular string part
            std::string text = tokens[current - 1].value;
            parts.push_back(std::make_unique<StringExpression>(text));
        } else if (match(TokenType::INTERPOLATE_START)) {
            // << expression >>
            ExpressionPtr expr = parseExpression();
            parts.push_back(std::move(expr));
            consume(TokenType::INTERPOLATE_END, "Expected '>>' after interpolated expression");
        } else {
            break;
        }
    }
    
    return std::make_unique<InterpolationExpression>(std::move(parts));
}

ExpressionPtr Parser::parsePrimary() {
    if (match(TokenType::NUMBER)) {
        double value = std::stod(tokens[current - 1].value);
        return std::make_unique<NumberExpression>(value);
    }
    
    // handle string interpolation: "text" << expr >> "more text"
    if (check(TokenType::STRING)) {
        // look ahead to see if this is followed by interpolation
        size_t lookahead = current + 1;
        bool hasInterpolation = false;
        
        // check if theres interpolation after this string
        while (lookahead < tokens.size()) {
            if (tokens[lookahead].type == TokenType::INTERPOLATE_START) {
                hasInterpolation = true;
                break;
            } else if (tokens[lookahead].type == TokenType::COMMA || 
                      tokens[lookahead].type == TokenType::RPAREN ||
                      tokens[lookahead].type == TokenType::NEWLINE ||
                      tokens[lookahead].type == TokenType::SEMICOLON) {
                break;
            }
            lookahead++;
        }
        
        if (hasInterpolation) {
            return parseStringInterpolation();
        } else {
            advance(); // consume the string
            return std::make_unique<StringExpression>(tokens[current - 1].value);
        }
    }
    
    // Handle arrays [1, 2, 3]
    if (match(TokenType::LBRACKET)) {
        std::vector<ExpressionPtr> elements;
        
        skipNewlines(); // skip newlines after opening bracket
        
        if (!check(TokenType::RBRACKET)) {
            do {
                skipNewlines();
                elements.push_back(parseExpression());
                skipNewlines();
            } while (match(TokenType::COMMA));
        }
        
        skipNewlines(); // skip newlines before closing bracket
        consume(TokenType::RBRACKET, "Expected ']' after array elements");
        return std::make_unique<ArrayExpression>(std::move(elements));
    }
    
    // Handle objects {key: value, key2: value2}
    if (match(TokenType::LBRACE)) {
        std::unordered_map<std::string, ExpressionPtr> properties;
        
        skipNewlines(); // skip newlines after opening brace
        
        while (!check(TokenType::RBRACE) && !isAtEnd()) {
            skipNewlines(); // skip newlines before property
            
            if (check(TokenType::RBRACE)) break; // Exit if we hit closing brace
            
            consume(TokenType::IDENTIFIER, "Expected property name");
            std::string key = tokens[current - 1].value;
            
            consume(TokenType::COLON, "Expected ':' after property name");
            
            skipNewlines(); // Skip newlines after colon
            
            // Check if this is a function definition
            if (check(TokenType::EF)) {
                advance(); // consume 'ef'
                ExpressionPtr value = parseFunctionExpression();
                properties[key] = std::move(value);
            } else {
                ExpressionPtr value = parseExpression();
                properties[key] = std::move(value);
            }
            
            skipNewlines(); // Skip newlines after value
            
            // Check for comma or end of object
            if (match(TokenType::COMMA)) {
                skipNewlines(); // Skip newlines after comma
            } else {
                break; // No comma, so we're done with properties
            }
        }
        
        skipNewlines(); // Skip newlines before closing brace
        consume(TokenType::RBRACE, "Expected '}' after object properties");
        return std::make_unique<ObjectExpression>(std::move(properties));
    }
    
    // Handle function expressions: ef(a, b) { ... }
    if (match(TokenType::EF)) {
        return parseFunctionExpression();
    }
    
    // Handle color syntax: c@color"text"
    if (check(TokenType::IDENTIFIER) && peek().value == "c") {
        // Look ahead to see if this is a color expression
        if (current + 1 < tokens.size() && tokens[current + 1].type == TokenType::AT) {
            advance(); // consume 'c'
            advance(); // consume '@'
            
            if (!check(TokenType::IDENTIFIER)) {
                throw std::runtime_error("Expected color name after 'c@'");
            }
            std::string color = advance().value;
            
            if (!check(TokenType::STRING)) {
                throw std::runtime_error("Expected string after color name");
            }
            std::string text = advance().value;
            
            return std::make_unique<ColorStringExpression>(color, text);
        }
    }
    
    if (match(TokenType::TYPEOF)) {
        consume(TokenType::LPAREN, "Expected '(' after 'typeof'");
        ExpressionPtr expr = parseExpression();
        consume(TokenType::RPAREN, "Expected ')' after typeof expression");
        return std::make_unique<TypeofExpression>(std::move(expr));
    }
    
    if (match(TokenType::IDENTIFIER)) {
        std::string name = tokens[current - 1].value;
        ExpressionPtr expr = std::make_unique<IdentifierExpression>(name);
        return parsePostfix(std::move(expr));
    }
    
    if (match(TokenType::LPAREN)) {
        ExpressionPtr expr = parseExpression();
        consume(TokenType::RPAREN, "Expected ')' after expression");
        return expr;
    }
    
    throw std::runtime_error("Unexpected token in expression at line " + std::to_string(peek().line));
}

ExpressionPtr Parser::parsePostfix(ExpressionPtr expr) {
    while (true) {
        if (match(TokenType::DOT)) {
            consume(TokenType::IDENTIFIER, "Expected property name after '.'");
            std::string property = tokens[current - 1].value;
            
            // check if this is a method call
            if (check(TokenType::LPAREN)) {
                advance(); // consume '('
                std::vector<ExpressionPtr> arguments;
                
                if (!check(TokenType::RPAREN)) {
                    do {
                        arguments.push_back(parseExpression());
                    } while (match(TokenType::COMMA));
                }
                
                consume(TokenType::RPAREN, "Expected ')' after method arguments");
                expr = std::make_unique<MethodCallExpression>(std::move(expr), property, std::move(arguments));
            } else {
                expr = std::make_unique<MemberExpression>(std::move(expr), property);
            }
        } else if (match(TokenType::LBRACKET)) {
            ExpressionPtr index = parseExpression();
            consume(TokenType::RBRACKET, "Expected ']' after array index");
            expr = std::make_unique<IndexExpression>(std::move(expr), std::move(index));
        } else if (match(TokenType::LPAREN)) {
            // Function call
            auto identifier = dynamic_cast<IdentifierExpression*>(expr.get());
            if (!identifier) {
                throw std::runtime_error("Invalid function call");
            }
            
            std::string functionName = identifier->name;
            std::vector<ExpressionPtr> arguments;
            
            if (!check(TokenType::RPAREN)) {
                do {
                    arguments.push_back(parseExpression());
                } while (match(TokenType::COMMA));
            }
            
            consume(TokenType::RPAREN, "Expected ')' after arguments");
            
            expr.release(); // release the original expression
            expr = std::make_unique<CallExpression>(functionName, std::move(arguments));
        } else {
            break;
        }
    }
    
    return expr;
}
