#include "ColorLibrary.h"
#include <iostream>
#include <unordered_map>
#include <cstdlib>
#include <cstring>

#ifdef _WIN32
    #include <windows.h>
    #ifndef FOREGROUND_RED
        #define FOREGROUND_RED 0x0004
    #endif
    #ifndef FOREGROUND_GREEN
        #define FOREGROUND_GREEN 0x0002
    #endif
    #ifndef FOREGROUND_BLUE
        #define FOREGROUND_BLUE 0x0001
    #endif
    #ifndef FOREGROUND_INTENSITY
        #define FOREGROUND_INTENSITY 0x0008
    #endif
    #ifndef BACKGROUND_RED
        #define BACKGROUND_RED 0x0040
    #endif
    #ifndef BACKGROUND_GREEN
        #define BACKGROUND_GREEN 0x0020
    #endif
    #ifndef BACKGROUND_BLUE
        #define BACKGROUND_BLUE 0x0010
    #endif
    #ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
        #define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
    #endif
#endif

static bool windowsColorInitialized = false;

void ColorLibrary::initialize() {
    #ifdef _WIN32
    if (!windowsColorInitialized) {
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        DWORD mode = 0;
        GetConsoleMode(hConsole, &mode);
        SetConsoleMode(hConsole, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
        windowsColorInitialized = true;
    }
    #endif
}

bool ColorLibrary::supportsColor() {
    #ifdef _WIN32
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD mode = 0;
    return GetConsoleMode(hConsole, &mode) && (mode & ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    #else
    const char* term = std::getenv("TERM");
    const char* colorterm = std::getenv("COLORTERM");
    return (term && (std::string(term).find("color") != std::string::npos ||
                     std::string(term).find("xterm") != std::string::npos ||
                     std::string(term) == "linux")) ||
           (colorterm && std::strlen(colorterm) > 0);
    #endif
}

Value ColorLibrary::callFunction(const std::string& name, const std::vector<Value>& args) {
    static const std::unordered_map<std::string, std::pair<std::string, int>> colorCodes = {
        {"red", {"\033[31m", FOREGROUND_RED}},
        {"green", {"\033[32m", FOREGROUND_GREEN}},
        {"yellow", {"\033[33m", FOREGROUND_RED | FOREGROUND_GREEN}},
        {"blue", {"\033[34m", FOREGROUND_BLUE}},
        {"magenta", {"\033[35m", FOREGROUND_RED | FOREGROUND_BLUE}},
        {"cyan", {"\033[36m", FOREGROUND_GREEN | FOREGROUND_BLUE}},
        {"white", {"\033[37m", FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE}},
        {"black", {"\033[30m", 0}},
        {"bright_red", {"\033[91m", FOREGROUND_RED | FOREGROUND_INTENSITY}},
        {"bright_green", {"\033[92m", FOREGROUND_GREEN | FOREGROUND_INTENSITY}},
        {"bright_yellow", {"\033[93m", (FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY)}},
        {"bright_blue", {"\033[94m", FOREGROUND_BLUE | FOREGROUND_INTENSITY}},
        {"bright_magenta", {"\033[95m", (FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY)}},
        {"bright_cyan", {"\033[96m", (FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY)}},
        {"bright_white", {"\033[97m", (FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY)}},
        {"reset", {"\033[0m", FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE}}
    };

    if (name == "print") {
        #ifdef _WIN32
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        GetConsoleScreenBufferInfo(hConsole, &csbi);
        WORD defaultAttrs = csbi.wAttributes;
        #endif

        for (size_t i = 0; i < args.size(); ++i) {
            if (i > 0) std::cout << " ";
            std::string text = valueToString(args[i]);
            
            // Check for color syntax c@color"text"
            if (text.length() > 3 && text[0] == 'c' && text[1] == '@') {
                size_t colorEnd = text.find('"');
                if (colorEnd != std::string::npos && colorEnd > 2) {
                    std::string color = text.substr(2, colorEnd - 2);
                    std::string content = text.substr(colorEnd + 1);
                    if (!content.empty() && content.back() == '"') {
                        content.pop_back();
                    }
                    
                    auto it = colorCodes.find(color);
                    if (it != colorCodes.end()) {
                        #ifdef _WIN32
                        SetConsoleTextAttribute(hConsole, it->second.second);
                        std::cout << content;
                        SetConsoleTextAttribute(hConsole, defaultAttrs);
                        #else
                        std::cout << it->second.first << content << "\033[0m";
                        #endif
                    } else {
                        std::cout << content;
                    }
                } else {
                    std::cout << text;
                }
            } else {
                std::cout << text;
            }
        }
        std::cout << std::endl;
        return makeNumber(0.0);
    }
    
    if (name == "colorize") {
        if (args.size() != 2) throw std::runtime_error("colorize expects 2 arguments: color and text");
        
        std::string color = valueToString(args[0]);
        std::string text = valueToString(args[1]);
        
        auto it = colorCodes.find(color);
        if (it != colorCodes.end()) {
            return makeString(it->second.first + text + "\033[0m");
        }
        
        return makeString(text);
    }
    
    if (name == "list_colors") {
        std::string result = "Available colors:\n";
        for (const auto& pair : colorCodes) {
            if (pair.first != "reset") {
                result += pair.first + "\n";
            }
        }
        std::cout << result;
        return makeString(result);
    }
    
    if (name == "supports_color") {
        return makeNumber(supportsColor() ? 1.0 : 0.0);
    }
    
    throw std::runtime_error("Unknown color function: " + name);
}
