#pragma once

#include "TerminalRenderer.hpp"
#include <vector>
#include <unordered_map>
#include <string>

class Shell {
public:
    Shell();
    ~Shell();

    void Start();
    void Stop();

private:
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
