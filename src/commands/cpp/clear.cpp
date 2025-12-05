#include "../headers/clear.hpp"
#include "../../core/TerminalRenderer.hpp"
#include "../../core/CommandRegistry.h"
#include <memory>
#include <iostream>


std::string ClearCommand::Name() const {
    return "clear";
}

void ClearCommand::Execute(const std::vector<std::string>&) {
    TerminalRenderer::Instance().Clear();
}

// Auto-register ClearCommand
static bool registered = []() {
    CommandRegistry::Instance().Register("clear", std::make_unique<ClearCommand>());
    return true;
}();
