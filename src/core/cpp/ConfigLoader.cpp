#include "../header/ConfigLoader.hpp"
#include <fstream>
#include <iostream>

ConfigLoader& ConfigLoader::Instance() {
    static ConfigLoader instance;
    return instance;
}

void ConfigLoader::LoadConfig(const std::string& config_path) {
    std::ifstream file(config_path);
    if (!file.is_open()) {
        throw RedTops::ConfigError("Failed to open configuration file: " + config_path);
    }

    try {
        file >> config_data_;
        is_loaded_ = true;
    } catch (const nlohmann::json::parse_error& e) {
        throw RedTops::ConfigError("Failed to parse configuration file " + config_path + ": " + e.what());
    } catch (const std::exception& e) {
        throw RedTops::ConfigError("An unexpected error occurred while loading config: " + std::string(e.what()));
    }
}

std::string ConfigLoader::GetShellName() const {
    if (!is_loaded_) throw RedTops::ConfigError("Configuration not loaded.");
    try {
        return config_data_["shell"]["name"].get<std::string>();
    } catch (const nlohmann::json::exception& e) {
        throw RedTops::ConfigError("Missing 'shell.name' in config: " + std::string(e.what()));
    }
}

std::string ConfigLoader::GetShellVersion() const {
    if (!is_loaded_) throw RedTops::ConfigError("Configuration not loaded.");
    try {
        return config_data_["shell"]["version"].get<std::string>();
    } catch (const nlohmann::json::exception& e) {
        throw RedTops::ConfigError("Missing 'shell.version' in config: " + std::string(e.what()));
    }
}

std::string ConfigLoader::GetShellPrompt() const {
    if (!is_loaded_) throw RedTops::ConfigError("Configuration not loaded.");
    try {
        return config_data_["shell"]["prompt"].get<std::string>();
    } catch (const nlohmann::json::exception& e) {
        throw RedTops::ConfigError("Missing 'shell.prompt' in config: " + std::string(e.what()));
    }
}

std::string ConfigLoader::GetThemePath() const {
    if (!is_loaded_) throw RedTops::ConfigError("Configuration not loaded.");
    try {
        return config_data_["theme"].get<std::string>();
    } catch (const nlohmann::json::exception& e) {
        throw RedTops::ConfigError("Missing 'theme' in config: " + std::string(e.what()));
    }
}
