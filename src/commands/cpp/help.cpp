#include "../headers/help.hpp"
#include "../../core/header/TerminalRenderer.hpp"
#include "../../core/header/CommandRegistry.hpp"
#include <memory>
#include <iostream>
#include <unordered_map>
#include <map>
#include <vector>

// Command metadata including description, category, and usage example
struct CommandMeta {
    std::string description;
    std::string category;
    std::string usage;
};

// Define metadata for each command
static std::unordered_map<std::string, CommandMeta> command_metadata = {
    // ---------------- Built-in commands ----------------
    {"help",     {"Show this help message", "Built-in", "help [command]"}},
    {"exit",     {"Exit the shell", "Built-in", "exit"}},
    {"cls",      {"Shortcut for clearing screen", "Built-in", "clear"}},
    {"clear",    {"Clear the terminal screen", "Built-in", "clear"}},
    {"cd",       {"Change current directory", "Built-in", "cd [PATH]"}},
    {"pwd",      {"Print working directory", "Built-in", "pwd"}},

    // ---------------- Filesystem commands ----------------
    {"ls",       {"List files and directories", "Filesystem", "ls [-a] [-l] [PATH]"}},
    {"cat",      {"Display file contents", "Filesystem", "cat <file>"}},
    {"mkdir",    {"Create directories", "Filesystem", "mkdir <dir>"}},
    {"rm",       {"Remove files or directories", "Filesystem", "rm [-r] <file|dir>"}},
    {"cp",       {"Copy files or directories", "Filesystem", "cp [-r] <source> <dest>"}},
    {"mv",       {"Move or rename files/directories", "Filesystem", "mv <source> <dest>"}},

    // ---------------- Network commands ----------------
    {"ping",     {"Check connectivity to a host", "Network", "ping 8.8.8.8"}},
    {"netinfo",  {"Display network information", "Network", "netinfo"}},
    {"sysinfo",  {"Display system information", "Network", "sysinfo"}},
    {"trace",    {"Perform a traceroute to a host", "Network", "trace <host>"}},
    {"netscan",  {"Scan local subnet for live hosts", "Network", "netscan [subnet]"}},
    {"portscan", {"Scan ports on a host", "Network", "portscan <host> [start_port] [end_port]"}}
};


// ANSI color codes
constexpr const char* COLOR_RESET = "\033[0m";
constexpr const char* COLOR_YELLOW = "\033[33m";
constexpr const char* COLOR_CYAN = "\033[36m";

std::string HelpCommand::Name() const {
    return "help";
}

void HelpCommand::Execute(const std::vector<std::string>& args) {
    auto& registry = CommandRegistry::Instance();
    auto& renderer = TerminalRenderer::Instance();

    if (!args.empty()) {
        // Show detailed help for a specific command
        auto* cmd = registry.Get(args[0]);
        if (cmd) {
            renderer.PrintLine(std::string(COLOR_YELLOW) + "Help for command: " + args[0] + COLOR_RESET);
            auto it = command_metadata.find(args[0]);
            if (it != command_metadata.end()) {
                renderer.PrintLine("  Category: " + std::string(COLOR_CYAN) + it->second.category + COLOR_RESET);
                renderer.PrintLine("  Description: " + it->second.description);
                renderer.PrintLine("  Usage: " + it->second.usage);
            } else {
                renderer.PrintLine("  No additional info available.");
            }
        } else {
            renderer.PrintLine("Unknown command: " + args[0]);
        }
        return;
    }

    // General help: group commands by category
    std::map<std::string, std::vector<std::string>> categorized;
    for (auto& cmdName : registry.GetCommandList()) {
        std::string category = "Uncategorized";
        auto it = command_metadata.find(cmdName);
        if (it != command_metadata.end()) category = it->second.category;
        categorized[category].push_back(cmdName);
    }

    renderer.PrintLine("Available commands:");
    for (auto& [category, cmds] : categorized) {
        renderer.PrintLine("\n" + std::string(COLOR_CYAN) + "[" + category + "]" + COLOR_RESET);
        for (auto& cmd : cmds) {
            auto it = command_metadata.find(cmd);
            std::string desc = (it != command_metadata.end()) ? " - " + it->second.description : "";
            renderer.PrintLine(std::string(COLOR_YELLOW) + cmd + COLOR_RESET + desc);
        }
    }
}

// Auto-register HelpCommand
static bool registered = []() {
    CommandRegistry::Instance().Register("help", std::make_unique<HelpCommand>());
    return true;
}();
