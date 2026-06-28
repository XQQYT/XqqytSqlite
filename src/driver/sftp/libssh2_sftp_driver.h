#pragma once

#include <memory>
#include <string>
#include <vector>

#include "driver_interface/i_sftp_driver.h"
#include "driver_interface/types.h"

struct _LIBSSH2_SESSION;
struct _LIBSSH2_SFTP;
typedef struct _LIBSSH2_SESSION LIBSSH2_SESSION;
typedef struct _LIBSSH2_SFTP LIBSSH2_SFTP;

namespace remote_sqlite_qt {

// LibSSH2 实现的 SFTP 驱动——封装 libssh2_sftp_* C API
class LibSsh2SftpDriver : public ISftpDriver {
public:
    explicit LibSsh2SftpDriver(LIBSSH2_SESSION* sshSession);
    ~LibSsh2SftpDriver() override;

    LibSsh2SftpDriver(const LibSsh2SftpDriver&) = delete;
    LibSsh2SftpDriver& operator=(const LibSsh2SftpDriver&) = delete;

    // 初始化 SFTP 子系统
    bool init();

    // ISftpDriver 接口
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

private:
    LIBSSH2_SESSION* sshSession_{nullptr};
    LIBSSH2_SFTP* sftpSession_{nullptr};
    bool initialized_{false};
};

}  // namespace remote_sqlite_qt
