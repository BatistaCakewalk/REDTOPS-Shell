#include "core/header/Shell.hpp"
#include <csignal>
#include <iostream>
#include <pcap.h> // Required for pcap_breakloop

void SignalHandler(int signum) {
    // If a sniff command is active, break its pcap_loop
    pcap_t* handle = Shell::Instance().GetCurrentPcapHandle();
    if (handle != nullptr) {
        pcap_breakloop(handle);
    }
    Shell::Instance().Stop();
}

int main(int argc, char** argv) {
    // Setup signal listeners
    std::signal(SIGINT, SignalHandler);  // Catch Ctrl+C
    std::signal(SIGTERM, SignalHandler); // Catch kill commands

    Shell::Instance().Start(argv[0]);

    return 0;
}
