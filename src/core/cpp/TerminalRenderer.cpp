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
    // ANSI clear (works on most terminals)
    std::cout << "\x1b[2J\x1b[H";
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
    Clear();
    std::cout << boottxt << std::endl;
    // small pause to emulate boot
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
}