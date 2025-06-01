#include <iostream>
#include <fstream>
#include <string>
#include "Tokenizer.h"
#include "Parser.h"
#include "Interpreter.h"

std::string readFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open file: " + filename);
    }
    
    std::string content;
    std::string line;
    while (std::getline(file, line)) {
        content += line + "\n";
    }
    
    return content;
}

void printUsage(const char* programName) {
    std::cout << "Usage: " << programName << " <file.rx>" << std::endl;
    std::cout << "       " << programName << " -i (interactive mode)" << std::endl;
}

void runInteractive() {
    std::cout << "RX Programming Language Interactive Mode" << std::endl;
    std::cout << "Type 'exit' to quit" << std::endl;
    
    Interpreter interpreter;
    std::string line;
    
    while (true) {
        std::cout << "rx> ";
        std::getline(std::cin, line);
        
        if (line == "exit") break;
        if (line.empty()) continue;
        
        try {
            Tokenizer tokenizer(line);
            std::vector<Token> tokens = tokenizer.tokenize();
            
            Parser parser(tokens);
            auto program = parser.parse();
            
            interpreter.interpret(program.get());
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printUsage(argv[0]);
        return 1;
    }
    
    std::string arg = argv[1];
    
    if (arg == "-i") {
        runInteractive();
        return 0;
    }
    
    if (arg.length() < 3 || arg.substr(arg.length() - 3) != ".rx") {
        std::cerr << "Error: File must have .rx extension" << std::endl;
        return 1;
    }
    
    try {
        std::string source = readFile(arg);
        
        Tokenizer tokenizer(source);
        std::vector<Token> tokens = tokenizer.tokenize();
        
        Parser parser(tokens);
        auto program = parser.parse();
        
        Interpreter interpreter;
        interpreter.interpret(program.get());
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
