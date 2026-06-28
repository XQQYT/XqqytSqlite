#include "ssh_session_guard.h"

#include "libssh2.h"

#include <cstdio>

namespace remote_sqlite_qt {

// ===== SshSessionGuard =====

SshSessionGuard::SshSessionGuard() = default;

SshSessionGuard::~SshSessionGuard() {
    reset();
}

SshSessionGuard::SshSessionGuard(SshSessionGuard&& other) noexcept
    : session_(other.session_) {
    other.session_ = nullptr;
}

SshSessionGuard& SshSessionGuard::operator=(SshSessionGuard&& other) noexcept {
    if (this != &other) {
        reset();
        session_ = other.session_;
        other.session_ = nullptr;
    }
    return *this;
}

bool SshSessionGuard::create() {
    reset();
    session_ = libssh2_session_init();
    return session_ != nullptr;
}

void SshSessionGuard::reset() {
    if (session_) {
        libssh2_session_free(session_);
        session_ = nullptr;
    }
}

// ===== SshChannelGuard =====

SshChannelGuard::SshChannelGuard(LIBSSH2_SESSION* session)
    : session_(session) {}

SshChannelGuard::~SshChannelGuard() {
    reset();
}

SshChannelGuard::SshChannelGuard(SshChannelGuard&& other) noexcept
    : session_(other.session_), channel_(other.channel_) {
    other.channel_ = nullptr;
}

SshChannelGuard& SshChannelGuard::operator=(SshChannelGuard&& other) noexcept {
    if (this != &other) {
        reset();
        session_ = other.session_;
        channel_ = other.channel_;
        other.channel_ = nullptr;
    }
    return *this;
}

bool SshChannelGuard::open() {
    reset();
    channel_ = libssh2_channel_open_session(session_);
    return channel_ != nullptr;
}

void SshChannelGuard::reset() {
    if (channel_) {
        libssh2_channel_free(channel_);
        channel_ = nullptr;
    }
}

// ===== KnownHostsGuard =====

KnownHostsGuard::KnownHostsGuard() = default;

KnownHostsGuard::~KnownHostsGuard() {
    if (hosts_) {
        libssh2_knownhost_free(hosts_);
    }
}

bool KnownHostsGuard::loadFromFile(const std::string& filePath) {
    hosts_ = libssh2_knownhost_init(nullptr);
    if (!hosts_) return false;

    int count = libssh2_knownhost_readfile(
        hosts_, filePath.c_str(),
        LIBSSH2_KNOWNHOST_FILE_OPENSSH);
    // count >= 0 表示成功（0 表示空文件也是 OK 的）
    return count >= 0;
}

bool KnownHostsGuard::saveToFile(const std::string& filePath) {
    if (!hosts_) return false;
    return libssh2_knownhost_writefile(hosts_, filePath.c_str(),
                                       LIBSSH2_KNOWNHOST_FILE_OPENSSH) == 0;
}

}  // namespace remote_sqlite_qt
