#pragma once

#include <memory>
#include <string>

#include "types.h"

namespace remote_sqlite_qt {

class ISftpDriver;

class ISshDriver {
public:
    virtual ~ISshDriver() = default;

    // ===== 连接管理 =====
    // 建立 SSH 连接，返回唯一连接 ID
    virtual std::string connect(const SshConfig& config) = 0;

    // 断开连接
    virtual void disconnect(const std::string& connectionId) = 0;

    // 连接状态查询
    virtual bool isConnected(const std::string& connectionId) const = 0;

    // ===== 远程命令执行 =====
    // 执行远端命令，返回 exitcode + stdout + stderr
    virtual ExecResult execCommand(const std::string& connectionId,
                                   const std::string& command,
                                   int timeoutSec = 30) = 0;

    // ===== SFTP 子会话 =====
    // 从现有 SSH 连接创建 SFTP 驱动
    virtual std::unique_ptr<ISftpDriver> createSftpDriver(
        const std::string& connectionId) = 0;

    // ===== 连接信息 =====
    // 获取目标主机的 host key（用于 known_hosts 校验）
    virtual std::string getHostKey(const std::string& connectionId) const = 0;

    // 获取连接信息摘要
    virtual ConnectionInfo getConnectionInfo(const std::string& connectionId) const = 0;
};

}  // namespace remote_sqlite_qt
