#include "../header/Shell.hpp"
#include "../../commands/headers/ping.hpp"
#include "../../commands/headers/sysinfo.hpp"
#include "../../commands/headers/netinfo.hpp"
#include "../../commands/headers/exit.hpp"
#include "../../commands/headers/fs_commands.hpp"
#include "../../commands/headers/trace.hpp"
#include "../../commands/headers/portscan.hpp"
#include "../../commands/headers/netscan.hpp"
#include "../header/CommandRegistry.hpp"
#include "../header/CommandParser.hpp"
#include <iostream>
#include <fstream>
#include <thread>
#include <chrono>
#include <sstream>
#include <termios.h>
#include <unistd.h>
#include <algorithm>
#include <filesystem>
#include <signal.h>
#include <cstring>
#include <sys/ioctl.h>

// Editor note: fully fixed the ghost line/prompt doubling bug

static struct termios orig_termios;
static bool raw_enabled = false;

static void DisableRawMode() {
    if (raw_enabled) {
        tcsetattr(STDIN_FILENO, TCSANOW, &orig_termios);
        raw_enabled = false;
    }
}

static bool EnableRawMode() {
    if (raw_enabled) return true;
    if (tcgetattr(STDIN_FILENO, &orig_termios) == -1) return false;
    struct termios raw = orig_termios;
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
    raw.c_iflag &= ~(IXON | ICRNL | BRKINT | INPCK | ISTRIP);
    raw.c_oflag &= ~(OPOST);
    raw.c_cflag |= (CS8);
    raw.c_cc[VMIN] = 1;
    raw.c_cc[VTIME] = 0;
    if (tcsetattr(STDIN_FILENO, TCSANOW, &raw) == -1) return false;
    raw_enabled = true;
    return true;
}

static char ReadByte() {
    char c = 0;
    ssize_t n = read(STDIN_FILENO, &c, 1);
    if (n <= 0) return 0;
    return c;
}

struct TempCookedMode {
    bool had_raw;
    TempCookedMode() { had_raw = raw_enabled; if (had_raw) DisableRawMode(); }
    ~TempCookedMode() { if (had_raw) EnableRawMode(); }
};

Shell::Shell() : running_(false), history_index_(-1) {}
Shell::~Shell() { DisableRawMode(); }

Shell& Shell::Instance() {
    static Shell instance;
    return instance;
}

std::string Shell::GetPromptString() const {
    try { return "REDTOPS | " + std::filesystem::current_path().string() + "> "; }
    catch (...) { return "REDTOPS | ? > "; }
}

void Shell::RegisterBuiltins() {
    CommandRegistry::Instance().Register("ping", std::make_unique<PingCommand>());
    CommandRegistry::Instance().Register("sysinfo", std::make_unique<SysInfoCommand>());
    CommandRegistry::Instance().Register("netinfo", std::make_unique<NetInfoCommand>());
    CommandRegistry::Instance().Register("trace", std::make_unique<TraceCommand>());
    CommandRegistry::Instance().Register("netscan", std::make_unique<NetScanCommand>());
    CommandRegistry::Instance().Register("portscan", std::make_unique<PortScanCommand>());
    CommandRegistry::Instance().Register("exit", std::make_unique<ExitCommand>(this));
    CommandRegistry::Instance().Register("pwd", std::make_unique<PwdCommand>());
    CommandRegistry::Instance().Register("cd", std::make_unique<CdCommand>());
    CommandRegistry::Instance().Register("ls", std::make_unique<LsCommand>());
    CommandRegistry::Instance().Register("cat", std::make_unique<CatCommand>());
    CommandRegistry::Instance().Register("mkdir", std::make_unique<MkdirCommand>());
    CommandRegistry::Instance().Register("rm", std::make_unique<RmCommand>());
    CommandRegistry::Instance().Register("cp", std::make_unique<CpCommand>());
    CommandRegistry::Instance().Register("mv", std::make_unique<MvCommand>());

    class ClearCommand : public Command {
    public:
        void Execute(const std::vector<std::string>&) override { TerminalRenderer::Instance().Clear(); }
        std::string Name() const override { return "clear"; }
    };
    CommandRegistry::Instance().Register("cls", std::make_unique<ClearCommand>());
    CommandRegistry::Instance().Register("clear", std::make_unique<ClearCommand>());
}

