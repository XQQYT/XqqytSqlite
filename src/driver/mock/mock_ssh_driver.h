#pragma once

#include <string>
#include <unordered_map>

#include "driver_interface/i_ssh_driver.h"
#include "driver_interface/i_sftp_driver.h"

namespace remote_sqlite_qt {

// Mock SSH 驱动——用于上层 Controller 在无真实 SSH 环境时开发与测试
class MockSshDriver : public ISshDriver {
public:
    MockSshDriver() = default;
    ~MockSshDriver() override = default;

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

    // 测试辅助方法
    void setExecResult(const std::string& stdOut, const std::string& stdErr, int exitCode);
    void setConnectResult(bool success);
    void simulateError(const std::string& message);

private:
    struct MockSession {
        SshConfig config;
        ConnectionState state{ConnectionState::Disconnected};
        std::string hostKey{"SHA256:mock-host-key-for-testing"};
    };

    std::unordered_map<std::string, MockSession> sessions_;
    ExecResult mockExecResult_;
    bool connectSuccess_{true};
    int connectionCounter_{0};
};

}  // namespace remote_sqlite_qt
