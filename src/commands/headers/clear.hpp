#pragma once
#include "../../core/Command.h"

class ClearCommand : public Command {
public:
    std::string Name() const override;
    void Execute(const std::vector<std::string>& args) override;
};
