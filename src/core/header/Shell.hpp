#pragma once

#include "TerminalRenderer.hpp"
#include <vector>
#include <unordered_map>
#include <string>
#include <filesystem> // For path manipulation
#include <fstream>    // For file I/O
#include <pcap.h>     // For pcap_t type

class Shell {
public:
    static Shell& Instance(); // Static method to access the single instance

    // Delete copy constructor and assignment operator
    Shell(const Shell&) = delete;
    Shell& operator=(const Shell&) = delete;

    ~Shell(); // Public destructor

    void Start(const char* argv0);
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
    const std::string history_file_path_ = std::string(std::getenv("HOME")) + "/.redtops_history"; // History file path
    const size_t max_history_size_ = 1000;             // Max history entries
    // ========================

    void RegisterBuiltins();
    void MainLoop();
    void LoadHistory();
    void SaveHistory();

    // QOL helpers
    std::string ApplyAliases(const std::string& input); // replaces input with alias if exists
    std::string TabComplete(const std::string& prefix); // completes command names starting with prefix

    pcap_t* current_pcap_handle_ = nullptr; // For managing active sniffing

public: // Make these public so SniffCommand can access them
    void SetCurrentPcapHandle(pcap_t* handle) { current_pcap_handle_ = handle; }
    void ClearCurrentPcapHandle() { current_pcap_handle_ = nullptr; }
    pcap_t* GetCurrentPcapHandle() const { return current_pcap_handle_; }
};

