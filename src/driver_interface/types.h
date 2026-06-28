#pragma once

#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace remote_sqlite_qt {

// ===== SSH 配置 =====
enum class AuthMethod {
    Password,
    PublicKey,
    SshAgent,
};

struct SshConfig {
    std::string host;
    int port{22};
    std::string username;
    AuthMethod authMethod{AuthMethod::Password};
    std::string password;         // AuthMethod == Password
    std::string privateKeyPath;   // AuthMethod == PublicKey
    std::string passphrase;       // 可选私钥密码
    std::string jumpHost;         // 格式: user@host:port
    int connectTimeout{30};       // 秒
    int keepAliveInterval{60};    // 秒
    std::string knownHostsPath;   // known_hosts 文件路径
};

// ===== 连接状态 =====
enum class ConnectionState {
    Disconnected,
    Connecting,
    Connected,
    Error,
};

// ===== SSH 命令执行结果 =====
struct ExecResult {
    int exitCode{0};
    std::string stdOut;
    std::string stdErr;
};

// ===== SFTP 文件条目 =====
struct FileEntry {
    std::string name;
    bool isDirectory{false};
    int64_t size{0};
    std::string mtime;  // 修改时间 ISO 8601
    std::string permissions;
};

// ===== 查询结果 =====
struct QueryResult {
    std::vector<std::string> columns;
    std::vector<std::vector<std::string>> rows;
    double elapsedMs{0.0};
    int totalRows{0};  // 总行数（分页时大于 rows.size()）
};

// ===== 表格信息 =====
struct TableInfo {
    std::string name;
    std::string type;  // table, view, index, trigger
    std::string sql;   // 建表 DDL
};

// ===== 列信息 =====
struct ColumnInfo {
    int cid{0};
    std::string name;
    std::string type;
    bool notNull{false};
    std::string defaultValue;
    bool primaryKey{false};
};

// ===== 索引信息 =====
struct IndexInfo {
    std::string name;
    std::string tableName;
    bool unique{false};
    std::vector<std::string> columns;
    std::string sql;
};

// ===== 同步冲突信息 =====
struct SyncConflict {
    std::string localPath;
    std::string remotePath;
    std::string localMtime;
    std::string remoteMtime;
    int64_t localSize{0};
    int64_t remoteSize{0};
};

// ===== 连接信息 =====
struct ConnectionInfo {
    std::string connectionId;
    std::string host;
    std::string username;
    ConnectionState state{ConnectionState::Disconnected};
};

// ===== 进度回调类型 =====
using ProgressCallback = std::function<void(int64_t transferred, int64_t total)>;

}  // namespace remote_sqlite_qt
