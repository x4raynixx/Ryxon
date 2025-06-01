#ifndef TOKENIZER_H
#define TOKENIZER_H

#include "Token.h"
#include <vector>
#include <string>

class Tokenizer {
private:
    std::string source;
    size_t current;
    int line;
    int column;
    
    char peek(int offset = 0) const;
    char advance();
    void skipWhitespace();
    void skipComment();
    Token makeNumber();
    Token makeString();
    Token makeIdentifier();
    Token makeOperator();
    
public:
    Tokenizer(const std::string& source);
    std::vector<Token> tokenize();
    Token nextToken();
    bool isAtEnd() const;
};

#endif
