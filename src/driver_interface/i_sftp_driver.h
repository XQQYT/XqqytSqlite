#pragma once

#include <cstdint>
#include <functional>
#include <string>
#include <vector>

#include "types.h"

namespace remote_sqlite_qt {

class ISftpDriver {
public:
    virtual ~ISftpDriver() = default;

    // ===== 目录操作 =====
    // 列出远程目录内容
    virtual std::vector<FileEntry> listDirectory(const std::string& remotePath) = 0;

    // 创建远程目录
    virtual void mkdir(const std::string& remotePath) = 0;

    // 删除远程文件
    virtual void remove(const std::string& remotePath) = 0;

    // ===== 文件传输 =====
    // 下载文件（remotePath → localPath），progressCallback 返回已传输/总字节数
    virtual void download(const std::string& remotePath,
                          const std::string& localPath,
                          ProgressCallback progressCallback) = 0;

    // 上传文件（localPath → remotePath）
    virtual void upload(const std::string& localPath,
                        const std::string& remotePath,
                        ProgressCallback progressCallback) = 0;

    // ===== 文件信息 =====
    virtual bool fileExists(const std::string& remotePath) = 0;
    virtual int64_t getFileSize(const std::string& remotePath) = 0;
    virtual std::string getFileMtime(const std::string& remotePath) = 0;
};

}  // namespace remote_sqlite_qt
