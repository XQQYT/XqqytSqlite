#include "connection_controller.h"
#include "eventbus/event_bus_manager.h"
#include "eventbus/event_names.h"
#include "spdlog/spdlog.h"

namespace remote_sqlite_qt {

ConnectionController::ConnectionController(std::shared_ptr<ISshDriver> sshDriver,
                                           EventBusManager& busMgr)
    : sshDriver_(std::move(sshDriver)), busMgr_(busMgr) {}

std::string ConnectionController::connect(const SshConfig& config) {
    spdlog::info("Connecting to {}@{}:{}", config.username, config.host, config.port);
    auto connId = sshDriver_->connect(config);
    if (connId.empty()) {
        spdlog::error("Failed to connect to {}@{}", config.username, config.host);
        busMgr_.publish(std::string(events::SshError), connId,
                        std::string("Connection failed: ") + config.host, -1);
        return "";
    }
    connections_[connId] = config;
    busMgr_.publish(std::string(events::SshConnected), connId);
    busMgr_.publish(std::string(events::ConnectionListChanged), getConnections());
    spdlog::info("Connected successfully: {}", connId);
    return connId;
}

void ConnectionController::disconnect(const std::string& connectionId) {
    spdlog::info("Disconnecting: {}", connectionId);
    sshDriver_->disconnect(connectionId);
    connections_.erase(connectionId);
    busMgr_.publish(std::string(events::SshDisconnected), connectionId,
                    std::string("User requested"));
    busMgr_.publish(std::string(events::ConnectionListChanged), getConnections());
}

bool ConnectionController::isConnected(const std::string& connectionId) const {
    return sshDriver_->isConnected(connectionId);
}

bool ConnectionController::testConnection(const SshConfig& config) {
    spdlog::info("Testing connection to {}@{}:{}", config.username, config.host, config.port);
    auto connId = sshDriver_->connect(config);
    if (connId.empty()) {
        busMgr_.publish(std::string(events::ConnectionTestResult), config.host, false,
                        std::string("Connection failed"));
        return false;
    }
    auto result = sshDriver_->execCommand(connId, "echo OK");
    bool success = (result.exitCode == 0);
    sshDriver_->disconnect(connId);
    busMgr_.publish(std::string(events::ConnectionTestResult), config.host, success,
                    success ? std::string("Connection successful")
                            : std::string("Command execution failed"));
    return success;
}

std::vector<ConnectionInfo> ConnectionController::getConnections() const {
    std::vector<ConnectionInfo> list;
    for (const auto& [id, cfg] : connections_) {
        list.push_back(sshDriver_->getConnectionInfo(id));
    }
    return list;
}

ConnectionInfo ConnectionController::getConnectionInfo(
    const std::string& connectionId) const {
    return sshDriver_->getConnectionInfo(connectionId);
}

}  // namespace remote_sqlite_qt
