#include "../headers/clear.hpp"
#include "../../core/header/TerminalRenderer.hpp"
#include "../../core/header/CommandRegistry.h"
#include <memory>
#include <iostream>

std::string ClearCommand::Name() const {
    return "clear";
}

void ClearCommand::Execute(const std::vector<std::string>&) {
    // Clear your custom renderer
    TerminalRenderer::Instance().Clear();

    // Also clear the actual terminal screen
#if defined(_WIN32)
    std::system("cls");
#else
    std::cout << "\033[2J\033[H" << std::flush;
#endif
}

// Auto-register ClearCommand
static bool registered = []() {
    CommandRegistry::Instance().Register("clear", std::make_unique<ClearCommand>());
    return true;
}();
