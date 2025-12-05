#include "Shell.h"
#include "../commands/headers/ping.hpp"
#include "../commands/headers/sysinfo.hpp"
#include "../commands/headers/netinfo.hpp"
#include "CommandRegistry.h"
#include "CommandParser.h"
#include <iostream>
#include <fstream>

Shell::Shell() : running_(false) {
}

Shell::~Shell() {
}

void Shell::RegisterBuiltins() {
    // Already registered via static lambdas in commands

    // PING COMMAND
    CommandRegistry::Instance().Register("ping", std::make_unique<PingCommand>());

    // SYSINFO COMMAND
    CommandRegistry::Instance().Register("sysinfo", std::make_unique<SysInfoCommand>());

    // NETINFO COMMAND
    CommandRegistry::Instance().Register("netinfo", std::make_unique<NetInfoCommand>());

}

void Shell::Start() {
    renderer_.Init();

    // Draw boot screen
    std::ifstream f("assets/screens/boot.txt");
    std::string boot;
    if (f) {
        std::string line;
        while (std::getline(f, line)) { boot += line + "\n"; }
    }
    renderer_.DrawBootScreen(boot);

    // Set the prompt
    renderer_.SetPrompt("RT> ");

    // Register commands before starting loop
    RegisterBuiltins();

    running_ = true;
    MainLoop();
}


void Shell::Stop() {
    running_ = false;
}

void Shell::MainLoop() {
    std::string input;
    while (running_) {
        // Print the prompt
        std::cout << renderer_.GetPrompt();

        // Get input
        if (!std::getline(std::cin, input)) {
            Stop();
            break;
        }

        // Tokenize
        auto tokens = CommandParser::Tokenize(input);
        if (tokens.empty()) continue;

        // Lookup command
        auto* cmd = CommandRegistry::Instance().Get(tokens[0]);
        if (!cmd) {
            renderer_.PrintLine("Unknown command: " + tokens[0]);
            continue;
        }

        // Remove command name from args
        tokens.erase(tokens.begin());

        // Execute
        cmd->Execute(tokens);
    }
}
