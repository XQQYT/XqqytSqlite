#pragma once

#include <string_view>

namespace remote_sqlite_qt::events {

// ===== SSH 连接事件 =====
constexpr std::string_view SshConnected{"ssh:connected"};
// payload: std::string connectionId

constexpr std::string_view SshDisconnected{"ssh:disconnected"};
// payload: std::string connectionId, std::string reason

constexpr std::string_view SshError{"ssh:error"};
// payload: std::string connectionId, std::string message, int code

constexpr std::string_view SshReconnecting{"ssh:reconnecting"};
// payload: std::string connectionId, int attempt

// ===== 查询事件 =====
constexpr std::string_view QueryRequest{"query:request"};
// payload: std::string connectionId, std::string dbPath, std::string sql, std::string requestId

constexpr std::string_view QueryResult{"query:result"};
// payload: std::string requestId, QueryResult result

constexpr std::string_view QueryError{"query:error"};
// payload: std::string requestId, std::string error

// ===== SFTP 事件 =====
constexpr std::string_view SftpProgress{"sftp:progress"};
// payload: std::string taskId, int64_t transferred, int64_t total

constexpr std::string_view SftpComplete{"sftp:complete"};
// payload: std::string taskId, std::string localPath

constexpr std::string_view SftpError{"sftp:error"};
// payload: std::string taskId, std::string error

constexpr std::string_view SftpFileList{"sftp:file_list"};
// payload: std::string path, std::vector<FileEntry> entries

// ===== 同步事件 =====
constexpr std::string_view SyncConflict{"sync:conflict"};
// payload: SyncConflict

constexpr std::string_view SyncComplete{"sync:complete"};
// payload: std::string remotePath

constexpr std::string_view SyncError{"sync:error"};
// payload: std::string message

// ===== 数据库浏览器事件 =====
constexpr std::string_view DbTablesLoaded{"db:tables_loaded"};
// payload: std::string connectionId, std::string dbPath, std::vector<TableInfo> tables

constexpr std::string_view DbTableData{"db:table_data"};
// payload: std::string tableName, TableData data

// ===== 连接管理事件 =====
constexpr std::string_view ConnectionTestResult{"connection:test_result"};
// payload: std::string host, bool success, std::string message

constexpr std::string_view ConnectionListChanged{"connection:list_changed"};
// payload: std::vector<ConnectionInfo>

// ===== 主题/设置事件 =====
constexpr std::string_view ThemeChanged{"theme:changed"};
// payload: std::string themeName  ("dark" / "light")

constexpr std::string_view LanguageChanged{"language:changed"};
// payload: std::string langCode  ("zh" / "en")

}  // namespace remote_sqlite_qt::events
