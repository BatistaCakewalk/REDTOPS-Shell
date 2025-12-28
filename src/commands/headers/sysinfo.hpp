#pragma once

#include "../../core/header/CommandRegistry.hpp"
#include <vector>
#include <string>

class SysInfoCommand : public Command {
public:
    SysInfoCommand() = default;
    void Execute(const std::vector<std::string>& args) override;
    std::string Name() const override { return "sysinfo"; }
};
