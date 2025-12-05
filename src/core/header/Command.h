#pragma once
#include <string>
#include <vector>


class Command {
public:
    virtual ~Command() = default;
    virtual std::string Name() const = 0;
    virtual std::string Help() const { return "(no help)"; }
    virtual void Execute(const std::vector<std::string>& args) = 0;
};