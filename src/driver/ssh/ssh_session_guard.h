#pragma once

#include <memory>
#include <string>

// libssh2 前向声明（避免头文件污染）
struct _LIBSSH2_SESSION;
struct _LIBSSH2_CHANNEL;
struct _LIBSSH2_KNOWNHOSTS;
typedef struct _LIBSSH2_SESSION LIBSSH2_SESSION;
typedef struct _LIBSSH2_CHANNEL LIBSSH2_CHANNEL;
typedef struct _LIBSSH2_KNOWNHOSTS LIBSSH2_KNOWNHOSTS;

namespace remote_sqlite_qt {

// RAII 包装 libssh2_session
class SshSessionGuard {
public:
    SshSessionGuard();
    ~SshSessionGuard();

    SshSessionGuard(const SshSessionGuard&) = delete;
    SshSessionGuard& operator=(const SshSessionGuard&) = delete;
    SshSessionGuard(SshSessionGuard&& other) noexcept;
    SshSessionGuard& operator=(SshSessionGuard&& other) noexcept;

    LIBSSH2_SESSION* get() const { return session_; }
    explicit operator bool() const { return session_ != nullptr; }

    // 创建新 session
    bool create();

    // 释放 session
    void reset();

private:
    LIBSSH2_SESSION* session_{nullptr};
};

// RAII 包装 libssh2_channel
class SshChannelGuard {
public:
    explicit SshChannelGuard(LIBSSH2_SESSION* session);
    ~SshChannelGuard();

    SshChannelGuard(const SshChannelGuard&) = delete;
    SshChannelGuard& operator=(const SshChannelGuard&) = delete;
    SshChannelGuard(SshChannelGuard&& other) noexcept;
    SshChannelGuard& operator=(SshChannelGuard&& other) noexcept;

    LIBSSH2_CHANNEL* get() const { return channel_; }
    explicit operator bool() const { return channel_ != nullptr; }

    // 打开会话通道
    bool open();

    // 关闭通道
    void reset();

private:
    LIBSSH2_SESSION* session_{nullptr};
    LIBSSH2_CHANNEL* channel_{nullptr};
};

// RAII 包装 libssh2_knownhost 集合
class KnownHostsGuard {
public:
    KnownHostsGuard();
    ~KnownHostsGuard();

    KnownHostsGuard(const KnownHostsGuard&) = delete;
    KnownHostsGuard& operator=(const KnownHostsGuard&) = delete;

    LIBSSH2_KNOWNHOSTS* get() const { return hosts_; }
    explicit operator bool() const { return hosts_ != nullptr; }

    // 从文件加载 known_hosts
    bool loadFromFile(const std::string& filePath);

    // 保存到文件
    bool saveToFile(const std::string& filePath);

private:
    LIBSSH2_KNOWNHOSTS* hosts_{nullptr};
};

}  // namespace remote_sqlite_qt
