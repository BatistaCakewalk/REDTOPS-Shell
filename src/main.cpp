#include "core/header/Shell.h"
#include <csignal>
#include <iostream>

// Global pointer so the signal handler can trigger a clean stop
Shell* g_shell_ptr = nullptr;

void SignalHandler(int signum) {
    if (g_shell_ptr) {
        // This will break the MainLoop and trigger the Shell destructor
        // which calls DisableRawMode() automatically.
        g_shell_ptr->Stop();
    }
}

int main(int argc, char** argv) {
    // Setup signal listeners
    std::signal(SIGINT, SignalHandler);  // Catch Ctrl+C
    std::signal(SIGTERM, SignalHandler); // Catch kill commands

    Shell shell;
    g_shell_ptr = &shell; 

    shell.Start();

    return 0;
}
