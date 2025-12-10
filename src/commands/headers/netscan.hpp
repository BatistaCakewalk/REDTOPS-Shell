#pragma once
#include "../../core/header/Command.h"
#include <vector>
#include <string>

class NetScanCommand : public Command {
public:
    void Execute(const std::vector<std::string>& args) override;
    std::string Name() const override { return "netscan"; }
};

