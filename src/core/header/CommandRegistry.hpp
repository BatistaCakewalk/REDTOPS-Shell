#pragma once
#include "Exceptions.hpp" // Include the custom exception header
#include "Command.hpp"
#include <string>
#include <unordered_map>
#include <memory>


class CommandRegistry {
public:
    static CommandRegistry& Instance();


    void Register(const std::string& name, std::unique_ptr<Command> cmd);
    Command* Get(const std::string& name) const; // Keep declaration for now, implementation will throw
    std::vector<std::string> GetCommandList() const;


private:
    CommandRegistry() = default;
    std::unordered_map<std::string, std::unique_ptr<Command>> commands_;
};