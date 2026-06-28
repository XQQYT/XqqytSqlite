#pragma once

#include <string>
#include <vector>

#include "driver_interface/i_sftp_driver.h"
#include "driver_interface/types.h"

namespace remote_sqlite_qt {

class MockSftpDriver : public ISftpDriver {
public:
    MockSftpDriver() = default;
    ~MockSftpDriver() override = default;

    std::vector<FileEntry> listDirectory(const std::string& remotePath) override;
    void mkdir(const std::string& remotePath) override;
    void remove(const std::string& remotePath) override;

    void download(const std::string& remotePath,
                  const std::string& localPath,
                  ProgressCallback progressCallback) override;

    void upload(const std::string& localPath,
                const std::string& remotePath,
                ProgressCallback progressCallback) override;

    bool fileExists(const std::string& remotePath) override;
    int64_t getFileSize(const std::string& remotePath) override;
    std::string getFileMtime(const std::string& remotePath) override;

    // 测试辅助：设置模拟的远程文件列表
    void setFileEntries(const std::vector<FileEntry>& entries);
    // 设置传输行为
    void setTransferDelay(int delayMs);
    void setTransferShouldFail(bool fail);

private:
    std::vector<FileEntry> fileEntries_;
    int transferDelay_{0};
    bool transferShouldFail_{false};
};

}  // namespace remote_sqlite_qt
