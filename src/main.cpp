#include "core/header/Shell.hpp"
#include <csignal>
#include <iostream>

void SignalHandler(int signum) {
    Shell::Instance().Stop();
}

int main(int argc, char** argv) {
    // Setup signal listeners
    std::signal(SIGINT, SignalHandler);  // Catch Ctrl+C
    std::signal(SIGTERM, SignalHandler); // Catch kill commands

    Shell::Instance().Start();

    return 0;
}
