#include "mock_sftp_driver.h"

#include <algorithm>
#include <stdexcept>
#include <thread>

namespace remote_sqlite_qt {

std::vector<FileEntry> MockSftpDriver::listDirectory(const std::string& /*remotePath*/) {
    return fileEntries_;
}

void MockSftpDriver::mkdir(const std::string& /*remotePath*/) {
    // no-op in mock
}

void MockSftpDriver::remove(const std::string& remotePath) {
    fileEntries_.erase(
        std::remove_if(fileEntries_.begin(), fileEntries_.end(),
                       [&](const FileEntry& e) { return e.name == remotePath; }),
        fileEntries_.end());
}

void MockSftpDriver::download(const std::string& /*remotePath*/,
                               const std::string& /*localPath*/,
                               ProgressCallback progressCallback) {
    if (transferDelay_ > 0) {
        std::this_thread::sleep_for(std::chrono::milliseconds(transferDelay_));
    }
    if (transferShouldFail_) {
        throw std::runtime_error("Mock SFTP download failed");
    }
    if (progressCallback) {
        progressCallback(1024, 1024);  // 模拟完成
    }
}

void MockSftpDriver::upload(const std::string& /*localPath*/,
                             const std::string& /*remotePath*/,
                             ProgressCallback progressCallback) {
    if (transferDelay_ > 0) {
        std::this_thread::sleep_for(std::chrono::milliseconds(transferDelay_));
    }
    if (transferShouldFail_) {
        throw std::runtime_error("Mock SFTP upload failed");
    }
    if (progressCallback) {
        progressCallback(1024, 1024);
    }
}

bool MockSftpDriver::fileExists(const std::string& remotePath) {
    return std::any_of(fileEntries_.begin(), fileEntries_.end(),
                       [&](const FileEntry& e) { return e.name == remotePath; });
}

int64_t MockSftpDriver::getFileSize(const std::string& remotePath) {
    auto it = std::find_if(fileEntries_.begin(), fileEntries_.end(),
                           [&](const FileEntry& e) { return e.name == remotePath; });
    return (it != fileEntries_.end()) ? it->size : -1;
}

std::string MockSftpDriver::getFileMtime(const std::string& remotePath) {
    auto it = std::find_if(fileEntries_.begin(), fileEntries_.end(),
                           [&](const FileEntry& e) { return e.name == remotePath; });
    return (it != fileEntries_.end()) ? it->mtime : "";
}

void MockSftpDriver::setFileEntries(const std::vector<FileEntry>& entries) {
    fileEntries_ = entries;
}

void MockSftpDriver::setTransferDelay(int delayMs) {
    transferDelay_ = delayMs;
}

void MockSftpDriver::setTransferShouldFail(bool fail) {
    transferShouldFail_ = fail;
}

}  // namespace remote_sqlite_qt
