#include "libssh2_ssh_driver.h"

#include "driver/sftp/libssh2_sftp_driver.h"
#include "libssh2.h"
#include "spdlog/spdlog.h"

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#define SOCKET int
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define closesocket close
#endif

#include <cstring>
#include <chrono>
#include <thread>

namespace remote_sqlite_qt {

LibSsh2SshDriver::LibSsh2SshDriver() {
#ifdef _WIN32
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif
    // 初始化 libssh2
    libssh2_init(0);
    knownHostsVerifier_ = std::make_unique<KnownHostsVerifier>();
}

LibSsh2SshDriver::~LibSsh2SshDriver() {
    // 断开所有连接
    for (auto& [id, _] : sessions_) {
        disconnect(id);
    }
    sessions_.clear();
    libssh2_exit();
#ifdef _WIN32
    WSACleanup();
#endif
}

void LibSsh2SshDriver::setKnownHostsPath(const std::string& path) {
    knownHostsPath_ = path;
    knownHostsVerifier_->loadKnownHosts(path);
}

std::string LibSsh2SshDriver::connect(const SshConfig& config) {
    spdlog::info("LibSSH2: connecting to {}@{}:{}", config.username, config.host, config.port);

    // 1. 解析跳板机（如果配置了）
    std::string actualHost = config.host;
    int actualPort = config.port;
    if (!config.jumpHost.empty()) {
        // ProxyJump 格式: user@host:port
        auto atPos = config.jumpHost.find('@');
        auto colonPos = config.jumpHost.find(':');
        std::string jumpUser = config.jumpHost.substr(0, atPos);
        std::string jumpHost = config.jumpHost.substr(
            atPos + 1, colonPos - atPos - 1);
        int jumpPort = std::stoi(config.jumpHost.substr(colonPos + 1));
        spdlog::debug("Jump host: {}@{}:{}", jumpUser, jumpHost, jumpPort);

        // TODO: 实现真正的跳板机隧道（当前版本直接连接）
        // 跳板机实现需要：连接 A→跳板机→端口转发→B，v1.0 暂不支持
        spdlog::warn("ProxyJump is not yet implemented, connecting directly");
    }

    // 2. 创建 socket
    int sock = createSocket(actualHost, actualPort, config.connectTimeout);
    if (sock == INVALID_SOCKET) {
        spdlog::error("Failed to create socket to {}:{}", actualHost, actualPort);
        return "";
    }

    // 3. 创建 SSH session
    auto session = std::make_unique<SshSession>();
    session->config = config;
    if (!session->session.create()) {
        spdlog::error("libssh2_session_init() failed");
        closesocket(sock);
        return "";
    }

    // 4. 设置 socket 为非阻塞（libssh2 推荐）
#ifdef _WIN32
    u_long mode = 1;
    ioctlsocket(sock, FIONBIO, &mode);
#else
    int flags = fcntl(sock, F_GETFL, 0);
    fcntl(sock, F_SETFL, flags | O_NONBLOCK);
#endif

    // 5. SSH 握手
    int rc = libssh2_session_handshake(session->session.get(), sock);
    if (rc) {
        spdlog::error("SSH handshake failed: {}", rc);
        closesocket(sock);
        return "";
    }

    // 6. Known hosts 校验
    if (knownHostsVerifier_ && !knownHostsPath_.empty()) {
        auto result = knownHostsVerifier_->verify(actualHost, actualPort,
                                                   session->session.get());
        switch (result) {
        case KnownHostResult::Mismatch:
            spdlog::error("Host key mismatch for {} — possible MITM attack!", actualHost);
            closesocket(sock);
            return "";
        case KnownHostResult::NotFound:
            spdlog::info("New host key for {}, adding to known_hosts", actualHost);
            knownHostsVerifier_->addAndSave(actualHost, actualPort,
                                            session->session.get(),
                                            knownHostsPath_);
            break;
        case KnownHostResult::Error:
            spdlog::warn("Known host verification error, continuing anyway");
            break;
        default:
            break;
        }
    }

    // 7. 认证
    if (!authenticate(*session, config)) {
        spdlog::error("Authentication failed for {}", config.username);
        closesocket(sock);
        return "";
    }

    // 8. 注册 session
    auto connId = nextConnectionId();
    session->connectionId = connId;
    session->state = ConnectionState::Connected;
    session->hostKeyFingerprint = getFingerprint(session->session.get());

    spdlog::info("Connected: {} (fingerprint: {})", connId, session->hostKeyFingerprint);
    sessions_[connId] = std::move(session);

    return connId;
}

void LibSsh2SshDriver::disconnect(const std::string& connectionId) {
    auto it = sessions_.find(connectionId);
    if (it == sessions_.end()) return;

    // 先发送断开请求，再释放 session
    libssh2_session_disconnect(it->second->session.get(), "Client disconnecting");
    sessions_.erase(it);
    spdlog::info("Disconnected: {}", connectionId);
}

bool LibSsh2SshDriver::isConnected(const std::string& connectionId) const {
    auto it = sessions_.find(connectionId);
    if (it == sessions_.end()) return false;
    // 检查是否仍有活跃会话
    return it->second->state == ConnectionState::Connected;
}

ExecResult LibSsh2SshDriver::execCommand(const std::string& connectionId,
                                          const std::string& command,
                                          int timeoutSec) {
    ExecResult result;
    auto it = sessions_.find(connectionId);
    if (it == sessions_.end()) {
        result.stdErr = "Connection not found";
        result.exitCode = -1;
        return result;
    }

    auto* sess = it->second->session.get();
    SshChannelGuard channel(sess);
    if (!channel.open()) {
        int err = libssh2_session_last_errno(sess);
        result.stdErr = "Failed to open channel: " + std::to_string(err);
        result.exitCode = -1;
        return result;
    }

    // 执行命令
    int rc = libssh2_channel_exec(channel.get(), command.c_str());
    if (rc != 0) {
        result.stdErr = "Failed to execute command: " + std::to_string(rc);
        result.exitCode = -1;
        return result;
    }

    // 读取输出（带超时的阻塞读取）
    char buffer[4096];
    auto startTime = std::chrono::steady_clock::now();

    // 读 stdout
    while (true) {
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::steady_clock::now() - startTime).count();
        if (elapsed > timeoutSec) {
            result.stdErr = "Command timed out";
            result.exitCode = -1;
            return result;
        }

        ssize_t n = libssh2_channel_read(channel.get(), buffer, sizeof(buffer));
        if (n > 0) {
            result.stdOut.append(buffer, static_cast<size_t>(n));
        } else if (n == LIBSSH2_ERROR_EAGAIN) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            continue;
        } else {
            break;  // n == 0 或错误
        }
    }

    // 读 stderr
    while (true) {
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::steady_clock::now() - startTime).count();
        if (elapsed > timeoutSec) break;

        ssize_t n = libssh2_channel_read_stderr(channel.get(), buffer, sizeof(buffer));
        if (n > 0) {
            result.stdErr.append(buffer, static_cast<size_t>(n));
        } else {
            break;
        }
    }

    // 等待退出并获取 exit code
    libssh2_channel_wait_closed(channel.get());
    result.exitCode = libssh2_channel_get_exit_status(channel.get());

    return result;
}

