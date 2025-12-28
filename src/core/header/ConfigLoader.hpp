#pragma once

#include <string>
#include <nlohmann/json.hpp> // Include nlohmann/json header
#include "Exceptions.hpp" // For custom exception handling

namespace RedTops {
    // Custom exception for configuration errors
    class ConfigError : public RedTopsException {
    public:
        explicit ConfigError(const std::string& message)
            : RedTopsException(message) {}
    };
}

class ConfigLoader {
public:
    static ConfigLoader& Instance();

    // Delete copy constructor and assignment operator
    ConfigLoader(const ConfigLoader&) = delete;
    ConfigLoader& operator=(const ConfigLoader&) = delete;

    void LoadConfig(const std::string& config_path);

    // Accessors for config values
    std::string GetShellName() const;
    std::string GetShellVersion() const;
    std::string GetShellPrompt() const;
    std::string GetThemePath() const;

private:
    ConfigLoader() = default;
    nlohmann::json config_data_;
    bool is_loaded_ = false;
};