#pragma once

#include <string>
#include <vector>

#include "driver_interface/types.h"

namespace remote_sqlite_qt {

// 配置持久化管理器——JSON 文件 + AES-256 加密敏感字段
class ConfigManager {
public:
    static ConfigManager& instance();

    ConfigManager(const ConfigManager&) = delete;
    ConfigManager& operator=(const ConfigManager&) = delete;

    // 加载/保存配置
    bool loadFromFile(const std::string& configPath);
    bool saveToFile(const std::string& configPath);

    // SSH 连接配置列表
    std::vector<SshConfig> getSavedConnections() const;
    void addConnection(const SshConfig& config);
    void removeConnection(const std::string& host, const std::string& username);
    void updateConnection(const std::string& host, const std::string& username,
                          const SshConfig& config);

    // 应用设置
    std::string theme() const { return theme_; }
    void setTheme(const std::string& theme);

    std::string language() const { return language_; }
    void setLanguage(const std::string& lang);

    int queryPageSize() const { return queryPageSize_; }
    void setQueryPageSize(int size);

private:
    ConfigManager() = default;
    ~ConfigManager() = default;

    std::vector<SshConfig> savedConnections_;
    std::string theme_{"dark"};
    std::string language_{"zh"};
    int queryPageSize_{100};
};

}  // namespace remote_sqlite_qt
