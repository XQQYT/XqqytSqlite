#pragma once

#include <string>

namespace remote_sqlite_qt {

// 构建远程 sqlite3 CLI 命令
class SqliteCommandBuilder {
public:
    // 构建完整命令：sqlite3 -json <dbPath> "<sql>"
    static std::string buildQueryCommand(const std::string& dbPath,
                                         const std::string& sql);

    // 构建 .schema / PRAGMA 命令
    static std::string buildTableListCommand(const std::string& dbPath);
    static std::string buildTableInfoCommand(const std::string& dbPath,
                                             const std::string& tableName);
    static std::string buildIndexListCommand(const std::string& dbPath,
                                             const std::string& tableName);
    static std::string buildRowCountCommand(const std::string& dbPath,
                                            const std::string& tableName);

    // 构建版本检查命令
    static std::string buildVersionCommand();

    // 转义 shell 命令中的单引号
    static std::string escapeSingleQuotes(const std::string& input);
};

}  // namespace remote_sqlite_qt
