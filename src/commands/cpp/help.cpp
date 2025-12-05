#include "../headers/help.hpp"
#include "../../core/TerminalRenderer.hpp"
#include "../../core/CommandRegistry.h"
#include <memory>
#include <iostream>

std::string HelpCommand::Name() const {
    return "help";
}

void HelpCommand::Execute(const std::vector<std::string>&) {
    auto& registry = CommandRegistry::Instance();
    auto& renderer = TerminalRenderer::Instance();

    renderer.PrintLine("Available commands:");
    for (auto& cmdName : registry.GetCommandList()) {
        renderer.PrintLine(" - " + cmdName);
    }
}

// Auto-register HelpCommand
static bool registered = []() {
    CommandRegistry::Instance().Register("help", std::make_unique<HelpCommand>());
    return true;
}();
