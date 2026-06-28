#include "sftp_controller.h"

#include "eventbus/event_bus_manager.h"
#include "eventbus/event_names.h"
#include "spdlog/spdlog.h"

namespace remote_sqlite_qt {

SftpController::SftpController(std::unique_ptr<ISftpDriver> sftpDriver,
                               EventBusManager& busMgr)
    : sftpDriver_(std::move(sftpDriver)), busMgr_(busMgr) {}

void SftpController::listDirectory(const std::string& remotePath) {
    spdlog::debug("Listing directory: {}", remotePath);
    try {
        auto entries = sftpDriver_->listDirectory(remotePath);
        busMgr_.publish(events::SftpFileList, remotePath, entries);
    } catch (const std::exception& e) {
        spdlog::error("SFTP list error: {}", e.what());
        busMgr_.publish(events::SftpError, std::string("list"),
                        std::string(e.what()));
    }
}

void SftpController::downloadFile(const std::string& remotePath,
                                   const std::string& localPath) {
    auto taskId = nextTaskId();
    spdlog::info("[{}] Download: {} -> {}", taskId, remotePath, localPath);

    try {
        sftpDriver_->download(remotePath, localPath,
            [this, taskId](int64_t transferred, int64_t total) {
                busMgr_.publish(events::SftpProgress, taskId, transferred, total);
            });
        busMgr_.publish(events::SftpComplete, taskId, localPath);
    } catch (const std::exception& e) {
        spdlog::error("[{}] Download failed: {}", taskId, e.what());
        busMgr_.publish(events::SftpError, taskId, std::string(e.what()));
    }
}

void SftpController::uploadFile(const std::string& localPath,
                                 const std::string& remotePath) {
    auto taskId = nextTaskId();
    spdlog::info("[{}] Upload: {} -> {}", taskId, localPath, remotePath);

    try {
        sftpDriver_->upload(localPath, remotePath,
            [this, taskId](int64_t transferred, int64_t total) {
                busMgr_.publish(events::SftpProgress, taskId, transferred, total);
            });
        busMgr_.publish(events::SftpComplete, taskId, remotePath);
    } catch (const std::exception& e) {
        spdlog::error("[{}] Upload failed: {}", taskId, e.what());
        busMgr_.publish(events::SftpError, taskId, std::string(e.what()));
    }
}

void SftpController::removeFile(const std::string& remotePath) {
    sftpDriver_->remove(remotePath);
}

void SftpController::createDirectory(const std::string& remotePath) {
    sftpDriver_->mkdir(remotePath);
}

std::string SftpController::nextTaskId() {
    return "sftp-" + std::to_string(++taskCounter_);
}

}  // namespace remote_sqlite_qt
