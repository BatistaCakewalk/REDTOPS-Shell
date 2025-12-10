#pragma once
#include "../../core/header/Command.h"
#include <string>

class PortScanCommand : public Command {
public:
    std::string Name() const override { return "portscan"; }
    void Execute(const std::vector<std::string>& args) override;
};
