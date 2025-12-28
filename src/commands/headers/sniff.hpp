#pragma once

#include "../../core/header/Command.hpp"
#include <string>
#include <vector>

class SniffCommand : public Command {
public:
    SniffCommand();
    void Execute(const std::vector<std::string>& args) override;
    std::string Name() const override { return "sniff"; }
    ~SniffCommand() override = default;
};