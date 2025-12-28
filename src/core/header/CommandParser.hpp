#pragma once
#include <string>
#include <vector>


class CommandParser {
public:
    // splits "scan 192.168.0.1 -p 1-100" -> {"scan","192.168.0.1","-p","1-100"}
    static std::vector<std::string> Tokenize(const std::string& line);
};