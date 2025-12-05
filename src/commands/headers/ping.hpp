#pragma once
#include "../../core/CommandRegistry.h"
#include <vector>
#include <string>

class PingCommand : public Command {
public:
    PingCommand();  // just declare it
    void Execute(const std::vector<std::string>& args) override;
    std::string Name() const override { return "ping"; }
};