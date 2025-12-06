#include "../header/TerminalRenderer.h"
#include <iostream>
#include <chrono>
#include <thread>


TerminalRenderer::TerminalRenderer() = default;
TerminalRenderer::~TerminalRenderer() = default;


void TerminalRenderer::Init() {
    // Minimal: ensure std::cout is not buffered for interactive UI
    std::cout.setf(std::ios::unitbuf);
}


void TerminalRenderer::Clear() {
#ifdef _WIN32
    // Windows cmd/powershell: use system call to clear screen
    system("cls");
#else
    // ANSI clear for Unix-like terminals
    std::cout << "\x1b[2J\x1b[H" << std::flush;
#endif
}


void TerminalRenderer::Print(const std::string& s) {
    std::cout << s;
}


void TerminalRenderer::PrintLine(const std::string& s) {
    std::cout << s << std::endl;
}


void TerminalRenderer::SetPrompt(const std::string& p) {
    prompt_ = p;
}



void TerminalRenderer::DrawBootScreen(const std::string& boottxt) {
//     // Clear the screen safely
// #ifdef _WIN32
//     system("cls"); // Windows
// #else
//     std::cout << "\x1b[2J\x1b[H" << std::flush; // Unix-like
// #endif

    // Print each line separately to avoid terminal issues
    std::istringstream iss(boottxt);
    std::string line;
    while (std::getline(iss, line)) {
        std::cout << line << std::endl; // endl flushes automatically
        std::this_thread::sleep_for(std::chrono::milliseconds(10)); // optional line-by-line effect
    }

    // Final pause to emulate boot
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
}