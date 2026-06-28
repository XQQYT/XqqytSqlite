#pragma once

#include <memory>
#include <string>
#include <unordered_map>

#include "driver_interface/i_ssh_driver.h"
#include "driver_interface/i_sftp_driver.h"
#include "driver_interface/types.h"
#include "driver/ssh/ssh_session_guard.h"
#include "driver/ssh/known_hosts.h"

namespace remote_sqlite_qt {

// LibSSH2 实现的 SSH 驱动——封装 libssh2_session_*, libssh2_channel_* C API
class LibSsh2SshDriver : public ISshDriver {
public:
    LibSsh2SshDriver();
    ~LibSsh2SshDriver() override;

    LibSsh2SshDriver(const LibSsh2SshDriver&) = delete;
    LibSsh2SshDriver& operator=(const LibSsh2SshDriver&) = delete;

    // 设置 known_hosts 文件路径
    void setKnownHostsPath(const std::string& path);

    // ISshDriver 接口
    std::string connect(const SshConfig& config) override;
    void disconnect(const std::string& connectionId) override;
    bool isConnected(const std::string& connectionId) const override;
    ExecResult execCommand(const std::string& connectionId,
                           const std::string& command,
                           int timeoutSec = 30) override;
    std::unique_ptr<ISftpDriver> createSftpDriver(
        const std::string& connectionId) override;
    std::string getHostKey(const std::string& connectionId) const override;
    ConnectionInfo getConnectionInfo(const std::string& connectionId) const override;

private:
    struct SshSession {
        SshSessionGuard session;
        SshConfig config;
        std::string connectionId;
        std::string hostKeyFingerprint;
        ConnectionState state{ConnectionState::Disconnected};
    };

    // 建立 TCP socket 连接
    int createSocket(const std::string& host, int port, int timeoutSec);

    // 执行 SSH 握手和认证
    bool performHandshake(SshSession& sess);

    // 认证方式分发
    bool authenticate(SshSession& sess, const SshConfig& config);

    // 获取远端 host key 指纹（SHA256）
    std::string getFingerprint(LIBSSH2_SESSION* session);

    // 生成连接 ID
    std::string nextConnectionId();

    std::unordered_map<std::string, std::unique_ptr<SshSession>> sessions_;
    std::unique_ptr<KnownHostsVerifier> knownHostsVerifier_;
    int connectionCounter_{0};
    std::string knownHostsPath_;
};

}  // namespace remote_sqlite_qt
