#include "mock_ssh_driver.h"

#include <sstream>

namespace remote_sqlite_qt {

std::string MockSshDriver::connect(const SshConfig& config) {
    if (!connectSuccess_) return "";

    auto connId = "mock-conn-" + std::to_string(++connectionCounter_);
    MockSession session;
    session.config = config;
    session.state = ConnectionState::Connected;
    sessions_[connId] = session;
    return connId;
}

void MockSshDriver::disconnect(const std::string& connectionId) {
    sessions_.erase(connectionId);
}

bool MockSshDriver::isConnected(const std::string& connectionId) const {
    auto it = sessions_.find(connectionId);
    return it != sessions_.end() &&
           it->second.state == ConnectionState::Connected;
}

ExecResult MockSshDriver::execCommand(const std::string& connectionId,
                                      const std::string& /*command*/,
                                      int /*timeoutSec*/) {
    if (!isConnected(connectionId)) {
        return ExecResult{-1, "", "Not connected"};
    }
    return mockExecResult_;
}

std::unique_ptr<ISftpDriver> MockSshDriver::createSftpDriver(
    const std::string& /*connectionId*/) {
    // MockSftpDriver 定义在 mock_sftp_driver.h 中
    // 此处返回 nullptr 表示未实现，真实创建由 libssh2 driver 处理
    return nullptr;
}

std::string MockSshDriver::getHostKey(const std::string& connectionId) const {
    auto it = sessions_.find(connectionId);
    return (it != sessions_.end()) ? it->second.hostKey : "";
}

ConnectionInfo MockSshDriver::getConnectionInfo(
    const std::string& connectionId) const {
    ConnectionInfo info;
    info.connectionId = connectionId;
    auto it = sessions_.find(connectionId);
    if (it != sessions_.end()) {
        info.host = it->second.config.host;
        info.username = it->second.config.username;
        info.state = it->second.state;
    }
    return info;
}

void MockSshDriver::setExecResult(const std::string& stdOut,
                                   const std::string& stdErr,
                                   int exitCode) {
    mockExecResult_.stdOut = stdOut;
    mockExecResult_.stdErr = stdErr;
    mockExecResult_.exitCode = exitCode;
}

void MockSshDriver::setConnectResult(bool success) {
    connectSuccess_ = success;
}

void MockSshDriver::simulateError(const std::string& /*message*/) {
    connectSuccess_ = false;
}

}  // namespace remote_sqlite_qt
