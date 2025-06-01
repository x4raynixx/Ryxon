#include "Token.h"

std::unordered_map<std::string, TokenType> TokenHelper::keywords = {
    {"ef", TokenType::EF},
    {"while", TokenType::WHILE},
    {"if", TokenType::IF},
    {"diff", TokenType::DIFF},
    {"ec", TokenType::EC},        // changed to lowercase
    {"ret", TokenType::RET},      // changed from return
    {"new", TokenType::NEW},
    {"Save", TokenType::SAVE},
    {"Message", TokenType::MESSAGE},
    {"typeof", TokenType::TYPEOF}
};

std::string TokenHelper::tokenTypeToString(TokenType type) {
    switch (type) {
        case TokenType::NUMBER: return "NUMBER";
        case TokenType::STRING: return "STRING";
        case TokenType::IDENTIFIER: return "IDENTIFIER";
        case TokenType::EF: return "EF";
        case TokenType::WHILE: return "WHILE";
        case TokenType::IF: return "IF";
        case TokenType::DIFF: return "DIFF";
        case TokenType::EC: return "EC";
        case TokenType::RET: return "RET";
        case TokenType::NEW: return "NEW";
        case TokenType::SAVE: return "SAVE";
        case TokenType::MESSAGE: return "MESSAGE";
        case TokenType::TYPEOF: return "TYPEOF";
        case TokenType::IMPORT: return "IMPORT";
        case TokenType::PLUS: return "PLUS";
        case TokenType::MINUS: return "MINUS";
        case TokenType::MULTIPLY: return "MULTIPLY";
        case TokenType::DIVIDE: return "DIVIDE";
        case TokenType::MODULO: return "MODULO";
        case TokenType::ASSIGN: return "ASSIGN";
        case TokenType::ARROW: return "ARROW";
        case TokenType::EQUAL: return "EQUAL";
        case TokenType::NOT_EQUAL: return "NOT_EQUAL";
        case TokenType::LESS: return "LESS";
        case TokenType::GREATER: return "GREATER";
        case TokenType::LESS_EQUAL: return "LESS_EQUAL";
        case TokenType::GREATER_EQUAL: return "GREATER_EQUAL";
        case TokenType::INTERPOLATE_START: return "INTERPOLATE_START";
        case TokenType::INTERPOLATE_END: return "INTERPOLATE_END";
        case TokenType::LPAREN: return "LPAREN";
        case TokenType::RPAREN: return "RPAREN";
        case TokenType::LBRACE: return "LBRACE";
        case TokenType::RBRACE: return "RBRACE";
        case TokenType::LBRACKET: return "LBRACKET";
        case TokenType::RBRACKET: return "RBRACKET";
        case TokenType::SEMICOLON: return "SEMICOLON";
        case TokenType::COMMA: return "COMMA";
        case TokenType::DOT: return "DOT";
        case TokenType::COLON: return "COLON";
        case TokenType::NEWLINE: return "NEWLINE";
        case TokenType::EOF_TOKEN: return "EOF";
        case TokenType::AT: return "AT";
        default: return "INVALID";
    }
}

bool TokenHelper::isKeyword(const std::string& word) {
    return keywords.find(word) != keywords.end();
}

TokenType TokenHelper::getKeywordType(const std::string& word) {
    auto it = keywords.find(word);
    return (it != keywords.end()) ? it->second : TokenType::IDENTIFIER;
}
