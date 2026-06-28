#include "libssh2_sftp_driver.h"

#include "libssh2.h"
#include "libssh2_sftp.h"

#include <cstdio>
#include <cstring>
#include <chrono>
#include <fstream>
#include <thread>

#include "spdlog/spdlog.h"

namespace remote_sqlite_qt {

LibSsh2SftpDriver::LibSsh2SftpDriver(LIBSSH2_SESSION* sshSession)
    : sshSession_(sshSession) {}

LibSsh2SftpDriver::~LibSsh2SftpDriver() {
    if (sftpSession_) {
        libssh2_sftp_shutdown(sftpSession_);
        sftpSession_ = nullptr;
    }
}

bool LibSsh2SftpDriver::init() {
    if (initialized_) return true;
    if (!sshSession_) {
        spdlog::error("SFTP: no SSH session available");
        return false;
    }

    sftpSession_ = libssh2_sftp_init(sshSession_);
    if (!sftpSession_) {
        spdlog::error("SFTP: libssh2_sftp_init() failed");
        return false;
    }

    initialized_ = true;
    spdlog::info("SFTP subsystem initialized");
    return true;
}

std::vector<FileEntry> LibSsh2SftpDriver::listDirectory(
    const std::string& remotePath) {
    std::vector<FileEntry> entries;
    if (!init()) return entries;

    LIBSSH2_SFTP_HANDLE* handle = libssh2_sftp_opendir(
        sftpSession_, remotePath.c_str());
    if (!handle) {
        spdlog::error("SFTP: cannot open directory: {}", remotePath);
        return entries;
    }

    char buffer[512];
    LIBSSH2_SFTP_ATTRIBUTES attrs;

    while (true) {
        int rc = libssh2_sftp_readdir(handle, buffer, sizeof(buffer), &attrs);
        if (rc <= 0) break;  // EOF 或错误

        std::string name(buffer);
        if (name == "." || name == "..") continue;

        FileEntry entry;
        entry.name = name;
        entry.isDirectory = LIBSSH2_SFTP_S_ISDIR(attrs.permissions);
        entry.size = static_cast<int64_t>(attrs.filesize);
        entry.permissions = std::to_string(attrs.permissions);

        // 格式化修改时间
        if (attrs.mtime > 0) {
            char timeBuf[32];
            time_t mtime = static_cast<time_t>(attrs.mtime);
            strftime(timeBuf, sizeof(timeBuf), "%Y-%m-%dT%H:%M:%S",
                     gmtime(&mtime));
            entry.mtime = timeBuf;
        }

        entries.push_back(entry);
    }

    libssh2_sftp_closedir(handle);
    return entries;
}

void LibSsh2SftpDriver::mkdir(const std::string& remotePath) {
    if (!init()) return;
    int rc = libssh2_sftp_mkdir(sftpSession_, remotePath.c_str(),
                                LIBSSH2_SFTP_S_IRWXU |
                                LIBSSH2_SFTP_S_IRGRP | LIBSSH2_SFTP_S_IXGRP |
                                LIBSSH2_SFTP_S_IROTH | LIBSSH2_SFTP_S_IXOTH);
    if (rc != 0) {
        spdlog::error("SFTP: mkdir failed: {}", remotePath);
    }
}

void LibSsh2SftpDriver::remove(const std::string& remotePath) {
    if (!init()) return;
    int rc = libssh2_sftp_unlink(sftpSession_, remotePath.c_str());
    if (rc != 0) {
        spdlog::error("SFTP: remove failed: {}", remotePath);
    }
}

void LibSsh2SftpDriver::download(const std::string& remotePath,
                                  const std::string& localPath,
                                  ProgressCallback progressCallback) {
    if (!init()) return;

    LIBSSH2_SFTP_HANDLE* handle = libssh2_sftp_open(
        sftpSession_, remotePath.c_str(),
        LIBSSH2_FXF_READ, 0);
    if (!handle) {
        spdlog::error("SFTP: cannot open remote file for read: {}", remotePath);
        throw std::runtime_error("Cannot open remote file: " + remotePath);
    }

    // 获取文件大小
    LIBSSH2_SFTP_ATTRIBUTES attrs;
    libssh2_sftp_fstat(handle, &attrs);
    int64_t totalSize = static_cast<int64_t>(attrs.filesize);

    // 打开本地文件
    std::ofstream localFile(localPath, std::ios::binary);
    if (!localFile.is_open()) {
        libssh2_sftp_close(handle);
        throw std::runtime_error("Cannot open local file: " + localPath);
    }

    // 分块下载（64KB 缓冲）
    constexpr size_t kBufferSize = 65536;
    auto buffer = std::make_unique<char[]>(kBufferSize);
    int64_t transferred = 0;

    while (true) {
        ssize_t n = libssh2_sftp_read(handle, buffer.get(), kBufferSize);
        if (n > 0) {
            localFile.write(buffer.get(), n);
            transferred += n;
            if (progressCallback) {
                progressCallback(transferred, totalSize);
            }
        } else if (n == LIBSSH2_ERROR_EAGAIN) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            continue;
        } else {
            break;  // EOF (n == 0) 或错误
        }
    }

