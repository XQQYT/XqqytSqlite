#pragma once

#include <memory>
#include <string>
#include <vector>

#include "driver_interface/i_sftp_driver.h"
#include "driver_interface/types.h"

namespace remote_sqlite_qt {

class EventBusManager;

class SftpController {
public:
    SftpController(std::unique_ptr<ISftpDriver> sftpDriver,
                   EventBusManager& busMgr);
    ~SftpController() = default;

    SftpController(const SftpController&) = delete;
    SftpController& operator=(const SftpController&) = delete;

    // 目录浏览
    void listDirectory(const std::string& remotePath);

    // 文件下载
    void downloadFile(const std::string& remotePath, const std::string& localPath);

    // 文件上传
    void uploadFile(const std::string& localPath, const std::string& remotePath);

    // 文件操作
    void removeFile(const std::string& remotePath);
    void createDirectory(const std::string& remotePath);

    // SFTP 驱动访问
    ISftpDriver* driver() { return sftpDriver_.get(); }

private:
    std::unique_ptr<ISftpDriver> sftpDriver_;
    EventBusManager& busMgr_;
    int taskCounter_{0};

    std::string nextTaskId();
};

}  // namespace remote_sqlite_qt
