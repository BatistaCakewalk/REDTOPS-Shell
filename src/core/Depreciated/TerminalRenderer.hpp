#pragma once
#include <string>
#include <iostream>

class TerminalRenderer {
public:
    static TerminalRenderer& Instance() {
        static TerminalRenderer instance;
        return instance;
    }

    TerminalRenderer(const TerminalRenderer&) = delete;
    TerminalRenderer& operator=(const TerminalRenderer&) = delete;

    void Init() {}

    void PrintLine(const std::string& text) {
        std::cout << text << std::endl;
    }

    void Clear() {
        std::cout << "\033[2J\033[H";
    }

    void SetPrompt(const std::string& p) { prompt_ = p; }
    std::string GetPrompt() const { return prompt_; } // ✅ make sure this exists

    void DrawBootScreen(const std::string& screen) {
        std::cout << screen << std::endl;
    }

private:
    TerminalRenderer() {}
    std::string prompt_ = "RT> ";
};
