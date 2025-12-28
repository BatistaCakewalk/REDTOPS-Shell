#pragma once

#include "TerminalRenderer.hpp"
#include <vector>
#include <unordered_map>
#include <string>

class Shell {
public:
    static Shell& Instance(); // Static method to access the single instance

    // Delete copy constructor and assignment operator
    Shell(const Shell&) = delete;
    Shell& operator=(const Shell&) = delete;

    ~Shell(); // Public destructor

    void Start();
    void Stop();

    std::string GetPromptString() const;

private:
    Shell(); // Private constructor

    bool running_ = false;

    // Reference to the singleton
    TerminalRenderer& renderer_ = TerminalRenderer::Instance();

    // ===== QOL Additions =====
    std::vector<std::string> history_;                 // Command history
    int history_index_ = -1;                           // Index for navigating history
    std::unordered_map<std::string,std::string> aliases_; // Aliases
    bool verbose_mode_ = false;                        // Global verbose flag
    // ========================

    void RegisterBuiltins();
    void MainLoop();

    // QOL helpers
    std::string ApplyAliases(const std::string& input); // replaces input with alias if exists
    std::string TabComplete(const std::string& prefix); // completes command names starting with prefix
};

