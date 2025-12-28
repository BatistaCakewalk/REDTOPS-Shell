#pragma once
#include <string>
#include <iostream>
#include <thread>
#include <chrono>

// ANSI Color Codes for the "Hacknet" look
namespace Color {
    const std::string RESET   = "\033[0m";
    const std::string GREEN   = "\033[38;5;46m"; // Bright Matrix Green
    const std::string CYAN    = "\033[38;5;51m"; // Tech Blue
    const std::string RED     = "\033[38;5;196m"; // Alert Red
    const std::string AMBER   = "\033[38;5;214m"; // Warning Gold
    const std::string DIM     = "\033[2m";       // Faded text
}

class TerminalRenderer {
public:
    static TerminalRenderer& Instance() {
        static TerminalRenderer instance;
        return instance;
    }

    // Delete copy/assignment for Singleton pattern
    TerminalRenderer(const TerminalRenderer&) = delete;
    TerminalRenderer& operator=(const TerminalRenderer&) = delete;

    void Init() {
        Clear();
    }

    // Standard immediate print
    void PrintLine(const std::string& text, const std::string& color = Color::RESET) {
        std::cout << color << text << Color::RESET << std::endl;
    }

    void PrintError(const std::string& error_message) {
        PrintLine("ERROR: " + error_message, Color::RED);
    }

    void PrintWarning(const std::string& warning_message) {
        PrintLine("WARNING: " + warning_message, Color::AMBER);
    }

    // Hacknet-style typewriter effect for boot-up or critical alerts
    void Typewrite(const std::string& text, int delay_ms = 15, const std::string& color = Color::RESET) {
        std::cout << color;
        for (char c : text) {
            std::cout << c << std::flush;
            std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));
        }
        std::cout << Color::RESET << std::endl;
    }

    void Clear() {
        // \033[2J clears screen, \033[H moves cursor to top-left
        std::cout << "\033[2J\033[H" << std::flush;
    }

    void DrawBootScreen(const std::string& screen) {
        Clear();
        // Print the boot screen in Cyan for that high-tech feel
        PrintLine(screen, Color::CYAN);
    }

    // Accessors for the Shell's prompt
    void SetPrompt(const std::string& p) { prompt_ = p; }
    std::string GetPrompt() const { return prompt_; }

private:
    TerminalRenderer() : prompt_("RT> ") {}
    std::string prompt_;
};
