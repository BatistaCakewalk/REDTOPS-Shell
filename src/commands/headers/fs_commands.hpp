#pragma once

#include "../../core/header/Command.hpp"
#include <string>
#include <vector>

class PwdCommand : public Command {
public:
    void Execute(const std::vector<std::string>& args) override;
    std::string Name() const override { return "pwd"; }
    ~PwdCommand() override = default;
};

class CdCommand : public Command {
public:
    void Execute(const std::vector<std::string>& args) override;
    std::string Name() const override { return "cd"; }
    ~CdCommand() override = default;
};

class LsCommand : public Command {
public:
    void Execute(const std::vector<std::string>& args) override;
    std::string Name() const override { return "ls"; }
    ~LsCommand() override = default;
};

class CatCommand : public Command {
public:
    void Execute(const std::vector<std::string>& args) override;
    std::string Name() const override { return "cat"; }
    ~CatCommand() override = default;
};

class MkdirCommand : public Command {
public:
    void Execute(const std::vector<std::string>& args) override;
    std::string Name() const override { return "mkdir"; }
    ~MkdirCommand() override = default;
};

class RmCommand : public Command {
public:
    void Execute(const std::vector<std::string>& args) override;
    std::string Name() const override { return "rm"; }
    ~RmCommand() override = default;
};

class CpCommand : public Command {
public:
    void Execute(const std::vector<std::string>& args) override;
    std::string Name() const override { return "cp"; }
    ~CpCommand() override = default;
};

class MvCommand : public Command {
public:
    void Execute(const std::vector<std::string>& args) override;
    std::string Name() const override { return "mv"; }
    ~MvCommand() override = default;
};

