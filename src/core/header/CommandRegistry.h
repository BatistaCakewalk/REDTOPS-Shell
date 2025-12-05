#pragma once
#include "Command.h"
#include <string>
#include <unordered_map>
#include <memory>


class CommandRegistry {
public:
    static CommandRegistry& Instance();


    void Register(const std::string& name, std::unique_ptr<Command> cmd);
    Command* Get(const std::string& name) const;
    std::vector<std::string> GetCommandList() const;


private:
    CommandRegistry() = default;
    std::unordered_map<std::string, std::unique_ptr<Command>> commands_;
};