#include "../header/CommandParser.h"
#include <sstream>


std::vector<std::string> CommandParser::Tokenize(const std::string& line) {
    std::istringstream iss(line);
    std::vector<std::string> tokens;
    std::string tok;
    while (iss >> tok) tokens.push_back(tok);
    return tokens;
}