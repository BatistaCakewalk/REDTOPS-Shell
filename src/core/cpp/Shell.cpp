#include "../header/Shell.h"
#include "../../commands/headers/ping.hpp"
#include "../../commands/headers/sysinfo.hpp"
#include "../../commands/headers/netinfo.hpp"
#include "../../commands/headers/exit.hpp"
#include "../header/CommandRegistry.h"
#include "../header/CommandParser.h"
#include <iostream>
#include <fstream>
#include <thread>
#include <chrono>
#include <sstream>
#include <termios.h>
#include <unistd.h>
#include <algorithm>

// Simple getch() for Unix terminals
static char GetChar() {
    struct termios oldt, newt;
    char ch;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO); // disable canonical mode and echo
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    ch = getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    return ch;
}

Shell::Shell() : running_(false), history_index_(0) {}

Shell::~Shell() {}

// Already injected ClearCommand and other builtins
void Shell::RegisterBuiltins() {
    CommandRegistry::Instance().Register("ping", std::make_unique<PingCommand>());
    CommandRegistry::Instance().Register("sysinfo", std::make_unique<SysInfoCommand>());
    CommandRegistry::Instance().Register("netinfo", std::make_unique<NetInfoCommand>());
    CommandRegistry::Instance().Register("exit", std::make_unique<ExitCommand>(this));

    class ClearCommand : public Command {
    public:
        void Execute(const std::vector<std::string>&) override {
            TerminalRenderer::Instance().Clear();
        }
        std::string Name() const override { return "clear"; }
    };
    CommandRegistry::Instance().Register("clear", std::make_unique<ClearCommand>());
}

void Shell::Start() {
    renderer_.Init();

    // Boot screen
    std::ifstream f("../assets/screens/boot.txt", std::ios::binary);
    std::string boot;
    if (f) {
        boot.assign((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
    }
    renderer_.DrawBootScreen(boot);

    std::cout << std::flush;
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    renderer_.SetPrompt("RT> ");
    RegisterBuiltins();

    running_ = true;
    MainLoop();
}

void Shell::Stop() {
    running_ = false;
}

std::string Shell::ApplyAliases(const std::string& input) {
    auto tokens = CommandParser::Tokenize(input);
    if (tokens.empty()) return input;
    if (aliases_.count(tokens[0])) {
        tokens[0] = aliases_[tokens[0]];
        std::string replaced;
        for (auto& t : tokens) replaced += t + " ";
        return replaced;
    }
    return input;
}

// ------------------- New helper: Tab completion -------------------
std::string Shell::TabComplete(const std::string& prefix) {
    auto commands = CommandRegistry::Instance().GetCommandList();
    std::vector<std::string> matches;
    for (auto& cmd : commands) {
        if (cmd.find(prefix) == 0) matches.push_back(cmd);
    }
    if (matches.empty()) return prefix;
    if (matches.size() == 1) return matches[0]; // unique match
    // multiple matches: just return prefix for now
    TerminalRenderer::Instance().PrintLine("\nPossible matches:");
    for (auto& m : matches) TerminalRenderer::Instance().PrintLine("  " + m);
    return prefix;
}

// ------------------- Updated MainLoop -------------------
void Shell::MainLoop() {
    std::string input;
    while (running_) {
        input.clear();
        std::cout << renderer_.GetPrompt();
        char ch;

        while (true) {
            ch = GetChar();

            if (ch == '\n') { // Enter
                std::cout << "\n";
                break;
            } else if (ch == 127 || ch == '\b') { // Backspace
                if (!input.empty()) {
                    input.pop_back();
                    std::cout << "\b \b" << std::flush;
                }
            } else if (ch == '\t') { // Tab
                input = TabComplete(input);
                std::cout << "\r" << renderer_.GetPrompt() << input << std::flush;
            } else if (ch == 27) { // Escape sequences (arrows)
                char seq1 = GetChar();
                char seq2 = GetChar();
                if (seq1 == '[') {
                    if (seq2 == 'A') { // Up arrow
                        if (!history_.empty() && history_index_ > 0) {
                            history_index_--;
                            input = history_[history_index_];
                            std::cout << "\r" << renderer_.GetPrompt() << input
                                      << std::string(50, ' ') << "\r" << renderer_.GetPrompt() << input << std::flush;
                        }
                    } else if (seq2 == 'B') { // Down arrow
                        if (!history_.empty() && history_index_ + 1 < history_.size()) {
                            history_index_++;
                            input = history_[history_index_];
                        } else {
                            input.clear();
                            history_index_ = history_.size();
                        }
                        std::cout << "\r" << renderer_.GetPrompt() << input
                                  << std::string(50, ' ') << "\r" << renderer_.GetPrompt() << input << std::flush;
                    }
                }
            } else {
                input += ch;
                std::cout << ch << std::flush;
            }
        }

        // Apply aliases
        input = ApplyAliases(input);

        // Multi-command execution
        std::istringstream cmdstream(input);
        std::string singlecmd;
        while (std::getline(cmdstream, singlecmd, ';')) {
            auto tokens = CommandParser::Tokenize(singlecmd);
            if (tokens.empty()) continue;

            auto* cmd = CommandRegistry::Instance().Get(tokens[0]);
            if (!cmd) {
                renderer_.PrintLine("Unknown command: " + tokens[0]);
                continue;
            }
            tokens.erase(tokens.begin());
            cmd->Execute(tokens);
        }

        // Add to history
        if (!input.empty()) {
            history_.push_back(input);
            history_index_ = history_.size();
        }
    }
}
