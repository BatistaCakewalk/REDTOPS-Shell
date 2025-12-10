#pragma once

#include <string>


class TerminalRenderer {
public:
    TerminalRenderer();
    ~TerminalRenderer();


    void Init();
    void Clear();
    void Print(const std::string& s);
    void PrintLine(const std::string& s);
    void SetPrompt(const std::string& p);
    void DrawBootScreen(const std::string& boottxt);


private:
    std::string prompt_;
};

