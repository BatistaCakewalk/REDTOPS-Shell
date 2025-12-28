// netinfo.hpp
#pragma once
#include "../../core/header/Command.hpp"
#include <vector>
#include <string>

class NetInfoCommand : public Command {
public:
    void Execute(const std::vector<std::string>& args) override;
    std::string Name() const override { return "netinfo"; } // <- implement the pure virtual
};
