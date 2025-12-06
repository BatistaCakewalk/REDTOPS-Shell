#include "../header/Shell.h"
#include "../../commands/headers/ping.hpp"
#include "../../commands/headers/sysinfo.hpp"
#include "../../commands/headers/netinfo.hpp"
#include "../header/CommandRegistry.h"
#include "../header/CommandParser.h"
#include <iostream>
#include <fstream>
#include <thread>
#include <chrono>


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

    // Read boot screen file
    std::ifstream f("../assets/screens/boot.txt", std::ios::binary);
    std::string boot;
    if (!f) {
        std::cerr << "ERROR: Could not open boot.txt" << std::endl;
    } else {
        boot.assign((std::istreambuf_iterator<char>(f)),
                     std::istreambuf_iterator<char>());
    }

    // Draw the boot screen
    renderer_.DrawBootScreen(boot);

    // Force flush to ensure output appears before MainLoop
    std::cout << std::flush;
    std::this_thread::sleep_for(std::chrono::milliseconds(100)); // small delay for terminal

    // Set prompt and register commands
    renderer_.SetPrompt("RT> ");
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
