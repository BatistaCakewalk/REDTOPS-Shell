#include "../header/CommandParser.h"
#include <sstream>


std::vector<std::string> CommandParser::Tokenize(const std::string& line) {
    std::istringstream iss(line);
    std::vector<std::string> tokens;
    std::string tok;
    
    if (line.find_first_not_of(" \t\n\r") == std::string::npos) return {};
    
    while (iss >> tok) tokens.push_back(tok);
    return tokens;
}