std::unique_ptr<ISftpDriver> LibSsh2SshDriver::createSftpDriver(
    const std::string& connectionId) {
    auto it = sessions_.find(connectionId);
    if (it == sessions_.end()) return nullptr;

    auto driver = std::make_unique<LibSsh2SftpDriver>(
        it->second->session.get());
    return driver;
}

std::string LibSsh2SshDriver::getHostKey(const std::string& connectionId) const {
    auto it = sessions_.find(connectionId);
    return (it != sessions_.end()) ? it->second->hostKeyFingerprint : "";
}

ConnectionInfo LibSsh2SshDriver::getConnectionInfo(
    const std::string& connectionId) const {
    ConnectionInfo info;
    info.connectionId = connectionId;
    auto it = sessions_.find(connectionId);
    if (it != sessions_.end()) {
        info.host = it->second->config.host;
        info.username = it->second->config.username;
        info.state = it->second->state;
    }
    return info;
}

// ===== 私有方法 =====

int LibSsh2SshDriver::createSocket(const std::string& host, int port,
                                    int timeoutSec) {
    // DNS 解析
    struct addrinfo hints = {};
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    struct addrinfo* result = nullptr;
    int gai = getaddrinfo(host.c_str(), std::to_string(port).c_str(),
                          &hints, &result);
    if (gai != 0) {
        spdlog::error("DNS resolution failed for {}: {}", host, gai_strerror(gai));
        return INVALID_SOCKET;
    }

    // 尝试连接
    SOCKET sock = INVALID_SOCKET;
    for (struct addrinfo* rp = result; rp != nullptr; rp = rp->ai_next) {
        sock = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (sock == INVALID_SOCKET) continue;

        if (::connect(sock, rp->ai_addr, static_cast<int>(rp->ai_addrlen)) == 0) {
            break;  // 连接成功
        }

        closesocket(sock);
        sock = INVALID_SOCKET;
    }

    freeaddrinfo(result);
    return sock;
}

bool LibSsh2SshDriver::performHandshake(SshSession& /*sess*/) {
    // 握手逻辑已在 connect() 中实现
    return true;
}

bool LibSsh2SshDriver::authenticate(SshSession& sess, const SshConfig& config) {
    auto* session = sess.session.get();

    // 获取服务端支持的认证方法列表
    char* userAuthList = libssh2_userauth_list(session,
        config.username.c_str(),
        static_cast<unsigned int>(config.username.length()));

    spdlog::debug("Server auth methods: {}",
                  userAuthList ? userAuthList : "(none)");

    switch (config.authMethod) {
    case AuthMethod::Password:
        return libssh2_userauth_password(session,
            config.username.c_str(),
            config.password.c_str()) == 0;

    case AuthMethod::PublicKey:
        return libssh2_userauth_publickey_fromfile(session,
            config.username.c_str(),
            nullptr,  // 公钥文件（nullptr 表示 .pub 后缀）
            config.privateKeyPath.c_str(),
            config.passphrase.empty() ? nullptr : config.passphrase.c_str()) == 0;

    case AuthMethod::SshAgent:
        return libssh2_userauth_publickey_fromfile(session,
            config.username.c_str(),
            nullptr,
            config.privateKeyPath.empty() ? nullptr : config.privateKeyPath.c_str(),
            config.passphrase.empty() ? nullptr : config.passphrase.c_str()) == 0;
    }

    return false;
}

std::string LibSsh2SshDriver::getFingerprint(LIBSSH2_SESSION* session) {
    const char* fp = libssh2_hostkey_hash(session, LIBSSH2_HOSTKEY_HASH_SHA256);
    if (!fp) return "unknown";

    // 转换为十六进制字符串
    std::string hex;
    for (size_t i = 0; i < 32; ++i) {
        char buf[4];
        snprintf(buf, sizeof(buf), "%02x", static_cast<unsigned char>(fp[i]));
        hex += buf;
        if (i < 31) hex += ':';
    }
    return "SHA256:" + hex;
}

std::string LibSsh2SshDriver::nextConnectionId() {
    return "ssh-" + std::to_string(++connectionCounter_);
}

}  // namespace remote_sqlite_qt
