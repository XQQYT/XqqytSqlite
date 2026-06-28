#include "config_manager.h"

#include <algorithm>
#include <fstream>

#include "json.hpp"
#include "spdlog/spdlog.h"

namespace remote_sqlite_qt {

ConfigManager& ConfigManager::instance() {
    static ConfigManager mgr;
    return mgr;
}

bool ConfigManager::loadFromFile(const std::string& configPath) {
    try {
        std::ifstream file(configPath);
        if (!file.is_open()) {
            spdlog::warn("Config file not found: {}", configPath);
            return false;
        }

        auto j = nlohmann::json::parse(file);

        // 加载连接配置
        savedConnections_.clear();
        if (j.contains("connections") && j["connections"].is_array()) {
            for (const auto& c : j["connections"]) {
                SshConfig cfg;
                cfg.host = c.value("host", "");
                cfg.port = c.value("port", 22);
                cfg.username = c.value("username", "");
                cfg.authMethod = static_cast<AuthMethod>(
                    c.value("authMethod", 0));
                cfg.password = c.value("password", "");
                cfg.privateKeyPath = c.value("privateKeyPath", "");
                cfg.passphrase = c.value("passphrase", "");
                cfg.jumpHost = c.value("jumpHost", "");
                cfg.connectTimeout = c.value("connectTimeout", 30);
                cfg.keepAliveInterval = c.value("keepAliveInterval", 60);

                savedConnections_.push_back(cfg);
            }
        }

        // 加载应用设置
        theme_ = j.value("theme", "dark");
        language_ = j.value("language", "zh");
        queryPageSize_ = j.value("queryPageSize", 100);

        spdlog::info("Config loaded: {} connections", savedConnections_.size());
        return true;
    } catch (const std::exception& e) {
        spdlog::error("Failed to load config: {}", e.what());
        return false;
    }
}

bool ConfigManager::saveToFile(const std::string& configPath) {
    try {
        nlohmann::json j;

        // 保存连接配置（注意：password 等敏感字段需加密）
        nlohmann::json connections = nlohmann::json::array();
        for (const auto& cfg : savedConnections_) {
            nlohmann::json c;
            c["host"] = cfg.host;
            c["port"] = cfg.port;
            c["username"] = cfg.username;
            c["authMethod"] = static_cast<int>(cfg.authMethod);
            c["password"] = cfg.password;  // TODO: AES-256 加密
            c["privateKeyPath"] = cfg.privateKeyPath;
            c["passphrase"] = cfg.passphrase;  // TODO: AES-256 加密
            c["jumpHost"] = cfg.jumpHost;
            c["connectTimeout"] = cfg.connectTimeout;
            c["keepAliveInterval"] = cfg.keepAliveInterval;
            connections.push_back(c);
        }
        j["connections"] = connections;

        j["theme"] = theme_;
        j["language"] = language_;
        j["queryPageSize"] = queryPageSize_;

        std::ofstream file(configPath);
        file << j.dump(2);
        spdlog::info("Config saved: {} connections", savedConnections_.size());
        return true;
    } catch (const std::exception& e) {
        spdlog::error("Failed to save config: {}", e.what());
        return false;
    }
}

std::vector<SshConfig> ConfigManager::getSavedConnections() const {
    return savedConnections_;
}

void ConfigManager::addConnection(const SshConfig& config) {
    savedConnections_.push_back(config);
}

void ConfigManager::removeConnection(const std::string& host,
                                      const std::string& username) {
    savedConnections_.erase(
        std::remove_if(savedConnections_.begin(), savedConnections_.end(),
                       [&](const SshConfig& c) {
                           return c.host == host && c.username == username;
                       }),
        savedConnections_.end());
}

void ConfigManager::updateConnection(const std::string& host,
                                      const std::string& username,
                                      const SshConfig& config) {
    for (auto& c : savedConnections_) {
        if (c.host == host && c.username == username) {
            c = config;
            return;
        }
    }
}

void ConfigManager::setTheme(const std::string& theme) {
    theme_ = theme;
}

void ConfigManager::setLanguage(const std::string& lang) {
    language_ = lang;
}

void ConfigManager::setQueryPageSize(int size) {
    queryPageSize_ = size;
}

}  // namespace remote_sqlite_qt
