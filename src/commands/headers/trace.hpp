#pragma once
#include "../../core/header/Command.hpp"
#include <vector>
#include <string>

class TraceCommand : public Command {
public:
    void Execute(const std::vector<std::string>& args) override;
    std::string Name() const override { return "trace"; }
};

