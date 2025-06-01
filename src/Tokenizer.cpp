#include "Tokenizer.h"
#include <cctype>
#include <stdexcept>

Tokenizer::Tokenizer(const std::string& source) 
    : source(source), current(0), line(1), column(1) {}

char Tokenizer::peek(int offset) const {
    size_t pos = current + offset;
    if (pos >= source.length()) return '\0';
    return source[pos];
}

char Tokenizer::advance() {
    if (current >= source.length()) return '\0';
    char c = source[current++];
    if (c == '\n') {
        line++;
        column = 1;
    } else {
        column++;
    }
    return c;
}

void Tokenizer::skipWhitespace() {
    while (!isAtEnd()) {
        char c = peek();
        if (c == ' ' || c == '\t' || c == '\r') {
            advance();
        } else if (c == '/' && peek(1) == '/') {
            skipComment();
        } else {
            break;
        }
    }
}

void Tokenizer::skipComment() {
    while (!isAtEnd() && peek() != '\n') {
        advance();
    }
}

Token Tokenizer::makeNumber() {
    int startLine = line, startColumn = column;
    std::string value;
    
    while (!isAtEnd() && (std::isdigit(peek()) || peek() == '.')) {
        value += advance();
    }
    
    return Token(TokenType::NUMBER, value, startLine, startColumn);
}

Token Tokenizer::makeString() {
    int startLine = line, startColumn = column;
    std::string value;
    char quote = advance(); // skip opening quote (either " or ')
    
    while (!isAtEnd() && peek() != quote) {
        if (peek() == '\\') {
            advance(); // skip backslash
            char escaped = advance();
            switch (escaped) {
                case 'n': value += '\n'; break;
                case 't': value += '\t'; break;
                case 'r': value += '\r'; break;
                case '\\': value += '\\'; break;
                case '"': value += '"'; break;
                case '\'': value += '\''; break;
                default: value += escaped; break;
            }
        } else {
            value += advance();
        }
    }
    
    if (isAtEnd()) {
        throw std::runtime_error("Unterminated string at line " + std::to_string(startLine));
    }
    
    advance(); // skip closing quote
    return Token(TokenType::STRING, value, startLine, startColumn);
}

Token Tokenizer::makeIdentifier() {
    int startLine = line, startColumn = column;
    std::string value;
    
    while (!isAtEnd() && (std::isalnum(peek()) || peek() == '_')) {
        value += advance();
    }
    
    TokenType type = TokenHelper::isKeyword(value) ? 
        TokenHelper::getKeywordType(value) : TokenType::IDENTIFIER;
    
    return Token(type, value, startLine, startColumn);
}

Token Tokenizer::makeOperator() {
    int startLine = line, startColumn = column;
    char c = advance();
    
    switch (c) {
        case '+': return Token(TokenType::PLUS, "+", startLine, startColumn);
        case '-': return Token(TokenType::MINUS, "-", startLine, startColumn);
        case '*': return Token(TokenType::MULTIPLY, "*", startLine, startColumn);
        case '/': return Token(TokenType::DIVIDE, "/", startLine, startColumn);
        case '%': return Token(TokenType::MODULO, "%", startLine, startColumn);
        case '(': return Token(TokenType::LPAREN, "(", startLine, startColumn);
        case ')': return Token(TokenType::RPAREN, ")", startLine, startColumn);
        case '{': return Token(TokenType::LBRACE, "{", startLine, startColumn);
        case '}': return Token(TokenType::RBRACE, "}", startLine, startColumn);
        case '[': return Token(TokenType::LBRACKET, "[", startLine, startColumn);
        case ']': return Token(TokenType::RBRACKET, "]", startLine, startColumn);
        case ';': return Token(TokenType::SEMICOLON, ";", startLine, startColumn);
        case ',': return Token(TokenType::COMMA, ",", startLine, startColumn);
        case '.': return Token(TokenType::DOT, ".", startLine, startColumn);
        case ':': return Token(TokenType::COLON, ":", startLine, startColumn);
        case '^': return Token(TokenType::IMPORT, "^", startLine, startColumn);
        case '@': return Token(TokenType::AT, "@", startLine, startColumn);
        case '=':
            if (peek() == '=') {
                advance();
                return Token(TokenType::EQUAL, "==", startLine, startColumn);
            } else if (peek() == '>') {
                advance();
                return Token(TokenType::ARROW, "=>", startLine, startColumn);
            }
            return Token(TokenType::ASSIGN, "=", startLine, startColumn);
        case '!':
            if (peek() == '=') {
                advance();
                return Token(TokenType::NOT_EQUAL, "!=", startLine, startColumn);
            }
            break;
        case '<':
            if (peek() == '=') {
                advance();
                return Token(TokenType::LESS_EQUAL, "<=", startLine, startColumn);
            } else if (peek() == '<') {
                advance();
                return Token(TokenType::INTERPOLATE_START, "<<", startLine, startColumn);
            }
            return Token(TokenType::LESS, "<", startLine, startColumn);
        case '>':
            if (peek() == '=') {
                advance();
                return Token(TokenType::GREATER_EQUAL, ">=", startLine, startColumn);
            } else if (peek() == '>') {
                advance();
                return Token(TokenType::INTERPOLATE_END, ">>", startLine, startColumn);
            }
            return Token(TokenType::GREATER, ">", startLine, startColumn);
    }
    
    return Token(TokenType::INVALID, std::string(1, c), startLine, startColumn);
}

std::vector<Token> Tokenizer::tokenize() {
    std::vector<Token> tokens;
    
    while (!isAtEnd()) {
        Token token = nextToken();
        if (token.type != TokenType::INVALID) {
            tokens.push_back(token);
        }
    }
    
    tokens.push_back(Token(TokenType::EOF_TOKEN, "", line, column));
    return tokens;
}

Token Tokenizer::nextToken() {
    skipWhitespace();
    
    if (isAtEnd()) {
        return Token(TokenType::EOF_TOKEN, "", line, column);
    }
    
    char c = peek();
    
    if (c == '\n') {
        advance();
        return Token(TokenType::NEWLINE, "\n", line - 1, column);
    }
    
    if (std::isdigit(c)) {
        return makeNumber();
    }
    
    if (c == '"' || c == '\'') {  // support both quote types
        return makeString();
    }
    
    if (std::isalpha(c) || c == '_') {
        return makeIdentifier();
    }
    
    return makeOperator();
}

bool Tokenizer::isAtEnd() const {
    return current >= source.length();
}
