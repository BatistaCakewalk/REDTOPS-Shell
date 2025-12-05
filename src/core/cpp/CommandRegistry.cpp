#include "../header/CommandRegistry.h"
#include <algorithm>


CommandRegistry& CommandRegistry::Instance() {
    static CommandRegistry inst;
    return inst;
}


void CommandRegistry::Register(const std::string& name, std::unique_ptr<Command> cmd) {
    commands_[name] = std::move(cmd);
}


Command* CommandRegistry::Get(const std::string& name) const {
    auto it = commands_.find(name);
    if (it == commands_.end()) return nullptr;
    return it->second.get();
}


std::vector<std::string> CommandRegistry::GetCommandList() const {
    std::vector<std::string> out;
    out.reserve(commands_.size());
    for (auto& kv : commands_) out.push_back(kv.first);
    std::sort(out.begin(), out.end());
    return out;
}