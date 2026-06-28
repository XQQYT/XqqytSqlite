#pragma once

#include <string>

// libssh2 前向声明
struct _LIBSSH2_SESSION;
typedef struct _LIBSSH2_SESSION LIBSSH2_SESSION;
struct _LIBSSH2_KNOWNHOSTS;
typedef struct _LIBSSH2_KNOWNHOSTS LIBSSH2_KNOWNHOSTS;

namespace remote_sqlite_qt {

class KnownHostsGuard;

// Known Hosts 校验结果
enum class KnownHostResult {
    Match,          // 指纹匹配，安全
    Mismatch,       // 指纹不匹配——中间人攻击风险！
    NotFound,       // host key 不在记录中（首次连接）
    Error,          // 校验过程出错
};

class KnownHostsVerifier {
public:
    KnownHostsVerifier();
    ~KnownHostsVerifier();

    KnownHostsVerifier(const KnownHostsVerifier&) = delete;
    KnownHostsVerifier& operator=(const KnownHostsVerifier&) = delete;

    // 从 known_hosts 文件加载
    bool loadKnownHosts(const std::string& filePath);

    // 校验远端 host key
    // host: 主机名或 IP
    // port: SSH 端口
    KnownHostResult verify(const std::string& host, int port,
                           LIBSSH2_SESSION* session);

    // 将新的 host key 添加到 known_hosts 集合并保存
    bool addAndSave(const std::string& host, int port,
                    LIBSSH2_SESSION* session,
                    const std::string& filePath);

    // 获取已知的主机列表
    std::string knownHostsPath() const { return knownHostsPath_; }

private:
    std::string knownHostsPath_;
    KnownHostsGuard* hosts_{nullptr};
};

}  // namespace remote_sqlite_qt