void Shell::Start() {
    auto& renderer = TerminalRenderer::Instance();
    renderer.Init();

    // 1. Load and display the ASCII art immediately
    std::ifstream f("../assets/screens/boot.txt", std::ios::binary);
    if (f) {
        std::string boot((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
        renderer.PrintLine(boot, Color::CYAN);
    }

    // 2. Simulated "System Check" sequence
    renderer.Typewrite(" > Initializing REDTOPS Kernel v0.5.1...", 10, Color::GREEN);
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    renderer.Typewrite(" > Checking System Architecture... [OK]", 10, Color::GREEN);
    
#ifdef _GNU_SOURCE
    renderer.PrintLine(" > OS: Linux Detected", Color::DIM);
#endif

    // 3. Register Commands with visual feedback
    renderer.Typewrite(" > Loading Network Modules...", 10, Color::GREEN);
    RegisterBuiltins();
    renderer.PrintLine("   Builtins loaded.", Color::DIM);

    // 4. Final transition
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    renderer.Typewrite(" > Terminal Access Granted.", 30, Color::CYAN);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    // renderer.Clear(); // This would remove the boot sequence which nah i want it :3
    
    EnableRawMode();
    running_ = true;
    MainLoop();
    DisableRawMode();
}

void Shell::Stop() { running_ = false; }

std::string Shell::ApplyAliases(const std::string& input) {
    auto tokens = CommandParser::Tokenize(input);
    if (tokens.empty()) return input;
    if (aliases_.count(tokens[0])) {
        tokens[0] = aliases_[tokens[0]];
        std::string out;
        for (auto& t : tokens) out += t + " ";
        return out;
    }
    return input;
}

std::string Shell::TabComplete(const std::string& prefix) {
    auto commands = CommandRegistry::Instance().GetCommandList();
    std::vector<std::string> matches;
    for (auto& cmd : commands)
        if (cmd.rfind(prefix, 0) == 0)
            matches.push_back(cmd);
    if (matches.empty()) return prefix;
    if (matches.size() == 1) return matches[0];
    TerminalRenderer::Instance().PrintLine("\nPossible matches:");
    for (auto& m : matches) TerminalRenderer::Instance().PrintLine("  " + m);
    return prefix;
}

void Shell::MainLoop() {
    std::string input;

    auto redraw = [&](const std::string& s) {
        std::cout << "\r" << "\x1b[2K" << GetPromptString() << s << std::flush;
    };

    while (running_) {
        input.clear();
        redraw(input);

        while (true) {
            char ch = ReadByte();
            if (ch == 0) continue;

            if (ch == '\r' || ch == '\n') {
                std::cout << "\n";
                break;
            } 
            else if (ch == 127 || ch == '\b') {
                if (!input.empty()) { input.pop_back(); redraw(input); }
            } 
            else if (ch == '\t') {
                input = TabComplete(input);
                redraw(input);
            } 
            else if (ch == 3) {
                std::cout << "^C\n";
                input.clear();
                redraw(input);
            } 
            else if (ch == 4) {
                std::cout << "\n";
                Stop();
                break;
            } 
            else if (ch == 27) {
                char s1 = ReadByte(); if (s1 == 0) continue;
                char s2 = ReadByte(); if (s2 == 0) continue;
                if (s1 == '[') {
                    if (s2 == 'A') {
                        if (!history_.empty()) {
                            if (history_index_ == -1) history_index_ = history_.size();
                            if (history_index_ > 0) history_index_--;
                            input = history_[history_index_];
                            redraw(input);
                        }
                    } else if (s2 == 'B') {
                        if (!history_.empty()) {
                            if (history_index_ == -1) history_index_ = history_.size();
                            if (history_index_ + 1 < (int)history_.size()) { history_index_++; input = history_[history_index_]; }
                            else { history_index_ = history_.size(); input.clear(); }
                            redraw(input);
                        }
                    }
                }
            } 
            else if (isprint(static_cast<unsigned char>(ch))) {
                input.push_back(ch);
                std::cout << ch << std::flush;
            }
        }

        if (!running_) break;

        input = ApplyAliases(input);

        // <--- FIXEDBUG: reset cursor to column 0 before command output --->
        std::cout << "\r\x1b[0K";  // moves to start of line & clears it

        { TempCookedMode guard;
            std::istringstream ss(input); std::string part;
            while (std::getline(ss, part, ';')) {
                auto tokens = CommandParser::Tokenize(part);
                if (tokens.empty()) continue;
                auto* cmd = CommandRegistry::Instance().Get(tokens[0]);
                if (!cmd) { renderer_.PrintLine("Unknown command: " + tokens[0]); continue; }
                tokens.erase(tokens.begin());
                cmd->Execute(tokens);
            }
            std::cout << std::flush;
        }

        if (!input.empty()) { history_.push_back(input); history_index_ = history_.size(); }
        else { history_index_ = history_.size(); }
    }
}

