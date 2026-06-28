#include "remote_sqlite_driver.h"

#include "json.hpp"

#include "spdlog/spdlog.h"

namespace remote_sqlite_qt {

RemoteSqliteDriver::RemoteSqliteDriver(std::shared_ptr<ISshDriver> sshDriver)
    : sshDriver_(std::move(sshDriver)) {}

void RemoteSqliteDriver::setConnection(const std::string& connectionId,
                                        const std::string& dbPath) {
    connectionId_ = connectionId;
    dbPath_ = dbPath;
}

bool RemoteSqliteDriver::isSqlite3Available() {
    auto result = execRemote("sqlite3 --version");
    return result.exitCode == 0 && !result.stdOut.empty();
}

std::string RemoteSqliteDriver::getSqliteVersion() {
    auto result = execRemote("sqlite3 --version");
    if (result.exitCode == 0) {
        // 输出格式: "3.45.0 2024-01-15 ..."
        auto pos = result.stdOut.find(' ');
        return (pos != std::string::npos) ? result.stdOut.substr(0, pos) : result.stdOut;
    }
    return "";
}

QueryResult RemoteSqliteDriver::query(const std::string& sql) {
    auto command = SqliteCommandBuilder::buildQueryCommand(dbPath_, sql);
    spdlog::debug("Remote query: {}", command);

    auto result = execRemote(command);
    if (result.exitCode != 0) {
        errorMessage_ = result.stdErr.empty() ? result.stdOut : result.stdErr;
        return {};
    }
    return parseJsonResult(result.stdOut);
}

bool RemoteSqliteDriver::execute(const std::string& sql) {
    auto command = SqliteCommandBuilder::buildQueryCommand(dbPath_, sql);
    auto result = execRemote(command);
    if (result.exitCode != 0) {
        errorMessage_ = result.stdErr;
        return false;
    }
    return true;
}

std::vector<TableInfo> RemoteSqliteDriver::getTables() {
    auto command = SqliteCommandBuilder::buildTableListCommand(dbPath_);
    auto result = execRemote(command);
    if (result.exitCode != 0) return {};

    auto j = nlohmann::json::parse(result.stdOut, nullptr, false);
    if (!j.is_array()) return {};

    std::vector<TableInfo> tables;
    for (const auto& item : j) {
        TableInfo info;
        info.name = item.value("name", "");
        info.type = item.value("type", "");
        info.sql = item.value("sql", "");
        tables.push_back(info);
    }
    return tables;
}

std::vector<ColumnInfo> RemoteSqliteDriver::getTableInfo(const std::string& tableName) {
    auto command = SqliteCommandBuilder::buildTableInfoCommand(dbPath_, tableName);
    auto result = execRemote(command);
    if (result.exitCode != 0) return {};

    auto j = nlohmann::json::parse(result.stdOut, nullptr, false);
    if (!j.is_array()) return {};

    std::vector<ColumnInfo> columns;
    for (const auto& item : j) {
        ColumnInfo col;
        col.cid = item.value("cid", 0);
        col.name = item.value("name", "");
        col.type = item.value("type", "");
        col.notNull = item.value("notnull", 0) == 1;
        col.defaultValue = item.value("dflt_value", "");
        col.primaryKey = item.value("pk", 0) == 1;
        columns.push_back(col);
    }
    return columns;
}

std::vector<IndexInfo> RemoteSqliteDriver::getIndexes(const std::string& tableName) {
    auto command = SqliteCommandBuilder::buildIndexListCommand(dbPath_, tableName);
    auto result = execRemote(command);
    if (result.exitCode != 0) return {};

    auto j = nlohmann::json::parse(result.stdOut, nullptr, false);
    if (!j.is_array()) return {};

    std::vector<IndexInfo> indexes;
    for (const auto& item : j) {
        IndexInfo idx;
        idx.name = item.value("name", "");
        idx.tableName = tableName;
        idx.unique = item.value("unique", 0) == 1;
        idx.sql = item.value("sql", "");
        // 索引列需要通过 PRAGMA index_info 单独获取
        indexes.push_back(idx);
    }
    return indexes;
}

int64_t RemoteSqliteDriver::getRowCount(const std::string& tableName) {
    auto command = SqliteCommandBuilder::buildRowCountCommand(dbPath_, tableName);
    auto result = execRemote(command);
    if (result.exitCode != 0) return 0;

    auto j = nlohmann::json::parse(result.stdOut, nullptr, false);
    if (j.is_array() && !j.empty()) {
        return j[0].value("COUNT(*)", 0);
    }
    return 0;
}

std::string RemoteSqliteDriver::lastError() const {
    return errorMessage_;
}

ExecResult RemoteSqliteDriver::execRemote(const std::string& command) {
    if (!sshDriver_ || connectionId_.empty()) {
        errorMessage_ = "Not connected";
        return ExecResult{-1, "", "Not connected"};
    }
    return sshDriver_->execCommand(connectionId_, command);
}

QueryResult RemoteSqliteDriver::parseJsonResult(const std::string& jsonStr) {
    QueryResult result;
    auto j = nlohmann::json::parse(jsonStr, nullptr, false);
    if (!j.is_array()) {
        errorMessage_ = "Invalid JSON response";
        return result;
    }

    // 提取列名（从第一行获取 keys）
    if (!j.empty() && j[0].is_object()) {
        for (auto& [key, _] : j[0].items()) {
            result.columns.push_back(key);
        }
    }

    // 提取行数据
    for (const auto& item : j) {
        std::vector<std::string> row;
        for (const auto& col : result.columns) {
            if (item.contains(col)) {
                const auto& val = item[col];
                if (val.is_null()) row.push_back("NULL");
                else if (val.is_string()) row.push_back(val.get<std::string>());
                else row.push_back(val.dump());
            } else {
                row.push_back("NULL");
            }
        }
        result.rows.push_back(std::move(row));
    }

    result.totalRows = static_cast<int>(result.rows.size());
    return result;
}

}  // namespace remote_sqlite_qt
