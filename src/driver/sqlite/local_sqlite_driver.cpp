#include "local_sqlite_driver.h"

#include "sqlite3.h"

namespace remote_sqlite_qt {

LocalSqliteDriver::LocalSqliteDriver() = default;

LocalSqliteDriver::~LocalSqliteDriver() {
    close();
}

bool LocalSqliteDriver::open(const std::string& dbPath) {
    if (db_) close();

    int rc = sqlite3_open_v2(dbPath.c_str(), &db_,
                             SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, nullptr);
    if (rc != SQLITE_OK) {
        errorMessage_ = sqlite3_errmsg(db_);
        sqlite3_close(db_);
        db_ = nullptr;
        return false;
    }
    // 启用 JSON 扩展、WAL 模式
    sqlite3_exec(db_, "PRAGMA journal_mode=WAL;", nullptr, nullptr, nullptr);
    return true;
}

void LocalSqliteDriver::close() {
    if (db_) {
        sqlite3_close(db_);
        db_ = nullptr;
    }
}

bool LocalSqliteDriver::isOpen() const {
    return db_ != nullptr;
}

QueryResult LocalSqliteDriver::query(const std::string& sql) {
    return execQuery(sql, true);
}

bool LocalSqliteDriver::execute(const std::string& sql) {
    auto result = execQuery(sql, false);
    return result.elapsedMs >= 0;  // 成功执行
}

std::vector<TableInfo> LocalSqliteDriver::getTables() {
    std::vector<TableInfo> tables;
    auto result = execQuery(
        "SELECT name, type, sql FROM sqlite_master "
        "WHERE type IN ('table','view','index','trigger') "
        "AND name NOT LIKE 'sqlite_%' ORDER BY type, name;",
        true);

    for (const auto& row : result.rows) {
        if (row.size() >= 2) {
            TableInfo info;
            info.name = row[0];
            info.type = row[1];
            info.sql = row.size() > 2 ? row[2] : "";
            tables.push_back(info);
        }
    }
    return tables;
}

std::vector<ColumnInfo> LocalSqliteDriver::getTableInfo(const std::string& tableName) {
    std::vector<ColumnInfo> columns;
    auto result = execQuery("PRAGMA table_info('" + tableName + "');", true);

    for (const auto& row : result.rows) {
        if (row.size() >= 6) {
            ColumnInfo col;
            col.cid = std::stoi(row[0]);
            col.name = row[1];
            col.type = row[2];
            col.notNull = (row[3] == "1");
            col.defaultValue = row[4];
            col.primaryKey = (row[5] == "1");
            columns.push_back(col);
        }
    }
    return columns;
}

std::vector<IndexInfo> LocalSqliteDriver::getIndexes(const std::string& tableName) {
    std::vector<IndexInfo> indexes;
    // 使用 PRAGMA index_list 获取索引列表
    auto listResult = execQuery("PRAGMA index_list('" + tableName + "');", true);
    for (const auto& row : listResult.rows) {
        if (row.size() >= 2) {
            IndexInfo idx;
            idx.name = row[1];
            idx.tableName = tableName;
            idx.unique = (row[2] == "1");

            // 获取索引列
            auto colsResult = execQuery("PRAGMA index_info('" + idx.name + "');", true);
            for (const auto& crow : colsResult.rows) {
                if (crow.size() >= 2) {
                    idx.columns.push_back(crow[2]);
                }
            }
            indexes.push_back(idx);
        }
    }
    return indexes;
}

int64_t LocalSqliteDriver::getRowCount(const std::string& tableName) {
    auto result = execQuery(
        "SELECT COUNT(*) FROM \"" + tableName + "\";", true);
    if (!result.rows.empty() && !result.rows[0].empty()) {
        return std::stoll(result.rows[0][0]);
    }
    return 0;
}

bool LocalSqliteDriver::beginTransaction() {
    return execute("BEGIN TRANSACTION;");
}

bool LocalSqliteDriver::commit() {
    return execute("COMMIT;");
}

bool LocalSqliteDriver::rollback() {
    return execute("ROLLBACK;");
}

int64_t LocalSqliteDriver::lastInsertRowId() {
    return sqlite3_last_insert_rowid(db_);
}

std::string LocalSqliteDriver::lastError() const {
    return errorMessage_;
}

QueryResult LocalSqliteDriver::execQuery(const std::string& sql, bool isDataQuery) {
    QueryResult result;
    if (!db_) {
        errorMessage_ = "Database not open";
        result.elapsedMs = -1;
        return result;
    }

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        errorMessage_ = sqlite3_errmsg(db_);
        result.elapsedMs = -1;
        return result;
    }

    // 获取列名
    int colCount = sqlite3_column_count(stmt);
    for (int i = 0; i < colCount; ++i) {
        result.columns.push_back(sqlite3_column_name(stmt, i));
    }

    // 逐行读取
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        std::vector<std::string> row;
        for (int i = 0; i < colCount; ++i) {
            const char* text = reinterpret_cast<const char*>(sqlite3_column_text(stmt, i));
            row.push_back(text ? text : "NULL");
        }
        result.rows.push_back(std::move(row));
    }

    sqlite3_finalize(stmt);
    result.totalRows = static_cast<int>(result.rows.size());
    result.elapsedMs = 0.0;  // 本地驱动无需计时
    return result;
}

}  // namespace remote_sqlite_qt
