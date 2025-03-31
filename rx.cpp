#include <iostream>
#include <fstream>
#include <string>
#include <unordered_map>
#include <sstream>
#include <regex>
#include <cstdlib>
#include <ctime>

std::unordered_map<std::string, int> variables;

std::string formatString(const std::string& str) {
    std::string result = str;
    std::regex rgx("%\\((.*?)\\)");
    std::smatch match;
    while (std::regex_search(result, match, rgx)) {
        std::string var = match[1].str();
        if (variables.find(var) != variables.end())
            result.replace(match.position(0), match.length(0), std::to_string(variables[var]));
        else
            result.replace(match.position(0), match.length(0), "undefined");
    }
    return result;
}

int randomInRange(int min, int max) {
    return min + rand() % (max - min + 1);
}

void printError(const std::string& message, int lineNumber, const std::string& filename) {
    std::cerr << "\033[1;31mError at line " << lineNumber << " in file " << filename << ": " << message << "\033[0m" << std::endl;
}

int parseValue(const std::string& str) {
    try {
        return std::stoi(str);
    }
    catch (...) {
        if (variables.find(str) != variables.end()) {
            return variables[str];
        }
        printError("Variable not defined: " + str, -1, "unknown");
        return 0;
    }
}

void executeRX(const std::string& code, const std::string& filename) {
    std::istringstream stream(code);
    std::string line;
    int lineNumber = 0;
    while (getline(stream, line)) {
        lineNumber++;
        line.erase(0, line.find_first_not_of(" \t"));
        line.erase(line.find_last_not_of(" \t") + 1);

        if (line.find("log(") == 0) {
            size_t startPos = line.find("'") + 1;
            size_t endPos = line.find("'", startPos);
            if (startPos == std::string::npos || endPos == std::string::npos) {
                printError("Invalid syntax in log statement!", lineNumber, filename);
                continue;
            }
            std::cout << formatString(line.substr(startPos, endPos - startPos)) << std::endl;
        }
        else if (line.find("batch(") == 0) {
            size_t startPos = line.find("'") + 1;
            size_t endPos = line.find("'", startPos);
            if (startPos == std::string::npos || endPos == std::string::npos) {
                printError("Invalid syntax in batch statement!", lineNumber, filename);
                continue;
            }
            system(line.substr(startPos, endPos - startPos).c_str());
        }
        else if (line.find("=") != std::string::npos) {
            size_t equalsPos = line.find("=");
            std::string varName = line.substr(0, equalsPos);
            varName.erase(0, varName.find_first_not_of(" \t"));
            varName.erase(varName.find_last_not_of(" \t") + 1);
            std::string expression = line.substr(equalsPos + 1);
            expression.erase(0, expression.find_first_not_of(" \t"));
            expression.erase(expression.find_last_not_of(" \t") + 1);

            if (expression.find("add(") == 0 || expression.find("sub(") == 0) {
                char op = (expression.find("add(") == 0) ? '+' : '-';
                size_t startPos = expression.find("(") + 1;
                size_t endPos = expression.find(")", startPos);
                if (startPos == std::string::npos || endPos == std::string::npos) {
                    printError("Invalid syntax for math operation!", lineNumber, filename);
                    continue;
                }
                std::string expr = expression.substr(startPos, endPos - startPos);
                size_t delimPos = expr.find(",");
                if (delimPos == std::string::npos) {
                    printError("Invalid parameters for math operation!", lineNumber, filename);
                    continue;
                }
                std::string var1 = expr.substr(0, delimPos);
                std::string var2 = expr.substr(delimPos + 1);
                var1.erase(0, var1.find_first_not_of(" \t"));
                var1.erase(var1.find_last_not_of(" \t") + 1);
                var2.erase(0, var2.find_first_not_of(" \t"));
                var2.erase(var2.find_last_not_of(" \t") + 1);

                int val1 = parseValue(var1);
                int val2 = parseValue(var2);
                variables[varName] = (op == '+') ? (val1 + val2) : (val1 - val2);
            }
            else if (expression.find("random(") == 0) {
                size_t startPos = expression.find("(") + 1;
                size_t endPos = expression.find(")", startPos);
                if (startPos == std::string::npos || endPos == std::string::npos) {
                    printError("Invalid syntax for random operation!", lineNumber, filename);
                    continue;
                }
                std::string range = expression.substr(startPos, endPos - startPos);
                size_t commaPos = range.find(",");
                if (commaPos == std::string::npos) {
                    printError("Invalid range for random operation!", lineNumber, filename);
                    continue;
                }
                try {
                    int min = std::stoi(range.substr(0, commaPos));
                    int max = std::stoi(range.substr(commaPos + 1));
                    variables[varName] = randomInRange(min, max);
                }
                catch (...) {
                    printError("Invalid values for random range!", lineNumber, filename);
                }
            }
            else {
                variables[varName] = parseValue(expression);
            }
        }
    }
}
int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "Usage: rx <filename.rx>" << std::endl;
        return 1;
    }
    std::ifstream file(argv[1]);
    if (!file) {
        std::cerr << "Error: Cannot open file " << argv[1] << std::endl;
        return 1;
    }
    std::string code, line;
    while (getline(file, line))
        code += line + "\n";
    executeRX(code, argv[1]);
    return 0;
}