#include "../headers/exit.hpp"
#include "../../core/header/TerminalRenderer.hpp"
#include <vector>
#include <string>

void ExitCommand::Execute(const std::vector<std::string>& args) {
    TerminalRenderer::Instance().PrintLine("Exiting REDTOPS...");
    if (shell_) shell_->Stop();
}
