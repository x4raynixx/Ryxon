#ifndef TOKEN_H
#define TOKEN_H

#include <string>
#include <unordered_map>

enum class TokenType {
    // Literals
    NUMBER,
    STRING,
    IDENTIFIER,
    
    // Keywords
    EF,           // function definition
    WHILE,        // while loop
    IF,           // if statement
    DIFF,         // else (different)
    EC,           // execute call (system) - now lowercase
    RET,          // return statement - changed from RETURN
    NEW,          // new keyword for variable declaration
    SAVE,         // Save keyword (deprecated)
    MESSAGE,      // Message keyword (deprecated)
    TYPEOF,       // typeof keyword
    
    // Library imports
    IMPORT,       // ^ symbol for imports
    
    // Operators
    PLUS,         // +
    MINUS,        // -
    MULTIPLY,     // *
    DIVIDE,       // /
    MODULO,       // %
    ASSIGN,       // =
    ARROW,        // => for save operations
    
    // Comparison
    EQUAL,        // ==
    NOT_EQUAL,    // !=
    LESS,         // <
    GREATER,      // >
    LESS_EQUAL,   // <=
    GREATER_EQUAL,// >=
    
    // String interpolation
    INTERPOLATE_START, // <<
    INTERPOLATE_END,   // >>
    
    // Delimiters
    LPAREN,       // (
    RPAREN,       // )
    LBRACE,       // {
    RBRACE,       // }
    LBRACKET,     // [
    RBRACKET,     // ]
    SEMICOLON,    // ;
    COMMA,        // ,
    DOT,          // .
    COLON,        // :

    AT,           // @ symbol for colors
    
    // Special
    NEWLINE,
    EOF_TOKEN,
    INVALID
};

struct Token {
    TokenType type;
    std::string value;
    int line;
    int column;
    
    Token(TokenType t = TokenType::INVALID, const std::string& v = "", int l = 0, int c = 0)
        : type(t), value(v), line(l), column(c) {}
};

class TokenHelper {
public:
    static std::unordered_map<std::string, TokenType> keywords;
    static std::string tokenTypeToString(TokenType type);
    static bool isKeyword(const std::string& word);
    static TokenType getKeywordType(const std::string& word);
};

#endif
