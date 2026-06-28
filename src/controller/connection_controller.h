#pragma once

#include <memory>
#include <string>
#include <unordered_map>

#include "driver_interface/i_ssh_driver.h"
#include "driver_interface/types.h"

namespace remote_sqlite_qt {

class EventBusManager;

class ConnectionController {
public:
    ConnectionController(std::shared_ptr<ISshDriver> sshDriver,
                         EventBusManager& busMgr);
    ~ConnectionController() = default;

    // 禁止拷贝
    ConnectionController(const ConnectionController&) = delete;
    ConnectionController& operator=(const ConnectionController&) = delete;

    // 连接管理
    std::string connect(const SshConfig& config);
    void disconnect(const std::string& connectionId);
    bool isConnected(const std::string& connectionId) const;

    // 连接测试
    bool testConnection(const SshConfig& config);

    // 获取连接列表
    std::vector<ConnectionInfo> getConnections() const;

    // 获取单个连接信息
    ConnectionInfo getConnectionInfo(const std::string& connectionId) const;

    // 获取 SSH 驱动（供 RemoteSqliteDriver 使用）
    std::shared_ptr<ISshDriver> sshDriver() const { return sshDriver_; }

private:
    std::shared_ptr<ISshDriver> sshDriver_;
    EventBusManager& busMgr_;
    std::unordered_map<std::string, SshConfig> connections_;
};

}  // namespace remote_sqlite_qt
