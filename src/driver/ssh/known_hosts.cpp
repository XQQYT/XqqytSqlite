#include "known_hosts.h"

#include "libssh2.h"
#include "ssh_session_guard.h"

#include <string>

namespace remote_sqlite_qt {

KnownHostsVerifier::KnownHostsVerifier() = default;

KnownHostsVerifier::~KnownHostsVerifier() {
    delete hosts_;
}

bool KnownHostsVerifier::loadKnownHosts(const std::string& filePath) {
    knownHostsPath_ = filePath;
    delete hosts_;
    hosts_ = new KnownHostsGuard();
    return hosts_->loadFromFile(filePath);
}

KnownHostResult KnownHostsVerifier::verify(const std::string& host, int port,
                                            LIBSSH2_SESSION* session) {
    if (!session) return KnownHostResult::Error;

    // 获取远端 host key 指纹
    size_t keyLen = 0;
    int keyType = 0;
    const char* keyRaw = libssh2_session_hostkey(session, &keyLen, &keyType);
    if (!keyRaw) return KnownHostResult::Error;

    // 计算指纹
    const char* fingerprint = libssh2_hostkey_hash(session, LIBSSH2_HOSTKEY_HASH_SHA256);
    if (!fingerprint) return KnownHostResult::Error;

    if (!hosts_ || !hosts_->get()) {
        // 无 known_hosts 文件，视为首次连接
        return KnownHostResult::NotFound;
    }

    // 在 known_hosts 中查找
    int checkResult = libssh2_knownhost_checkp(
        hosts_->get(),
        host.c_str(), port,
        keyRaw, keyLen,
        LIBSSH2_KNOWNHOST_TYPE_PLAIN |
        LIBSSH2_KNOWNHOST_KEYENC_RAW,
        nullptr);

    switch (checkResult) {
    case LIBSSH2_KNOWNHOST_CHECK_MATCH:
        return KnownHostResult::Match;
    case LIBSSH2_KNOWNHOST_CHECK_MISMATCH:
        return KnownHostResult::Mismatch;
    case LIBSSH2_KNOWNHOST_CHECK_NOTFOUND:
        return KnownHostResult::NotFound;
    default:
        return KnownHostResult::Error;
    }
}

bool KnownHostsVerifier::addAndSave(const std::string& host, int port,
                                     LIBSSH2_SESSION* session,
                                     const std::string& filePath) {
    if (!session) return false;

    size_t keyLen = 0;
    int keyType = 0;
    const char* keyRaw = libssh2_session_hostkey(session, &keyLen, &keyType);
    if (!keyRaw) return false;

    if (!hosts_ || !hosts_->get()) {
        delete hosts_;
        hosts_ = new KnownHostsGuard();
        hosts_->loadFromFile("");
    }

    int rc = libssh2_knownhost_addc(
        hosts_->get(),
        host.c_str(), nullptr,  // nullptr = 无别名
        keyRaw, keyLen,
        nullptr, 0,              // 注释
        LIBSSH2_KNOWNHOST_TYPE_PLAIN |
        LIBSSH2_KNOWNHOST_KEYENC_RAW,
        nullptr);

    if (rc != 0) return false;

    return hosts_->saveToFile(filePath);
}

}  // namespace remote_sqlite_qt