    localFile.close();
    libssh2_sftp_close(handle);

    spdlog::info("SFTP: downloaded {} → {} ({} / {} bytes)",
                 remotePath, localPath, transferred, totalSize);
}

void LibSsh2SftpDriver::upload(const std::string& localPath,
                                const std::string& remotePath,
                                ProgressCallback progressCallback) {
    if (!init()) return;

    // 获取本地文件大小
    std::ifstream localFile(localPath, std::ios::binary | std::ios::ate);
    if (!localFile.is_open()) {
        throw std::runtime_error("Cannot open local file: " + localPath);
    }
    auto totalSize = static_cast<int64_t>(localFile.tellg());
    localFile.seekg(0);

    // 打开远程文件（覆盖写入）
    LIBSSH2_SFTP_HANDLE* handle = libssh2_sftp_open(
        sftpSession_, remotePath.c_str(),
        LIBSSH2_FXF_WRITE | LIBSSH2_FXF_CREAT | LIBSSH2_FXF_TRUNC,
        LIBSSH2_SFTP_S_IRUSR | LIBSSH2_SFTP_S_IWUSR |
        LIBSSH2_SFTP_S_IRGRP | LIBSSH2_SFTP_S_IROTH);
    if (!handle) {
        spdlog::error("SFTP: cannot open remote file for write: {}", remotePath);
        throw std::runtime_error("Cannot open remote file: " + remotePath);
    }

    // 分块上传（64KB 缓冲）
    constexpr size_t kBufferSize = 65536;
    auto buffer = std::make_unique<char[]>(kBufferSize);
    int64_t transferred = 0;

    while (localFile) {
        localFile.read(buffer.get(), kBufferSize);
        auto bytesRead = static_cast<size_t>(localFile.gcount());
        if (bytesRead == 0) break;

        size_t written = 0;
        while (written < bytesRead) {
            ssize_t n = libssh2_sftp_write(handle, buffer.get() + written,
                                           bytesRead - written);
            if (n > 0) {
                written += static_cast<size_t>(n);
                transferred += n;
                if (progressCallback) {
                    progressCallback(transferred, totalSize);
                }
            } else if (n == LIBSSH2_ERROR_EAGAIN) {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            } else {
                libssh2_sftp_close(handle);
                localFile.close();
                throw std::runtime_error("SFTP write error");
            }
        }
    }

    localFile.close();
    libssh2_sftp_close(handle);

    spdlog::info("SFTP: uploaded {} → {} ({} bytes)",
                 localPath, remotePath, transferred);
}

bool LibSsh2SftpDriver::fileExists(const std::string& remotePath) {
    if (!init()) return false;

    LIBSSH2_SFTP_ATTRIBUTES attrs;
    int rc = libssh2_sftp_stat(sftpSession_, remotePath.c_str(), &attrs);
    return rc == 0;
}

int64_t LibSsh2SftpDriver::getFileSize(const std::string& remotePath) {
    if (!init()) return -1;

    LIBSSH2_SFTP_ATTRIBUTES attrs;
    int rc = libssh2_sftp_stat(sftpSession_, remotePath.c_str(), &attrs);
    return (rc == 0) ? static_cast<int64_t>(attrs.filesize) : -1;
}

std::string LibSsh2SftpDriver::getFileMtime(const std::string& remotePath) {
    if (!init()) return "";

    LIBSSH2_SFTP_ATTRIBUTES attrs;
    int rc = libssh2_sftp_stat(sftpSession_, remotePath.c_str(), &attrs);
    if (rc != 0 || attrs.mtime == 0) return "";

    char buf[32];
    time_t t = static_cast<time_t>(attrs.mtime);
    strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%S", gmtime(&t));
    return std::string(buf);
}

}  // namespace remote_sqlite_qt
