#pragma once
#include "../../core/header/Command.h"
#include "../../core/header/Shell.h"

class ExitCommand : public Command {
public:
    ExitCommand(Shell* shell) : shell_(shell) {}
    void Execute(const std::vector<std::string>& args) override;
    std::string Name() const override { return "exit"; }

private:
    Shell* shell_;
};
