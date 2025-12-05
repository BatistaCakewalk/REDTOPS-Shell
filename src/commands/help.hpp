#pragma once
#include "../core/Command.h"

class HelpCommand : public Command {
public:
    std::string Name() const override;
    void Execute(const std::vector<std::string>& args) override;
};
